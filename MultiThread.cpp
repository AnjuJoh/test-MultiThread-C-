#include <windows.h>
#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <condition_variable>

// Global variables
HWND hList1, hList2, hButton;
HBRUSH hBrushBlue;
std::vector<int> numbers;
std::mutex mtx;
std::condition_variable cv1;
std::condition_variable cv2;

// Function to generate numbers and add to list box

// Function to generate numbers and add to list box
void GenerateNumbers() {
    for (int i = 1; i <= 100; ++i) {
        std::this_thread::sleep_for(std::chrono::seconds(1));{

        std::lock_guard<std::mutex> lock(mtx);
        numbers.push_back(i);

        std::wstring num = std::to_wstring(i);

        SendMessage(hList1, LB_ADDSTRING, 0, (LPARAM)num.c_str());
        int count = SendMessage(hList1, LB_GETCOUNT, 0, 0);
        SendMessage(hList1, LB_SETTOPINDEX, count - 1, 0);
    }

        if (i % 5 == 0) {
            cv1.notify_one();
        }
    }
}

// Function to print multiples of 5
void PrintMultiples() {
    std::unique_lock<std::mutex> lock(mtx);

    for (int i = 5; i <= 100; i += 5) {
        cv1.wait(lock, [&]() { return std::find(numbers.begin(), numbers.end(), i) != numbers.end(); });

        std::wstring num = std::to_wstring(i);
        SendMessage(hList1, LB_ADDSTRING, 0, (LPARAM)num.c_str());
        int count = SendMessage(hList1, LB_GETCOUNT, 0, 0);
        SendMessage(hList1, LB_SETTOPINDEX, count - 1, 0);

        SendMessage(hList2, LB_ADDSTRING, 0, (LPARAM)num.c_str());
        count = SendMessage(hList2, LB_GETCOUNT, 0, 0);
        SendMessage(hList2, LB_SETTOPINDEX, count - 1, 0);

        if (i % 5 == 0) {
            cv2.notify_one();
        }
    }
}

// Function to clear the content of list boxes and the numbers vector
void ClearContent() {
    SendMessage(hList1, LB_RESETCONTENT, 0, 0);
    SendMessage(hList2, LB_RESETCONTENT, 0, 0);

    std::lock_guard<std::mutex> lock(mtx);
    numbers.clear();
}

// Window procedure to handle messages
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE: {
        // Create the first list box with modern style
        hList1 = CreateWindowW(L"LISTBOX", NULL,
            WS_CHILD | WS_VISIBLE | LBS_NOTIFY | WS_VSCROLL | WS_BORDER,
            10, 10, 200, 500,
            hwnd, NULL, NULL, NULL);

        // Create the second list box with modern style
        hList2 = CreateWindowW(L"LISTBOX", NULL,
            WS_CHILD | WS_VISIBLE | LBS_NOTIFY | WS_VSCROLL | WS_BORDER,
            220, 10, 200, 500,
            hwnd, NULL, NULL, NULL);

        // Create the start button with modern style
        hButton = CreateWindowW(L"BUTTON", L"Start",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            430, 10, 100, 30,
            hwnd, (HMENU)1, NULL, NULL);

        // Create a blue brush for the button background
        hBrushBlue = CreateSolidBrush(RGB(0, 0, 255));

        break;
    }
    case WM_CTLCOLORBTN: {
        HDC hdc = (HDC)wParam;
        if ((HWND)lParam == hButton) {
            SetBkColor(hdc, RGB(0, 0, 255));
            SetTextColor(hdc, RGB(255, 255, 255));  // Optional: set text color to white
            return (INT_PTR)hBrushBlue;
        }
        break;
    }
    case WM_COMMAND: {
        if (LOWORD(wParam) == 1) {  // Button click
            EnableWindow(hButton, FALSE); // Disable the Start button

            // Clear previous content
            ClearContent();

            std::thread generateThread(GenerateNumbers);
            std::thread printThread(PrintMultiples);

            generateThread.detach();
            printThread.detach();
        }
        break;
    }
    case WM_DESTROY: {
        DeleteObject(hBrushBlue);  // Clean up the brush
        PostQuitMessage(0);
        break;
    }
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

// Entry point
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"Sample Window Class";

    WNDCLASS wc = { };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"List Box Example",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 550, 550,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
