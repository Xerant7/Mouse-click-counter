#include <windows.h>
#include <string>
#include <ctime>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int clickCount = 0;
HWND buttonHandle;
HWND timerButtons[4];  // Array for timer buttons
HWND resultLabel;      // Label to display results
bool isTimerRunning = false;
time_t startTime;
int timerDuration = 0;
const int TIMER_ID = 1;

// Timer durations in seconds
const int TIMER_DURATIONS[] = {15, 30, 60, 120};

void StartTimer(HWND hwnd, int duration) {
    if (!isTimerRunning) {
        timerDuration = duration;
        clickCount = 0;
        isTimerRunning = true;
        startTime = time(nullptr);
        SetTimer(hwnd, TIMER_ID, 1000, nullptr);  // Update every second

        // Disable timer buttons during the test
        for (int i = 0; i < 4; i++) {
            EnableWindow(timerButtons[i], FALSE);
        }
    }
}

void EndTimer(HWND hwnd) {
    if (isTimerRunning) {
        KillTimer(hwnd, TIMER_ID);
        isTimerRunning = false;

        // Calculate and display results
        double clicksPerSecond = static_cast<double>(clickCount) / timerDuration;
        std::string result = "Result: " + std::to_string(clickCount) + " clicks\n" +
                           "CPS: " + std::to_string(clicksPerSecond);
        SetWindowText(resultLabel, result.c_str());

        // Re-enable timer buttons
        for (int i = 0; i < 4; i++) {
            EnableWindow(timerButtons[i], TRUE);
        }

        // Reset click counter display
        SetWindowText(buttonHandle, "Clicks: 0");
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "Click Counter Class";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClass(&wc);

    // Create a larger window to accommodate all elements
    HWND hwnd = CreateWindow(
        CLASS_NAME,
        "Click Counter with Timer",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 400,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    // Create the main clicking button
    buttonHandle = CreateWindow(
        "BUTTON",
        "Clicks: 0",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        50, 50, 300, 50,
        hwnd,
        (HMENU)1,
        hInstance,
        NULL
    );

    // Create timer buttons
    const char* timerLabels[] = {"15 sec", "30 sec", "1 min", "2 min"};
    for (int i = 0; i < 4; i++) {
        timerButtons[i] = CreateWindow(
            "BUTTON",
            timerLabels[i],
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            50 + (i * 80), 120, 70, 30,
            hwnd,
            (HMENU)(i + 2),  // IDs 2-5 for timer buttons
            hInstance,
            NULL
        );
    }

    // Create result label
    resultLabel = CreateWindow(
        "STATIC",
        "Click a timer button to start",
        WS_VISIBLE | WS_CHILD,
        50, 170, 300, 40,
        hwnd,
        NULL,
        hInstance,
        NULL
    );

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_COMMAND: {
            int buttonId = LOWORD(wParam);
            if (buttonId == 1 && isTimerRunning) {  // Main button clicked
                clickCount++;
                std::string buttonText = "Clicks: " + std::to_string(clickCount);
                SetWindowText(buttonHandle, buttonText.c_str());
            }
            else if (buttonId >= 2 && buttonId <= 5) {  // Timer buttons
                StartTimer(hwnd, TIMER_DURATIONS[buttonId - 2]);
            }
            break;
        }

        case WM_TIMER:
            if (wParam == TIMER_ID) {
                time_t currentTime = time(nullptr);
                int elapsed = static_cast<int>(currentTime - startTime);
                int remaining = timerDuration - elapsed;

                if (remaining <= 0) {
                    EndTimer(hwnd);
                }
                else {
                    // Update remaining time on the main button
                    std::string timeStr = "Time left: " + std::to_string(remaining) + "s - " +
                                        std::to_string(clickCount) + " clicks";
                    SetWindowText(buttonHandle, timeStr.c_str());
                }
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
            EndPaint(hwnd, &ps);
            return 0;
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
