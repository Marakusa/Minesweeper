// Minesweeper.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Minesweeper.h"
#include <string>
#include <Windowsx.h>
#include <map>
#include <list>
#include <sstream>
#include <iostream>

using std::string;
using std::map;
using std::list;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Minesweeper variables:
int buttonSize = 24;
constexpr int gameAreaSize = 10;
int gamePadding = 10;
int toolbarHeight = 20;
int maxDensity = 10;
int generatorSeedOffset = 0;

int gameArea[gameAreaSize * gameAreaSize];
string gameAreaText[gameAreaSize * gameAreaSize];
HWND buttons[gameAreaSize * gameAreaSize] = { };
int revealed[gameAreaSize * gameAreaSize] = { };
bool started = false;

HWND window = NULL;

int GetDensity(int areaSize)
{
    int density = (int)ceil(pow((float)(maxDensity) / ((float)areaSize / 10.0), 1.3));

    if (density > maxDensity) density = maxDensity;
    else if (density < 5) density = 5;

    return density;
}
// Generate the mine field
bool GenerateField()
{
    srand(time(0) + generatorSeedOffset);

    int randomDensity = GetDensity(gameAreaSize);

    // Generate mines
    for (int index = 0; index < gameAreaSize * gameAreaSize; index++)
    {
        // -1 = mine
        // 0 = nothing
        int gtype = rand() % randomDensity;
        gameArea[index] = gtype == 0 ? -1 : 0;
        gameAreaText[index] = gtype == 0 ? "M" : "0";
    }

    // Generate numbers
    for (int index = 0; index < gameAreaSize * gameAreaSize; index++)
    {
        if (gameArea[index] != -1)
        {
            // Get x and y coordinates by index
            float pos = (float)index / (float)gameAreaSize;
            int x = round((float)(pos - floor(pos)) * (float)gameAreaSize);
            int y = floor(pos);
            for (int rx = -1; rx < 2; rx++)
            {
                for (int ry = -1; ry < 2; ry++)
                {
                    if (x + rx >= 0 && x + rx < gameAreaSize &&
                        y + ry >= 0 && y + ry < gameAreaSize
                        && gameArea[index + rx + ry * gameAreaSize] == -1)
                    {
                        gameArea[index] += 1;
                    }
                }
            }

            string convertedString = std::to_string(gameArea[index]);
            gameAreaText[index] = convertedString;
        }
    }

    return true;
}
void RevealPoint(int index, LPARAM lParam, WPARAM wParam);
void CheckSlot(int index, int px, int py, WPARAM wParam)
{
    int checkIndex = px + py * gameAreaSize;
    if (gameArea[index] == 0 && gameArea[checkIndex] != -1 && revealed[checkIndex] != 1)
    {
        revealed[checkIndex] = 1;
        RevealPoint(checkIndex, (LPARAM)buttons[checkIndex], wParam);
    }
}
void RevealPoint(int index, LPARAM lParam, WPARAM wParam)
{
    // Get x and y coordinates by index
    float pos = (float)index / (float)gameAreaSize;
    int x = round((float)(pos - floor(pos)) * (float)gameAreaSize);
    int y = floor(pos);

    DestroyWindow(buttons[index]);

    if (gameArea[index] != 0)
    {
        HWND hwndLabel = CreateWindow(
            L"static",  // Predefined class; Unicode assumed 
            (const wchar_t*)((gameAreaText[index]).c_str()),      // Button text 
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
            buttonSize * x + gamePadding,         // x position 
            buttonSize * y + gamePadding,         // y position 
            buttonSize,        // Button width
            buttonSize,        // Button height
            window,       // Parent window
            NULL,       // No menu.
            (HINSTANCE)GetWindowLongPtr(window, GWLP_HINSTANCE),
            NULL);
        
        const TCHAR* fontName = _T("Arial");
        const long nFontSize = (int)(buttonSize * 0.7);

        HDC hdc = GetDC(hwndLabel);

        LOGFONT logFont = { 0 };
        logFont.lfHeight = -MulDiv(nFontSize, GetDeviceCaps(hdc, LOGPIXELSY), 72);
        logFont.lfWeight = FW_BLACK;
        _tcscpy_s(logFont.lfFaceName, fontName);

        HFONT font = CreateFontIndirect(&logFont);

        SendMessage(hwndLabel, WM_SETFONT, (WPARAM)font, (LPARAM)MAKELONG(TRUE, 0));
        
        buttons[index] = hwndLabel;
    }

    int px = x;
    int py = y;

    // Right
    px = x + 1;
    py = y;
    if (px >= 0 && px < gameAreaSize && py >= 0 && py < gameAreaSize)
    {
        CheckSlot(index, px, py, wParam);
    }

    // Left
    px = x - 1;
    py = y;
    if (px >= 0 && px < gameAreaSize && py >= 0 && py < gameAreaSize)
    {
        CheckSlot(index, px, py, wParam);
    }

    // Up
    px = x;
    py = y + 1;
    if (px >= 0 && px < gameAreaSize && py >= 0 && py < gameAreaSize)
    {
        CheckSlot(index, px, py, wParam);
    }

    // Down
    px = x;
    py = y - 1;
    if (px >= 0 && px < gameAreaSize && py >= 0 && py < gameAreaSize)
    {
        CheckSlot(index, px, py, wParam);
    }
}

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MINESWEEPER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MINESWEEPER));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MINESWEEPER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_MINESWEEPER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   RECT rect;
   rect.left = rect.top = 0;
   rect.right = buttonSize * gameAreaSize + gamePadding * 2;
   rect.bottom = buttonSize * gameAreaSize + toolbarHeight + gamePadding * 2;
   AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);
   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0,
       rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   window = hWnd;

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int pressedIndex = -1;

    switch (message)
    {
        case WM_CREATE:
        {
            for (int index = 0; index < gameAreaSize * gameAreaSize; index++)
            {
                // Get x and y coordinates by index
                float pos = (float)index / (float)gameAreaSize;
                int x = round((float)(pos - floor(pos)) * (float)gameAreaSize);
                int y = floor(pos);

                HWND hwndButton = CreateWindow(
                    L"BUTTON",  // Predefined class; Unicode assumed 
                    L"",      // Button text 
                    WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
                    buttonSize * x + gamePadding,         // x position 
                    buttonSize * y + gamePadding,         // y position 
                    buttonSize,        // Button width
                    buttonSize,        // Button height
                    hWnd,       // Parent window
                    NULL,       // No menu.
                    (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                    NULL);

                buttons[index] = hwndButton;
            }
        }
        case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
                case IDM_ABOUT:
                    DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                    break;
                case IDM_EXIT:
                    if (MessageBox(hWnd, L"Really quit?", szTitle, MB_OKCANCEL) == IDOK)
                    {
                        DestroyWindow(hWnd);
                    }
                    // Else: canceled
                    return 0;
                    break;
                case BN_CLICKED:
                    for (int index = 0; index < gameAreaSize * gameAreaSize; index++)
                    {
                        if ((HWND)lParam == buttons[index])
                        {
                            pressedIndex = index;
                            break;
                        }
                    }

                    if (pressedIndex >= 0 && pressedIndex < gameAreaSize * gameAreaSize)
                    {
                        while (!started)
                        {
                            if (GenerateField()) started = gameArea[pressedIndex] == 0;
                            generatorSeedOffset++;
                        }

                        revealed[pressedIndex] = 1;
                        RevealPoint(pressedIndex, lParam, wParam);
                    }
                    break;
                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        }
        break;
        case WM_CLOSE:
        {
            if (MessageBox(hWnd, L"Really quit?", szTitle, MB_OKCANCEL) == IDOK)
            {
                DestroyWindow(hWnd);
            }
            // Else: canceled
            return 0;
        }
        break;
        case WM_DESTROY:
            PostQuitMessage(0);
        break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
