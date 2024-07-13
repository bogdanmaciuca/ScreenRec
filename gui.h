#ifndef GUI_H
#define GUI_H

#include <windows.h>

#define GUI_CLASS_NAME "screenrec"
#define GUI_WND_NAME "Screen Recorder"

LRESULT CALLBACK GUI_WindowProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
typedef struct GUI_t {
    HWND window;
    WNDCLASSA windowClass;
    int wndWidth;
    int wndHeight;
    HDC hdc;
    BITMAPINFO bmpInfo;

    BYTE* pixels;
    int srcWidth;
    int srcHeight;

    bool selectionFinished;
    POINT selectionStart;
    POINT selectionEnd;
    HBRUSH selectionBrush;
} GUI;
GUI gGui;

bool GUI_Init(HINSTANCE instance, int srcWidth, int srcHeight) {
    gGui.selectionStart.x = -1;
    gGui.selectionStart.y = -1;
    gGui.selectionEnd.x = -1;
    gGui.selectionEnd.y = -1;
    gGui.srcWidth = srcWidth;
    gGui.srcHeight = srcHeight;

    gGui.windowClass.hInstance = instance;
    gGui.windowClass.lpszClassName = GUI_CLASS_NAME;
    gGui.windowClass.lpfnWndProc = GUI_WindowProc;
    RegisterClassA(&gGui.windowClass);

    RECT desktopRect;
    HWND desktopWindow = GetDesktopWindow();
    GetWindowRect(desktopWindow, &desktopRect);
    int desktopWidth = desktopRect.right - desktopRect.left;
    int desktopHeight = desktopRect.bottom - desktopRect.top;

    int windowX    = desktopWidth / 8;
    int windowY    = desktopHeight / 8;
    gGui.wndWidth  = desktopWidth * 3 / 4;
    gGui.wndHeight = desktopHeight * 3 / 4;

    gGui.window = CreateWindowA(
        GUI_CLASS_NAME, GUI_WND_NAME,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        windowX, windowY, gGui.wndWidth, gGui.wndHeight,
        NULL, NULL, instance, NULL
    );

    gGui.bmpInfo.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    gGui.bmpInfo.bmiHeader.biPlanes      = 1;
    gGui.bmpInfo.bmiHeader.biBitCount    = 32;
    gGui.bmpInfo.bmiHeader.biCompression = BI_RGB;
    gGui.bmpInfo.bmiHeader.biWidth       = srcWidth;
    gGui.bmpInfo.bmiHeader.biHeight      = -srcHeight; 
    gGui.bmpInfo.bmiHeader.biSizeImage   = gGui.srcWidth * gGui.srcHeight * 4;

    gGui.hdc = GetDC(gGui.window);
    gGui.selectionBrush = CreateSolidBrush(RGB(50, 150, 50));

    return (gGui.window != NULL);
}

void GUI_Terminate() {
    DeleteObject(gGui.selectionBrush);
}

int GUI_GetMessage() {
    MSG message;
    int retVal = GetMessageA(&message, NULL, 0, 0);
    if (retVal > 0) {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }
    return retVal;
}

void GUI_SetPixels(BYTE* pixels) {
    gGui.pixels = pixels;
}

void GUI__DrawSelection(HDC hdc) {
    RECT rect;
    rect.left   = gGui.selectionStart.x;
    rect.right  = gGui.selectionEnd.x;
    rect.top    = gGui.selectionStart.y;
    rect.bottom = gGui.selectionEnd.y;
    FrameRect(hdc, &rect, gGui.selectionBrush);
}


void GUI_ToScreenSpace(POINT* point) {
    float ratioX = (float)gGui.srcWidth / (float)gGui.wndWidth;
    float ratioY = (float)gGui.srcHeight / (float)gGui.wndHeight;
    point->x *= ratioX;
    point->y *= ratioY;
}

LRESULT CALLBACK GUI_WindowProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    switch(message) {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_LBUTTONDOWN:
        gGui.selectionFinished = false;
        gGui.selectionStart.x = LOWORD(lparam);
        gGui.selectionStart.y = HIWORD(lparam);
        gGui.selectionEnd.x = LOWORD(lparam);
        gGui.selectionEnd.y = HIWORD(lparam);
        break;
    case WM_LBUTTONUP:
        gGui.selectionFinished = true;
        break;
    case WM_MOUSEMOVE:
        if (!gGui.selectionFinished) {
            gGui.selectionEnd.x = LOWORD(lparam);
            gGui.selectionEnd.y = HIWORD(lparam);
        }
        break;
    case WM_KEYDOWN:
        if (gGui.selectionFinished && (wparam == VK_RETURN || wparam == VK_SPACE))
            PostQuitMessage(0);
        break;
    case WM_PAINT: {
        if (gGui.pixels) {
            if (!StretchDIBits(gGui.hdc, 0, 0, gGui.wndWidth, gGui.wndHeight,
                0, 0, gGui.srcWidth, gGui.srcHeight, gGui.pixels, &gGui.bmpInfo, DIB_RGB_COLORS, SRCCOPY))
                printf("StretchDIBits() failed.\n");
        }
        if (gGui.selectionStart.x != -1)
            GUI__DrawSelection(gGui.hdc);
    } break;
    default:
        return DefWindowProcA(window, message, wparam, lparam);
    }
    return 0;
}

#endif
