// MyDx11App.cpp : Defines the entry point for the application.
//
#include "MyDx11App.h"

#include <windows.h>
#include "Engine.h"

static bool global_windowDidResize = false;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    LRESULT result = 0;
    switch (msg)
    {
    case WM_KEYDOWN:
    {
        if (wparam == VK_ESCAPE)
            DestroyWindow(hwnd);
        break;
    }
    case WM_DESTROY:
    {
        PostQuitMessage(0);
        break;
    }
    case WM_SIZE:
    {
        global_windowDidResize = true;
        break;
    }
    default:
        result = DefWindowProcW(hwnd, msg, wparam, lparam);
    }
    return result;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nShowCmd*/)
{
    // Open a window
    HWND hwnd;
    {
        WNDCLASSEXW winClass = {};
        winClass.cbSize = sizeof(WNDCLASSEXW);
        winClass.style = CS_HREDRAW | CS_VREDRAW;
        winClass.lpfnWndProc = &WndProc;
        winClass.hInstance = hInstance;
        winClass.hIcon = LoadIconW(0, IDI_APPLICATION);
        winClass.hCursor = LoadCursorW(0, IDC_ARROW);
        winClass.lpszClassName = L"MyWindowClass";
        winClass.hIconSm = LoadIconW(0, IDI_APPLICATION);

        if (!RegisterClassExW(&winClass))
        {
            MessageBoxA(0, "RegisterClassEx failed", "Fatal Error", MB_OK);
            return GetLastError();
        }

        RECT initialRect = {0, 0, 1024, 768};
        AdjustWindowRectEx(&initialRect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_OVERLAPPEDWINDOW);
        LONG initialWidth = initialRect.right - initialRect.left;
        LONG initialHeight = initialRect.bottom - initialRect.top;

        hwnd = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW,
                               winClass.lpszClassName,
                               L"My Dx APP",
                               WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                               CW_USEDEFAULT, CW_USEDEFAULT,
                               initialWidth,
                               initialHeight,
                               0, 0, hInstance, 0);

        if (!hwnd)
        {
            MessageBoxA(0, "CreateWindowEx failed", "Fatal Error", MB_OK);
            return GetLastError();
        }
    }

    // Initialize the engine
    Engine *engine = new Engine();
    engine->Init(hwnd);

    // SetVertexData
    {
        UINT numVerts;
        UINT stride;
        UINT offset;
        float vertexData[] = {// x, y, r, g, b, a
                              0.0f, 0.5f, 0.f, 1.f, 0.f, 1.f,
                              0.5f, -0.5f, 1.f, 0.f, 0.f, 1.f,
                              -0.5f, -0.5f, 0.f, 0.f, 1.f, 1.f};
        stride = 6 * sizeof(float);
        numVerts = sizeof(vertexData) / stride;
        offset = 0;
        engine->SetVertexData(vertexData, stride, numVerts, offset);
    }

    // Main loop
    while (engine->ShouldRun)
    {
        MSG msg = {};
        while (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                engine->ShouldRun = false;
            }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        engine->Render(hwnd, global_windowDidResize);
    }

    return 0;
}