#pragma once

#include <d3d11_1.h>
#pragma comment(lib, "d3d11.lib")

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

#include <assert.h>

class Engine
{

private:
    ID3D11Device1 *d3d11Device;
    ID3D11DeviceContext1 *d3d11DeviceContext;
    IDXGISwapChain1 *d3d11SwapChain;
    ID3D11RenderTargetView *d3d11FrameBufferView;

    ID3D11VertexShader *vertexShader;
    ID3D11PixelShader *pixelShader;

    ID3D11InputLayout *inputLayout;
    ID3D11Buffer *vertexBuffer;
    ID3DBlob *vsBlob;

private:
    UINT numVerts;
    UINT stride;
    UINT offset;

private:
    void CreateDeviceAndContext();
    void CreateSwapChain(HWND hwnd);
    void CreateRenderTarget();
    void CreateShaders();
    void CreateInputLayout();

public:
    ~Engine();

    void Init(HWND hwnd);
    void Render(HWND hwnd, bool &global_windowDidResize);
    void SetVertexData(float *vertexData, UINT Stride, UINT NumVerts, UINT Offset);
    // DEPRECATED
    void SetVertexBuffer(D3D11_BUFFER_DESC vertexBufferDesc, D3D11_SUBRESOURCE_DATA vertexSubresourceData, UINT Stride, UINT NumVerts, UINT Offset);

public:
    bool ShouldRun = true;
    FLOAT backgroundColor[4] = {0.5f, 0.3f, 0.6f, 1.0f};
};