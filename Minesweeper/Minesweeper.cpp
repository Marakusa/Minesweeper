// Minesweeper.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Minesweeper.h"
#include <string>
#include <Windowsx.h>
#include <map>
#include <list>

using std::string;
using std::map;
using std::list;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Minesweeper variables:
int buttonSize = 32;
constexpr int gameAreaSize = 16;
int gamePadding = 16;
int toolbarHeight = 20;

int gameArea[gameAreaSize][gameAreaSize];
string gameAreaText[gameAreaSize][gameAreaSize];
map<POINT, HWND> buttons = { };
list<POINT> revealed = { };

// Generate the mine field
bool GenerateField()
{
    // Generate mines
    for (int gx = 0; gx < gameAreaSize; gx++)
    {
        for (int gy = 0; gy < gameAreaSize; gy++)
        {
            // -1 = mine
            // 0 = nothing
            int gtype = rand() % 20;
            gameArea[gx][gy] = gtype == 0 ? -1 : 0;
            gameAreaText[gx][gy] = gtype == 0 ? "M" : "0";
        }
    }

    // Generate numbers
    for (int gx = 0; gx < gameAreaSize; gx++)
    {
        for (int gy = 0; gy < gameAreaSize; gy++)
        {
            if (gameArea[gx][gy] != -1)
            {
                for (int rx = -1; rx < 2; rx++)
                {
                    for (int ry = -1; ry < 2; ry++)
                    {
                        if (gx + rx >= 0 && gx + rx < gameAreaSize &&
                            gy + ry >= 0 && gy + ry < gameAreaSize
                            && gameArea[gx + rx][gy + ry] == -1)
                        {
                            gameArea[gx][gy] += 1;
                        }
                    }
                }

                string convertedString = std::to_string(gameArea[gx][gy]);
                gameAreaText[gx][gy] = convertedString;
            }
        }
    }

    return true;
}
void RevealPoint(POINT point, LPARAM lParam, WPARAM wParam, int x, int y)
{
    for (int px = -1; px < 2; px++)
    {
        for (int py = -1; py < 2; py++)
        {
            bool found = (std::find(revealed.begin(), revealed.end(), gameArea[px][py]) != revealed.end());
            if (gameArea[x][y] == 0 && !found)
            {
                POINT ppoint;
                ppoint.x = point.x;
                ppoint.y = point.y;

                revealed.push_front(ppoint);

                RevealPoint(ppoint, lParam, wParam, ppoint.x, ppoint.y);
            }
        }
    }
    SendMessage((HWND)lParam, WM_SETTEXT, wParam, (LPARAM)(gameArea[point.x][point.y] == 0 ? L"" : (const wchar_t*)((gameAreaText[point.x][point.y]).c_str())));
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
    switch (message)
    {
        case WM_CREATE:
        {
            //MessageBox(hWnd, L"Failed to generate the mine field", szTitle, MB_OK);
            //PostQuitMessage(0);
            //break;

            if (GenerateField())
            {
                for (int x = 0; x < gameAreaSize; x++)
                {
                    for (int y = 0; y < gameAreaSize; y++)
                    {
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

                        const TCHAR* fontName = _T("Arial");
                        const long nFontSize = 22;

                        HDC hdc = GetDC(hwndButton);

                        LOGFONT logFont = { 0 };
                        logFont.lfHeight = -MulDiv(nFontSize, GetDeviceCaps(hdc, LOGPIXELSY), 72);
                        logFont.lfWeight = FW_BLACK;
                        _tcscpy_s(logFont.lfFaceName, fontName);

                        HFONT font = CreateFontIndirect(&logFont);

                        SendMessage(hwndButton, WM_SETFONT, (WPARAM)font, (LPARAM)MAKELONG(TRUE, 0));
                    
                        POINT point;
                        point.x = x;
                        point.y = y;
                        buttons[point] = hwndButton;
                    }
                }
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
                    for (int x = 0; x < gameAreaSize; x++)
                    {
                        for (int y = 0; y < gameAreaSize; y++)
                        {
                            POINT point;
                            point.x = x;
                            point.y = y;

                            if ((HWND)lParam == buttons[point])
                            {
                                revealed.push_front(point);

                                RevealPoint(point, lParam, wParam, point.x, point.y);
                                break;
                            }
                        }
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
