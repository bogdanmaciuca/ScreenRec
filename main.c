#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <direct.h>
#include <sys/stat.h>
#include <stdbool.h>
#include "stb_image_write.h"

// BGRA -> RGBA
void SwapChannels(BYTE* pixels, int width, int height) {
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            int b = pixels[(j * width + i) * 4 + 0];
            int g = pixels[(j * width + i) * 4 + 1];
            int r = pixels[(j * width + i) * 4 + 2];
            int a = 255;
            pixels[(j * width + i) * 4 + 0] = r;
            pixels[(j * width + i) * 4 + 1] = g;
            pixels[(j * width + i) * 4 + 2] = b;
            pixels[(j * width + i) * 4 + 3] = a;
        }
    }
}

typedef struct Capture_t {
    BYTE* pixels;
    int width, height;
    HDC windowDC;
    HDC memoryDC;
    HBITMAP bitmap;
    HBITMAP memoryBitmap;
    HBITMAP oldBitmap;
    BITMAPINFO bitmapInfo;
} Capture;

void CaptureInit(Capture* cap) {
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    stbi_flip_vertically_on_write(true);
    
    ZeroMemory(cap, sizeof(Capture));

    HWND window = GetDesktopWindow();
    RECT windowRect;
    GetClientRect(window, &windowRect);
    cap->width = windowRect.right - windowRect.left;
    cap->height = windowRect.bottom - windowRect.top;

    cap->windowDC = GetDC(window);
    cap->bitmap = CreateCompatibleBitmap(cap->windowDC, cap->width, cap->height);
    SelectObject(cap->windowDC, cap->bitmap);

    cap->memoryBitmap = CreateCompatibleBitmap(cap->windowDC, cap->width, cap->height);
    cap->memoryDC = CreateCompatibleDC(cap->windowDC);
    cap->oldBitmap = SelectObject(cap->memoryDC, cap->memoryBitmap);

    BitBlt(cap->memoryDC, 0, 0, cap->width, cap->height, cap->windowDC, 0, 0, SRCCOPY | CAPTUREBLT);

    cap->bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    SelectObject(cap->memoryDC, cap->oldBitmap);
    if (!GetDIBits(cap->memoryDC, cap->memoryBitmap, 0, cap->height, NULL, &cap->bitmapInfo, DIB_RGB_COLORS)) {
        printf("GetDIBits() failed.\n");
        exit(1);
    }

    cap->pixels = malloc(cap->bitmapInfo.bmiHeader.biSizeImage);
}

void CaptureGetFrame(Capture *cap) {
    BitBlt(cap->memoryDC, 0, 0, cap->width, cap->height, cap->windowDC, 0, 0, SRCCOPY | CAPTUREBLT);
    if (!GetDIBits(cap->memoryDC, cap->memoryBitmap, 0, cap->height, cap->pixels, &cap->bitmapInfo, DIB_RGB_COLORS)) {
        printf("GetDIBits() failed.\n");
        exit(1);
    }
}

char* GetTimeAsString() {
    struct tm tm;
    time_t t;
    int strLen = 32;
    char *timeString = malloc(strLen*sizeof(char));

    t = time(NULL);
    localtime_s(&tm, &t);

    strftime(timeString, strLen, "%m.%d-%H.%M.%S", &tm);

    return timeString;
}

// Filename without extension!!!
void CaptureScreenshot(Capture* cap, const char* filename) {
    char fullFilename[128] = {};
    strcpy_s(fullFilename, sizeof(fullFilename), filename);
    strcat_s(fullFilename, sizeof(fullFilename), " ");
    char* timeString = GetTimeAsString();
    strcat_s(fullFilename, sizeof(fullFilename), timeString);
    free(timeString);
    strcat_s(fullFilename, sizeof(fullFilename), ".png");

    CaptureGetFrame(cap);
    int channelNum = 4;
    int stride = cap->width * channelNum;
    SwapChannels(cap->pixels, cap->width, cap->height);
    stbi_write_png(fullFilename, cap->width, cap->height, channelNum, cap->pixels, stride);

    // Save to clipboard
    OpenClipboard(NULL);
    EmptyClipboard();
    SetClipboardData(CF_BITMAP, cap->memoryBitmap);
    CloseClipboard();
}

void CaptureTerminate(Capture* cap) {
    DeleteObject(cap->memoryBitmap);
    DeleteObject(cap->bitmap);
    DeleteDC(cap->memoryDC);
    if (cap->pixels) free(cap->pixels);
}

void EnsureDir(const char *path) {
    struct _stat st = {0};
    if (_stat(path, &st) == -1)
        _mkdir(path);
}

int main() {
    Capture capture;
    CaptureInit(&capture);

    EnsureDir("Screenshots");
    CaptureScreenshot(&capture, "Screenshots/Screenshot");

    CaptureTerminate(&capture);
    return 0;
}
