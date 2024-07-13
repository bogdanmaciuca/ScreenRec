/*
* TODO:
* - GUI (window which takes a screenshot and lets you crop it)
*   - compute the cursor coordinates in source space 
*   and adjust the capture parameters accordingly
*/

#include <windows.h>
#include <time.h>
#include "capture.h"
#include "gui.h"

#define _DEBUG

#define CAP_SCREENSHOT_FILENAME "ScreenRec_Data/Screenshots/Screenshot"
#define TIME_TO_MINIMIZE 150 // In milliseconds

void EnsureDir(const char *path) {
    struct _stat st = {0};
    if (_stat(path, &st) == -1)
        _mkdir(path);
}

#ifdef _DEBUG
int main() {
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
#endif

    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    Capture capture;
    Capture_Init(&capture, 0, 0, 0, 0);
    Capture_GetFrame(&capture);

#ifdef _DEBUG
    if (!GUI_Init(NULL, capture.width, capture.height)) exit(1);
#else
    if(!GUI_Init(hInstance, capture.width, capture.height)) exit(1);
#endif

    POINT cursorPos;
    GUI_SetPixels(capture.pixels);
    while (GUI_GetMessage());
    Capture_Terminate(&capture);

    if (gGui.selectionFinished) {
        if (gGui.usedForRec) {
            // ...
        }
        else {
            ShowWindow(gGui.window, SW_MINIMIZE);
            Sleep(TIME_TO_MINIMIZE); // Wait for the window to be minimized
            EnsureDir("ScreenRec_Data");
            EnsureDir("ScreenRec_Data/Screenshots");
            Capture_Init(&capture, gGui.selectionStart.x, gGui.selectionStart.y, gGui.selectionEnd.x, gGui.selectionEnd.y);
            Capture_Screenshot(&capture, CAP_SCREENSHOT_FILENAME);
        }
    }
    //time_t start,end;
    //time (&start);
    //for (int i = 0; i < 600; i++)
    //    Capture_GetFrame(&capture);
    //time (&end);
    //double dif = difftime (end,start);
    //printf ("Elasped time is %.2lf seconds.", dif);

    Capture_Terminate(&capture);
    GUI_Terminate();
    return 0;
}
