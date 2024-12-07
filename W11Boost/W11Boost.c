#include "resource.h"
#include "Common.h"
#include <Shlwapi.h>
#include <shellapi.h>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                       // Current instance
wchar_t szTitle[MAX_LOADSTRING];       // The title bar text
wchar_t szWindowClass[MAX_LOADSTRING]; // The main window class name

// Forward declarations of functions included in this code module:
unsigned short MyRegisterClass(HINSTANCE hInstance);
bool InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, unsigned int, WPARAM, LPARAM);

struct
{
    UINT restore_point, local_privacy, microsoft_store, appx_support, disable_sleep;
} checkbox_status;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine,
                      _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

// Place code that should always be ran here:
#ifdef _DEBUG
    AllocConsole();
    FILE *pCerr;
    freopen_s(&pCerr, "CONOUT$", "w", stderr);
#endif

    wchar_t *fullPath = get_log_directory();
    if (PathFileExistsW(fullPath))
    {
        SHFILEOPSTRUCTW dir = {0};
        dir.wFunc = FO_DELETE;
        dir.pFrom = fullPath;
        dir.fFlags = FOF_NO_UI | FOF_NOERRORUI;
        SHFileOperationW(&dir);
    }
    CreateDirectoryW(fullPath, NULL);
    free(fullPath);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_W11BOOST, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAcceleratorsW(hInstance, MAKEINTRESOURCE(IDC_W11BOOST));

    MSG msg;

    // Main message loop:
    while (GetMessageW(&msg, NULL, 0, 0))
    {
        if (!TranslateAcceleratorW(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    return (int)msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
unsigned short MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex = {
        .cbSize = sizeof(WNDCLASSEX),
        .style = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc = WndProc,
        .cbClsExtra = 0,
        .cbWndExtra = 0,
        .hInstance = hInstance,
        .hCursor = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground = (HBRUSH)(COLOR_WINDOW + 1),
        .lpszMenuName = MAKEINTRESOURCEW(IDC_W11BOOST),
        .lpszClassName = szWindowClass,
    };

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
bool InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Store instance handle in our global variable

    UINT screen_dpi = GetDpiForSystem();
    int width = MulDiv(480, screen_dpi, 100);
    int height = MulDiv(300, screen_dpi, 100);

    HWND hWnd = CreateWindowExW(0, szWindowClass, szTitle, WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU, CW_USEDEFAULT,
                                0, width, height, NULL, NULL, hInstance, NULL);

    if (!hWnd)
    {
        return FALSE;
    }

    HMONITOR monitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi = {sizeof(mi)};

    // Puts W11Boost's window in the center of the current monitor
    if (GetMonitorInfoW(monitor, &mi))
    {
        RECT rcWork = mi.rcWork;
        int x = rcWork.left + (rcWork.right - rcWork.left - width) / 2;
        int y = rcWork.top + (rcWork.bottom - rcWork.top - height) / 2;

        SetWindowPos(hWnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
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
LRESULT CALLBACK WndProc(HWND hWnd, unsigned int message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE: {
        int font_size = 24;
        UINT screen_dpi = GetDpiForWindow(hWnd);
        int font_dpi = 96;
        HFONT hFont = CreateFontW(MulDiv(font_size, screen_dpi, font_dpi), 0, 0, 0, FW_LIGHT, FALSE, FALSE, 0,
                                  ANSI_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_NATURAL_QUALITY,
                                  DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

        RECT rcClient;
        GetClientRect(hWnd, &rcClient);

        struct s_common
        {
            int left, right, top, bottom, centerWidth, centerHeight;
        } common;

        struct s_button
        {
            int width, height;
            HWND apply;
        } button;

        struct s_checkbox
        {
            int width, height;
            HWND localPrivacy, backup, store, appx, sleep;
        } checkbox;

        common.left = rcClient.left + 4;
        common.right = rcClient.right - 8;

        common.top = rcClient.top;
        common.bottom = rcClient.bottom - 4;

        common.centerWidth = common.right / 2;
        common.centerHeight = common.bottom / 2;

        button.width = common.right;
        button.height = (common.centerHeight * 4) / 10; // 40%

        checkbox.width = rcClient.right;
        checkbox.height = (common.centerHeight * 3) / 10; // 30%

        HWND apply_button = CreateWindowW(L"BUTTON", L"Apply W11Boost", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT,
                                          common.left, common.bottom - button.height, button.width, button.height, hWnd,
                                          (HMENU)IDC_APPLY_W11BOOST, GetModuleHandle(NULL), NULL);

        HWND appx_checkbox =
            CreateWindowW(L"BUTTON", L"Install support for WinGet and .appx/.appxbundle",
                          WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_FLAT, common.left, common.top, checkbox.width,
                          checkbox.height, hWnd, (HMENU)IDC_INSTALL_WINGET, GetModuleHandle(NULL), NULL);

        HWND anti_forensics_checkbox =
            CreateWindowW(L"BUTTON", L"Reduce local data collection / anti-forensics",
                          WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_FLAT, common.left, common.top + checkbox.height,
                          checkbox.width, checkbox.height, hWnd, (HMENU)IDC_PRIVACY_MODE, GetModuleHandle(NULL), NULL);

        HWND system_restore_checkbox = CreateWindowW(
            L"BUTTON", L"Create a backup / System Restore point", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_FLAT,
            common.left, common.top + (checkbox.height * 2), checkbox.width, checkbox.height, hWnd,
            (HMENU)IDC_CREATE_RESTORE_POINT, GetModuleHandle(NULL), NULL);

        HWND microsoft_store_checkbox =
            CreateWindowW(L"BUTTON", L"Install the Microsoft Store", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_FLAT,
                          common.left, common.top + (checkbox.height * 3), checkbox.width, checkbox.height, hWnd,
                          (HMENU)IDC_INSTALL_MICROSOFT_STORE, GetModuleHandle(NULL), NULL);

        HWND disable_sleep_and_hibernate =
            CreateWindowW(L"BUTTON", L"Disable sleep and hibernate", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_FLAT,
                          common.left, common.top + (checkbox.height * 4), checkbox.width, checkbox.height, hWnd,
                          (HMENU)IDC_DISABLE_SLEEP, GetModuleHandle(NULL), NULL);

        SendMessageW(apply_button, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessageW(appx_checkbox, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessageW(anti_forensics_checkbox, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessageW(system_restore_checkbox, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessageW(microsoft_store_checkbox, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessageW(disable_sleep_and_hibernate, WM_SETFONT, (WPARAM)hFont, TRUE);

        CheckDlgButton(hWnd, IDC_CREATE_RESTORE_POINT, BST_CHECKED);
    }
    break;
    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        int result;
        // Parse the selections:
        switch (wmId)
        {
        case IDC_APPLY_W11BOOST:
            if (MessageBoxW(hWnd, L"Do you want to apply W11Boost?", L"W11Boost", MB_YESNO) == IDYES)
            {
                checkbox_status.restore_point = IsDlgButtonChecked(hWnd, IDC_CREATE_RESTORE_POINT);
                checkbox_status.local_privacy = IsDlgButtonChecked(hWnd, IDC_PRIVACY_MODE);
                checkbox_status.microsoft_store = IsDlgButtonChecked(hWnd, IDC_INSTALL_MICROSOFT_STORE);
                checkbox_status.appx_support = IsDlgButtonChecked(hWnd, IDC_INSTALL_WINGET);
                checkbox_status.disable_sleep = IsDlgButtonChecked(hWnd, IDC_DISABLE_SLEEP);


                result = create_restore_point();
                if (result != 0)
                    MessageBoxW(hWnd, L"System Restore point failed to be created!", L"W11Boost", MB_OK | MB_ICONERROR);

                result = install_privacy_mode();
                if (result != 0)
                    MessageBoxW(hWnd, L"Failed to install Privacy Mode!", L"W11Boost", MB_OK | MB_ICONERROR);

                wchar_t cmd_line[] = L"wsreset.exe -i";
                result = start_command_and_wait(cmd_line);
                if (result != 0)
                    MessageBoxW(hWnd, L"Failed to install the Microsoft Store!", L"W11Boost", MB_OK | MB_ICONERROR);

                result = install_appx_support();
                if (result != 0)
                    MessageBoxW(hWnd, L"Failed to install appx support!", L"W11Boost", MB_OK | MB_ICONERROR);

                result = disable_sleep();
                if (result != 0)
                    MessageBoxW(hWnd, L"Failed to disable sleep or hibernate!", L"W11Boost", MB_OK | MB_ICONERROR);

                result = gp_edits();
                if (result != 0)
                    MessageBoxW(hWnd, L"Failed to apply W11Boost!", L"W11Boost", MB_OK | MB_ICONERROR);
            }
            break;

        case IDC_CREATE_RESTORE_POINT:
            checkbox_status.restore_point = IsDlgButtonChecked(hWnd, IDC_CREATE_RESTORE_POINT);

            if (checkbox_status.restore_point)
            {
                CheckDlgButton(hWnd, IDC_CREATE_RESTORE_POINT, BST_UNCHECKED);
            }
            else
            {
                CheckDlgButton(hWnd, IDC_CREATE_RESTORE_POINT, BST_CHECKED);
            }
            break;

        case IDC_PRIVACY_MODE:
            checkbox_status.local_privacy = IsDlgButtonChecked(hWnd, IDC_PRIVACY_MODE);

            if (checkbox_status.local_privacy)
            {
                CheckDlgButton(hWnd, IDC_PRIVACY_MODE, BST_UNCHECKED);
            }
            else
            {
                CheckDlgButton(hWnd, IDC_PRIVACY_MODE, BST_CHECKED);
            }
            break;

        case IDC_INSTALL_MICROSOFT_STORE:
            checkbox_status.microsoft_store = IsDlgButtonChecked(hWnd, IDC_INSTALL_MICROSOFT_STORE);

            if (checkbox_status.microsoft_store)
            {
                CheckDlgButton(hWnd, IDC_INSTALL_MICROSOFT_STORE, BST_UNCHECKED);
            }
            else
            {
                CheckDlgButton(hWnd, IDC_INSTALL_MICROSOFT_STORE, BST_CHECKED);
            }
            break;

        case IDC_INSTALL_WINGET:
            checkbox_status.appx_support = IsDlgButtonChecked(hWnd, IDC_INSTALL_WINGET);

            if (checkbox_status.appx_support)
            {
                CheckDlgButton(hWnd, IDC_INSTALL_WINGET, BST_UNCHECKED);
            }
            else
            {
                CheckDlgButton(hWnd, IDC_INSTALL_WINGET, BST_CHECKED);
            }
            break;

        case IDC_DISABLE_SLEEP:
            checkbox_status.disable_sleep = IsDlgButtonChecked(hWnd, IDC_DISABLE_SLEEP);

            if (checkbox_status.disable_sleep)
            {
                CheckDlgButton(hWnd, IDC_DISABLE_SLEEP, BST_UNCHECKED);
            }
            else
            {
                CheckDlgButton(hWnd, IDC_DISABLE_SLEEP, BST_CHECKED);
            }
            break;

        default:
            return DefWindowProcW(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        if (hdc == NULL)
        {
            MessageBoxW(hWnd, L"BeginPaint failed", L"W11Boost -> Error", MB_OK | MB_ICONERROR);
            return -1;
        }
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }
    return 0;
}
