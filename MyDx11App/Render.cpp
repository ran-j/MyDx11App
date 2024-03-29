#include "Render.h"

void Render::Init(HWND hwnd)
{
    CreateDeviceAndContext();
    CreateSwapChain(hwnd);
    CreateRenderTarget();
    CreateShaders();
    CreateInputLayout();
}

Render::~Render()
{
    if (d3d11Device)
        d3d11Device->Release();
    if (d3d11DeviceContext)
        d3d11DeviceContext->Release();
    if (d3d11SwapChain)
        d3d11SwapChain->Release();
    if (d3d11FrameBufferView)
        d3d11FrameBufferView->Release();
    if (vertexShader)
        vertexShader->Release();
    if (pixelShader)
        pixelShader->Release();
    if (inputLayout)
        inputLayout->Release();
    if (vertexBuffer)
        vertexBuffer->Release();
    if (vsBlob)
        vsBlob->Release();
}

void Render::CreateDeviceAndContext()
{
    ID3D11Device *baseDevice;
    ID3D11DeviceContext *baseDeviceContext;
    D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_11_0};
    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(DEBUG_BUILD)
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    HRESULT hResult = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE,
                                        0, creationFlags,
                                        featureLevels, ARRAYSIZE(featureLevels),
                                        D3D11_SDK_VERSION, &baseDevice,
                                        0, &baseDeviceContext);
    if (FAILED(hResult))
    {
        MessageBoxA(0, "D3D11CreateDevice() failed", "Fatal Error", MB_OK);
    }

    hResult = baseDevice->QueryInterface(__uuidof(ID3D11Device1), (void **)&d3d11Device);
    assert(SUCCEEDED(hResult));

    baseDevice->Release();

    hResult = baseDeviceContext->QueryInterface(__uuidof(ID3D11DeviceContext1), (void **)&d3d11DeviceContext);
    assert(SUCCEEDED(hResult));

    baseDeviceContext->Release();

#ifdef DEBUG_BUILD
    // Set up debug layer to break on D3D11 errors
    ID3D11Debug *d3dDebug = nullptr;
    d3d11Device->QueryInterface(__uuidof(ID3D11Debug), (void **)&d3dDebug);
    if (d3dDebug)
    {
        ID3D11InfoQueue *d3dInfoQueue = nullptr;
        if (SUCCEEDED(d3dDebug->QueryInterface(__uuidof(ID3D11InfoQueue), (void **)&d3dInfoQueue)))
        {
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
            d3dInfoQueue->Release();
        }
        d3dDebug->Release();
    }
#endif
}

void Render::CreateSwapChain(HWND hwnd)
{
    // Get DXGI Factory (needed to create Swap Chain)
    IDXGIFactory2 *dxgiFactory;
    {
        IDXGIDevice1 *dxgiDevice;
        HRESULT hResult = d3d11Device->QueryInterface(__uuidof(IDXGIDevice1), (void **)&dxgiDevice);
        assert(SUCCEEDED(hResult));

        IDXGIAdapter *dxgiAdapter;
        hResult = dxgiDevice->GetAdapter(&dxgiAdapter);
        assert(SUCCEEDED(hResult));
        dxgiDevice->Release();

        DXGI_ADAPTER_DESC adapterDesc;
        dxgiAdapter->GetDesc(&adapterDesc);

        OutputDebugStringA("Graphics Device: ");
        OutputDebugStringW(adapterDesc.Description);

        hResult = dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), (void **)&dxgiFactory);
        assert(SUCCEEDED(hResult));
        dxgiAdapter->Release();
    }

    DXGI_SWAP_CHAIN_DESC1 d3d11SwapChainDesc = {};
    d3d11SwapChainDesc.Width = 0;  // use window width
    d3d11SwapChainDesc.Height = 0; // use window height
    d3d11SwapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
    d3d11SwapChainDesc.SampleDesc.Count = 1;
    d3d11SwapChainDesc.SampleDesc.Quality = 0;
    d3d11SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    d3d11SwapChainDesc.BufferCount = 2;
    d3d11SwapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    d3d11SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    d3d11SwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    d3d11SwapChainDesc.Flags = 0;

    HRESULT hResult = dxgiFactory->CreateSwapChainForHwnd(d3d11Device, hwnd, &d3d11SwapChainDesc, 0, 0, &d3d11SwapChain);
    assert(SUCCEEDED(hResult));

    dxgiFactory->Release();
}

void Render::CreateRenderTarget()
{
    // Get back buffer from swap chain
    ID3D11Texture2D *backBuffer;
    HRESULT hResult = d3d11SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&backBuffer);
    assert(SUCCEEDED(hResult));

    // Create render target view
    hResult = d3d11Device->CreateRenderTargetView(backBuffer, 0, &d3d11FrameBufferView);
    assert(SUCCEEDED(hResult));

    backBuffer->Release();
}

void Render::CreateShaders()
{
    // Create Vertex Shader
    {
        ID3DBlob *shaderCompileErrorsBlob;
        HRESULT hResult = D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "vs_main", "vs_5_0", 0, 0, &vsBlob, &shaderCompileErrorsBlob);

        if (FAILED(hResult))
        {
            const char *errorString = NULL;
            if (hResult == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
            {
                errorString = "Could not compile shader; file not found";
            }
            else if (shaderCompileErrorsBlob)
            {
                errorString = (const char *)shaderCompileErrorsBlob->GetBufferPointer();
                shaderCompileErrorsBlob->Release();
            }
            MessageBoxA(0, errorString, "Shader Compiler Error", MB_ICONERROR | MB_OK);
        }

        hResult = d3d11Device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vertexShader);
        assert(SUCCEEDED(hResult));
    }

    // Create Pixel Shader
    {
        ID3DBlob *psBlob;
        ID3DBlob *shaderCompileErrorsBlob;

        HRESULT hResult = D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "ps_main", "ps_5_0", 0, 0, &psBlob, &shaderCompileErrorsBlob);

        if (FAILED(hResult))
        {
            const char *errorString = NULL;
            if (hResult == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
            {
                errorString = "Could not compile shader; file not found";
            }
            else if (shaderCompileErrorsBlob)
            {
                errorString = (const char *)shaderCompileErrorsBlob->GetBufferPointer();
                shaderCompileErrorsBlob->Release();
            }
            MessageBoxA(0, errorString, "Shader Compiler Error", MB_ICONERROR | MB_OK);
        }

        hResult = d3d11Device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixelShader);
        assert(SUCCEEDED(hResult));

        psBlob->Release();
    }
}

void Render::CreateInputLayout()
{
    D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
        {
            {"POS", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"COL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        };

    HRESULT hResult = d3d11Device->CreateInputLayout(inputElementDesc, ARRAYSIZE(inputElementDesc), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout);
    assert(SUCCEEDED(hResult));

    vsBlob->Release();
}

ID3D11Buffer *Render::CreateVertexBuffer(D3D11_BUFFER_DESC vertexBufferDesc, D3D11_SUBRESOURCE_DATA vertexSubresourceData)
{
    ID3D11Buffer *vertexBuffer;
    HRESULT hResult = d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexSubresourceData, &vertexBuffer);
    assert(SUCCEEDED(hResult));
    return vertexBuffer;
}

void Render::RenderLoop(HWND hwnd, bool &global_windowDidResize)
{

    if (global_windowDidResize)
    {
        d3d11DeviceContext->OMSetRenderTargets(0, 0, 0);
        d3d11FrameBufferView->Release();

        HRESULT res = d3d11SwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
        assert(SUCCEEDED(res));

        ID3D11Texture2D *d3d11FrameBuffer;
        res = d3d11SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&d3d11FrameBuffer);
        assert(SUCCEEDED(res));

        res = d3d11Device->CreateRenderTargetView(d3d11FrameBuffer, NULL,
                                                  &d3d11FrameBufferView);
        assert(SUCCEEDED(res));
        d3d11FrameBuffer->Release();

        global_windowDidResize = false;
    }

    d3d11DeviceContext->ClearRenderTargetView(d3d11FrameBufferView, backgroundColor);

    RECT winRect;
    GetClientRect(hwnd, &winRect);

    D3D11_VIEWPORT viewport = {0.0f, 0.0f, (FLOAT)(winRect.right - winRect.left), (FLOAT)(winRect.bottom - winRect.top), 0.0f, 1.0f};
    d3d11DeviceContext->RSSetViewports(1, &viewport);

    d3d11DeviceContext->OMSetRenderTargets(1, &d3d11FrameBufferView, nullptr);

    d3d11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    d3d11DeviceContext->IASetInputLayout(inputLayout);

    d3d11DeviceContext->VSSetShader(vertexShader, nullptr, 0);
    d3d11DeviceContext->PSSetShader(pixelShader, nullptr, 0);

    // d3d11DeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    UINT numVerts = 0;
    for (int i = 0; i < vertexData.size(); i++)
    {
        numVerts += vertexData[i].numVerts;
        d3d11DeviceContext->IASetVertexBuffers(vertexData[i].startSlot, 1, &vertexData[i].buffer, &vertexData[i].stride, &vertexData[i].offset);
    }

    d3d11DeviceContext->Draw(numVerts, 0);

    d3d11SwapChain->Present(1, 0);
}