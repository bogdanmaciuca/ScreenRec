#ifndef CAPTURE_H
#define CAPTURE_H

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <direct.h>
#include <stdbool.h>
#include <sys/stat.h>
#include "stb_image_write.h"

typedef struct Capture_t {
    BYTE* pixels;
    int x, y;
    int width, height;
    int srcWidth, srcHeight;
    HDC windowDC;
    HDC memoryDC;
    HBITMAP bitmap;
    HBITMAP memoryBitmap;
    HBITMAP oldBitmap;
    BITMAPINFO bitmapInfo;
} Capture;

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

void Capture_Init(Capture* cap, int x1, int y1, int x2, int y2) {
    ZeroMemory(cap, sizeof(Capture));

    HWND window = GetDesktopWindow();
    RECT windowRect;
    GetClientRect(window, &windowRect);
    cap->srcWidth = windowRect.right - windowRect.left;
    cap->srcHeight = windowRect.bottom - windowRect.top;
    if (x2 == 0 || y2 == 0) {
        cap->x = 0; cap->y = 0;
        cap->width = cap->srcWidth;
        cap->height = cap->srcHeight;
    }
    else {
        cap->x = x1; cap->y = y1;
        cap->width = x2 - x1;
        cap->height = y2 - y1;
    }
    cap->windowDC = GetDC(window);
    cap->bitmap = CreateCompatibleBitmap(cap->windowDC, cap->srcWidth, cap->srcHeight);
    SelectObject(cap->windowDC, cap->bitmap);

    cap->memoryBitmap = CreateCompatibleBitmap(cap->windowDC, cap->width, cap->height);
    cap->memoryDC = CreateCompatibleDC(cap->windowDC);
    cap->oldBitmap = SelectObject(cap->memoryDC, cap->memoryBitmap);

    BitBlt(cap->memoryDC, 0, 0, cap->width, cap->height, cap->windowDC, cap->x, cap->y, SRCCOPY);

    cap->bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    SelectObject(cap->memoryDC, cap->oldBitmap);
    if (!GetDIBits(cap->memoryDC, cap->memoryBitmap, 0, cap->srcHeight, NULL, &cap->bitmapInfo, DIB_RGB_COLORS)) {
        printf("GetDIBits() failed.\n");
        exit(1);
    }
    cap->bitmapInfo.bmiHeader.biHeight *= -1;

    cap->pixels = malloc(cap->width * cap->height * 4);
}

void Capture_Terminate(Capture* cap) {
    DeleteObject(cap->memoryBitmap);
    DeleteObject(cap->bitmap);
    DeleteDC(cap->memoryDC);
    if (cap->pixels) free(cap->pixels);
}

void Capture_GetFrame(Capture *cap) {
    BitBlt(cap->memoryDC, 0, 0, cap->width, cap->height, cap->windowDC, cap->x, cap->y, SRCCOPY);
    if (!GetDIBits(cap->memoryDC, cap->memoryBitmap, 0, cap->height, cap->pixels, &cap->bitmapInfo, DIB_RGB_COLORS)) {
        printf("GetDIBits() failed.\n");
        exit(1);
    }
}

// Filename without extension!!!
void Capture_Screenshot(Capture* cap, const char* filename) {
    char fullFilename[128] = {};
    strcpy_s(fullFilename, sizeof(fullFilename), filename);
    strcat_s(fullFilename, sizeof(fullFilename), " ");
    char* timeString = GetTimeAsString();
    strcat_s(fullFilename, sizeof(fullFilename), timeString);
    free(timeString);
    strcat_s(fullFilename, sizeof(fullFilename), ".png");

    Capture_GetFrame(cap);
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

#endif
