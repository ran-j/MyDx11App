// MyDx11App.cpp : Defines the entry point for the application.
//
#include "MyDx11App.h"

#include <windows.h>
#include "Render.h"

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

    // Initialize the Render
    Render *render = new Render();
    render->Init(hwnd);

    // SetVertexData
    {
        UINT numFloatsPerVertex = 3; // número de floats por vértice

        float vertexData1[] = {
            0.0f, 0.5f, 0.f, // Vertex 1 position
            1.f, 0.f, 1.f,   // Vertex 1 color

            0.5f, -0.5f, 1.f, // Vertex 2 position
            0.f, 0.f, 1.f,    // Vertex 2 color

            -0.5f, -0.5f, 0.0f, // Vertex 3 position
            0.f, 1.f, 1.f       // Vertex 3 color

        };

        UINT numFloatsInVertexData1 = sizeof(vertexData1) / sizeof(float);

        RenderVertex vertex1;
        vertex1.stride = 6 * sizeof(float);
        vertex1.offset = 0;
        vertex1.startSlot = 1;
        vertex1.numVerts = numFloatsInVertexData1 / numFloatsPerVertex;

        D3D11_BUFFER_DESC vertexBufferDesc1 = {};
        vertexBufferDesc1.ByteWidth = sizeof(vertexData1);
        vertexBufferDesc1.Usage = D3D11_USAGE_IMMUTABLE;
        vertexBufferDesc1.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA vertexSubresourceData1 = {vertexData1};

        auto vertex1Buffer = render->CreateVertexBuffer(vertexBufferDesc1, vertexSubresourceData1);

        vertex1.buffer = vertex1Buffer;

        // DATA 2 $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
        float vertexData2[] = {
            0.0f, 0.5f, 0.f, // Vertex 1 position
            1.f, 0.f, 1.f,   // Vertex 1 color

            0.5f, -0.5f, 1.f, // Vertex 2 position
            0.f, 0.f, 1.f,    // Vertex 2 color

            -0.5f, -0.5f, 0.0f, // Vertex 3 position
            0.f, 1.f, 1.f       // Vertex 3 color
        };

        UINT numFloatsInVertexData2 = sizeof(vertexData2) / sizeof(float);

        RenderVertex vertex2;
        vertex2.stride = 6 * sizeof(float);
        vertex2.offset = 0;
        vertex2.startSlot = 0;
        vertex2.numVerts = numFloatsInVertexData2 / numFloatsPerVertex;

        D3D11_BUFFER_DESC vertexBufferDesc2 = {};
        vertexBufferDesc2.ByteWidth = sizeof(vertexData2);
        vertexBufferDesc2.Usage = D3D11_USAGE_IMMUTABLE;
        vertexBufferDesc2.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA vertexSubresourceData2 = {vertexData2};

        auto vertex2Buffer = render->CreateVertexBuffer(vertexBufferDesc2, vertexSubresourceData2);

        vertex2.buffer = vertex2Buffer;

        render->AddVertexData(vertex1);
        render->AddVertexData(vertex2);
    }

    // Main loop
    while (render->ShouldRun)
    {
        MSG msg = {};
        while (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                render->ShouldRun = false;
            }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        render->RenderLoop(hwnd, global_windowDidResize);
    }

    return 0;
}