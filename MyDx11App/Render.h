#pragma once

#include <d3d11_1.h>
#pragma comment(lib, "d3d11.lib")

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

#include <assert.h>
#include <vector>

struct RenderVertex
{
    UINT stride;
    UINT numVerts;
    UINT offset;
    UINT startSlot;
    ID3D11Buffer *buffer;
};

class Render
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

    std::vector<RenderVertex> vertexData;

private:
    void CreateDeviceAndContext();
    void CreateSwapChain(HWND hwnd);
    void CreateRenderTarget();
    void CreateShaders();
    void CreateInputLayout();

public:
    ~Render();

    void Init(HWND hwnd);
    void RenderLoop(HWND hwnd, bool &global_windowDidResize);
    ID3D11Buffer *CreateVertexBuffer(D3D11_BUFFER_DESC vertexBufferDesc, D3D11_SUBRESOURCE_DATA vertexSubresourceData);

    inline void AddVertexData(RenderVertex data) { vertexData.push_back(data); }

public:
    bool ShouldRun = true;
    FLOAT backgroundColor[4] = {0.5f, 0.3f, 0.6f, 1.0f};
};