#include "StdIncludes.h"
#include "GraphicsWindow.h"
#include "../../Embed/Resource.h"
#include "DebugTools.h"
#include "Logger.h"
#include <sstream>

#pragma comment(lib, "d3d11")
extern "C" IMAGE_DOS_HEADER __ImageBase;

#define ENABLE_4X_MSAA true

namespace
{
	GraphicsWindow* _instance = nullptr;
}

GraphicsWindow* GraphicsWindow::GetInstance()
{
	ASSERT(_instance);
	return _instance;
}

GraphicsWindow* GraphicsWindow::CreateInstance(int width, int height, bool fullscreen)
{
	ASSERT(!_instance);
	_instance = new GraphicsWindow(width, height, fullscreen);
	return _instance;
}

void GraphicsWindow::DestroyInstance()
{
	if (_instance)
	{
		delete _instance;
		_instance = nullptr;
	}
}

GraphicsWindow::GraphicsWindow(int width, int height, bool fullscreen)
	: m_width(width)
	, m_height(height)
	, m_device(nullptr)
	, m_context(nullptr)
	, m_swapChain(nullptr)
	, m_blendState(nullptr)
	, m_blendStateWithAlpha(nullptr)
	, m_depthEnabledStencilState(nullptr)
	, m_depthDisabledStencilState(nullptr)
	, m_standardSamplerState(nullptr)
	, m_repeatingSamplerState(nullptr)
	, m_wireframeRasterizerState(nullptr)
	, m_standardRasterizerState(nullptr)
	, m_scissorRasterizerState(nullptr)
	, m_renderTargetView(nullptr)
	, m_depthStencilView(nullptr)
	, m_depthStencilBuffer(nullptr)
	, m_active(true)
	, m_minimized(false)
	, m_maximized(false)
	, m_resizing(false)
	, m_mouseX(0)
	, m_mouseY(0)
	, m_mouseDiffX(0)
	, m_mouseDiffY(0)
{
	m_hInst = reinterpret_cast<HINSTANCE>(&__ImageBase);
	// Initialize global strings
	LoadStringW(m_hInst, IDS_APP_TITLE, m_title, MAX_LOADSTRING);
	
	wsprintf(m_windowClass, L"GameWindow");
	// Register class

	m_currentCursor = Cursor::Arrow;
	m_cursors[0] = LoadCursor(nullptr, IDC_ARROW);    // default cursor
	m_cursors[1] = LoadCursor(nullptr, IDC_CROSS);    // cross cursor
	m_cursors[2] = LoadCursor(nullptr, IDC_WAIT);     // wait cursor
	m_cursors[3] = LoadCursor(nullptr, IDC_IBEAM);    // text cursor

	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = m_hInst;
	wcex.hIcon = LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_VOXELPROJECT));
	wcex.hCursor = m_cursors[static_cast<int>(m_currentCursor)];
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = m_windowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	RegisterClassExW(&wcex);

	ASSERT(!m_window);
	ENSURE(CreateWindowW(m_windowClass, m_title, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, width, height,
		nullptr, nullptr, m_hInst, this));

	ASSERT(m_window);

	m_timer.Reset();
}

GraphicsWindow::~GraphicsWindow()
{
	if (m_depthStencilBuffer)
	{
		m_depthStencilBuffer->Release();
		m_depthStencilBuffer = nullptr;
	}

	if (m_depthStencilView)
	{
		m_depthStencilView->Release();
		m_depthStencilView = nullptr;
	}
	
	if (m_renderTargetView)
	{
		m_renderTargetView->Release();
		m_renderTargetView = nullptr;
	}

	if (m_scissorRasterizerState)
	{
		m_scissorRasterizerState->Release();
		m_scissorRasterizerState = nullptr;
	}

	if (m_standardRasterizerState)
	{
		m_standardRasterizerState->Release();
		m_standardRasterizerState = nullptr;
	}

	if (m_wireframeRasterizerState)
	{
		m_wireframeRasterizerState->Release();
		m_wireframeRasterizerState = nullptr;
	}

	if (m_repeatingSamplerState)
	{
		m_repeatingSamplerState->Release();
		m_repeatingSamplerState = nullptr;
	}

	if (m_standardSamplerState)
	{
		m_standardSamplerState->Release();
		m_standardSamplerState = nullptr;
	}

	if (m_depthDisabledStencilState)
	{
		m_depthDisabledStencilState->Release();
		m_depthDisabledStencilState = nullptr;
	}

	if (m_depthEnabledStencilState)
	{
		m_depthEnabledStencilState->Release();
		m_depthEnabledStencilState = nullptr;
	}

	if (m_blendState)
	{
		m_blendState->Release();
		m_blendState = nullptr;
	}

	if (m_blendStateWithAlpha)
	{
		m_blendStateWithAlpha->Release();
		m_blendStateWithAlpha = nullptr;
	}

	if (m_swapChain)
	{
		m_swapChain->Release();
		m_swapChain = nullptr;
	}

	if (m_context)
	{
		m_context->Release();
		m_context = nullptr;
	}

	if (m_device)
	{
		m_device->Release();
		m_device = nullptr;
	}
}

void GraphicsWindow::Show()
{
	ShowWindow(m_window, SW_SHOW);
	m_isOpen = true;
}



void GraphicsWindow::_D3DInitialize()
{
	if (!_D3DCreateDevice())
		return;

	if (!_D3DDeviceSupportsD3D11())
		return;

	if (!_D3DRetrieveMSAAQuality())
		return;

	if (!_D3DCreateSwapChain())
		return;

	if (!_D3DCreateBlendStates())
		return;

	if (!_D3DCreateDepthStencilStates())
		return;

	if (!_D3DCreateSamplerStates())
		return;

	if (!_D3DCreateRasterizerStates())
		return;

	if(!D3DErrorOccurred())
		_D3DSetDefaultStates();
}

bool GraphicsWindow::_D3DCreateDevice()
{
	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	if (!D3DCheck((
		D3D11CreateDevice
		(
			nullptr,				  // Default Adapter
			D3D_DRIVER_TYPE_HARDWARE, // We need some metal baby!
			nullptr,				  // NO SOFTWARE DEVICE!
			createDeviceFlags,		  // Flags
			nullptr,				  // Default feature level array
			0,						  // Zero feature levels passed in
			D3D11_SDK_VERSION,		  // SDK version
			&m_device,				  // GPU output
			&m_featureLevel,          // Feature level output
			&m_context			      // Context output
		)
	), L"D3D11CreateDevice")) return false;

	return true;
}

bool GraphicsWindow::_D3DDeviceSupportsD3D11()
{
	if (m_featureLevel != D3D_FEATURE_LEVEL_11_0)
	{
		Logger::GetInstance()->WriteError(L"D3D11 not supported on this machine.");
		// Release the device so Failed() returns true
		m_device->Release();
		m_device = nullptr;
		return false;
	}
	
	return true;
}

bool GraphicsWindow::_D3DRetrieveMSAAQuality()
{
	if (!D3DCheck(m_device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m_MSAAQuality),
		L"ID3D11Device::CheckMultisampleQualityLevels"))
		return false;

	ASSERT(m_MSAAQuality > 0);

	return true;
}

bool GraphicsWindow::_D3DCreateSwapChain()
{
	/*
	*  Let's describe the features we want in the swap chain
	*/
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = m_width;
	sd.BufferDesc.Height = m_height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.OutputWindow = m_window;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;

	if (ENABLE_4X_MSAA)
	{
		sd.SampleDesc.Count = 4;
		sd.SampleDesc.Quality = m_MSAAQuality - 1;
	}
	else
	{
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
	}

	/*
	 * Let's create the swap chain. In order to do so, we need to retrieve the dxgi device, and from that
	 * grab the dxgi adapter, and from that (Sheesh!) grab the dxgi factory, which is capable of creating
	 * a swap chain
	 */

	// Grab the dxgi device
	IDXGIDevice* dxgiDevice = nullptr;
	if (!D3DCheck(m_device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice)),
		L"ID3D11Device::QueryInterface")) return false;

	// Grab the dxgi adapter
	IDXGIAdapter* adapter = nullptr;
	if (!D3DCheck(dxgiDevice->GetAdapter(&adapter), L"IDXGIDevice::GetAdapter"))
	{
		dxgiDevice->Release();
		return false;
	}
		

	// Grab the dxgi factory
	IDXGIFactory* dxgiFactory = nullptr;
	if (!D3DCheck(adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory)),
		L"IDXGIAdapter::GetParent"))
	{
		adapter->Release();
		dxgiDevice->Release();
		return false;
	}

	// Finally, create the swap chain
	if (!D3DCheck(dxgiFactory->CreateSwapChain(m_device, &sd, &m_swapChain), L"IDXGIFactory1::CreateSwapChain"))
	{
		dxgiFactory->Release();
		adapter->Release();
		dxgiDevice->Release();
		return false;
	}

	// Cleanup
	dxgiFactory->Release();
	adapter->Release();
	dxgiDevice->Release();

	return true;
}

bool GraphicsWindow::_D3DCreateBlendStates()
{
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));

	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	/*	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_DEST_ALPHA;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = 0x0F;//D3D11_COLOR_WRITE_ENABLE_ALL;*/

	/*	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;*/

	//render to target
	//blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_GREEN | D3D11_COLOR_WRITE_ENABLE_BLUE;

	if (!D3DCheck(m_device->CreateBlendState(&blendDesc, &m_blendStateWithAlpha), L"ID3D11Device::CreateBlendState (With Alpha)"))
		return false;

	blendDesc.RenderTarget[0].BlendEnable = FALSE;
	if (!D3DCheck(m_device->CreateBlendState(&blendDesc, &m_blendState), L"ID3D11Device::CreateBlendState (No Alpha)"))
		return false;

	return true;
}

bool GraphicsWindow::_D3DCreateDepthStencilStates()
{

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	// Initialize the description of the stencil state.
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	// Set up the description of the stencil state.
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;


	if (!D3DCheck(m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthEnabledStencilState),
		L"ID3D11Device::CreateDepthStencilState (depth enabled)")) return false;

	

	depthStencilDesc.DepthEnable = false;
	if (!D3DCheck(m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthDisabledStencilState),
		L"ID3D11Device::CreateDepthStencilState (depth disabled)")) return false;

	return true;
}

bool GraphicsWindow::_D3DCreateSamplerStates()
{

	D3D11_SAMPLER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.MipLODBias = 0.0f;
	desc.MaxAnisotropy = 1;
	desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	desc.BorderColor[0] = 0;
	desc.BorderColor[1] = 0;
	desc.BorderColor[2] = 0;
	desc.BorderColor[3] = 0;
	desc.MinLOD = 0;
	desc.MaxLOD = D3D11_FLOAT32_MAX;

	if(!D3DCheck(m_device->CreateSamplerState(&desc, &m_standardSamplerState), 
		L"ID3D11Device::CreateSamplerState (standard)")) return false;

	desc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;

	if (!D3DCheck(m_device->CreateSamplerState(&desc, &m_repeatingSamplerState),
		L"ID3D11Device::CreateSamplerState (standard)")) return false;

	return true;
}

bool GraphicsWindow::_D3DCreateRasterizerStates()
{
	D3D11_RASTERIZER_DESC rsStateDesc;

	ZeroMemory(&rsStateDesc, sizeof(rsStateDesc));
	rsStateDesc.FillMode = D3D11_FILL_WIREFRAME;
	rsStateDesc.CullMode = D3D11_CULL_BACK;
	//rsStateDesc.CullMode = D3D11_CULL_NONE;
	rsStateDesc.FrontCounterClockwise = false;
	rsStateDesc.DepthBias = 0;
	rsStateDesc.DepthBiasClamp = 0.0f;
	rsStateDesc.SlopeScaledDepthBias = 0.0f;
	rsStateDesc.DepthClipEnable = true;
	rsStateDesc.ScissorEnable = false;
	rsStateDesc.MultisampleEnable = true;
	rsStateDesc.AntialiasedLineEnable = false;
	
	if(!D3DCheck(m_device->CreateRasterizerState(&rsStateDesc, &m_wireframeRasterizerState),
		L"ID3D11Device::CreateRasterizerState (wireframe)")) return false;

	rsStateDesc.FillMode = D3D11_FILL_SOLID;

	if (!D3DCheck(m_device->CreateRasterizerState(&rsStateDesc, &m_standardRasterizerState),
		L"ID3D11Device::CreateRasterizerState (standard)")) return false;

	rsStateDesc.ScissorEnable = true;

	if (!D3DCheck(m_device->CreateRasterizerState(&rsStateDesc, &m_scissorRasterizerState),
		L"ID3D11Device::CreateRasterizerState (scissor)")) return false;
	return true;
}

void GraphicsWindow::_D3DResizeEvent(int width, int height)
{
	ASSERT(m_context);
	ASSERT(m_device);
	ASSERT(m_swapChain);

	if(m_renderTargetView)
		m_renderTargetView->Release();

	if (m_depthStencilView)
		m_depthStencilView->Release();
	
	if (m_depthStencilBuffer)
		m_depthStencilBuffer->Release();

	// resize the swap chain and recreate the render target view
	if (!D3DCheck(m_swapChain->ResizeBuffers(1, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0), 
		L"ID3D11SwapChain::ResizeBuffers")) return;

	ID3D11Texture2D* backBuffer;

	if (!D3DCheck(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)), 
		L"ID3D11SwapChain::GetBuffer")) return;
	if (!D3DCheck(m_device->CreateRenderTargetView(backBuffer, nullptr, &m_renderTargetView), 
		L"ID3D11Device::CreateRenderTargetView")) return;

	backBuffer->Release();

	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = width;
	depthStencilDesc.Height = height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// Use 4X MSAA? --must match swap chain MSAA values.
	if (ENABLE_4X_MSAA)
	{
		depthStencilDesc.SampleDesc.Count = 4;
		depthStencilDesc.SampleDesc.Quality = m_MSAAQuality - 1;
	}
	// No MSAA
	else
	{
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
	}


	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	if(!D3DCheck(m_device->CreateTexture2D(&depthStencilDesc, nullptr, &m_depthStencilBuffer), 
		L"ID3D11Device::CreateTexture2D (depth buffer)")) return;
	if(!D3DCheck(m_device->CreateDepthStencilView(m_depthStencilBuffer, nullptr, &m_depthStencilView),
		L"ID3D11Device::CreateDepthStencilView") ) return;

	SetRenderTarget(m_renderTargetView, width, height);
}

void GraphicsWindow::SetRenderTarget(ID3D11RenderTargetView* renderTargetView, int width, int height)
{
	// Set the viewport transform.
	m_screenViewport.TopLeftX = 0;
	m_screenViewport.TopLeftY = 0;
	m_screenViewport.Width = static_cast<float>(width);
	m_screenViewport.Height = static_cast<float>(height);
	m_screenViewport.MinDepth = 0.0f;
	m_screenViewport.MaxDepth = 1.0f;

	ASSERT(m_context);
	m_context->RSSetViewports(1, &m_screenViewport);


	// Clear the current render target
	ID3D11RenderTargetView* dummyTarget = nullptr;
	m_context->OMSetRenderTargets(1, &dummyTarget, nullptr);

	if (renderTargetView == m_renderTargetView)
	{
		// include depth buffer in this case
		m_context->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);
	}
	else
	{
		m_context->OMSetRenderTargets(1, &renderTargetView, nullptr);
	}

}

void GraphicsWindow::_D3DSetDefaultStates()
{
	SetRasterizerState(RasterizerState::Fill);
	EnableDepthTest(true);
}

void GraphicsWindow::EnableAlphaBlending(bool enable)
{
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	if (enable)
	{
		m_context->OMSetBlendState(m_blendStateWithAlpha, blendFactor, 0xFFFFFFFF);
	}
	else
	{
		m_context->OMSetBlendState(m_blendState, blendFactor, 0xFFFFFFFF);
	}
}

void GraphicsWindow::SetRasterizerState(const RasterizerState state)
{
	ASSERT(m_context);

	switch (state)
	{
	case RasterizerState::Wireframe:
		m_context->RSSetState(m_wireframeRasterizerState);
		break;
	case RasterizerState::Scissor:
		m_context->RSSetState(m_scissorRasterizerState);
		break;
	default:
		m_context->RSSetState(m_standardRasterizerState);
	}
}

void GraphicsWindow::EnableDepthTest(const bool enable)
{
	m_context->OMSetDepthStencilState(
		enable ? m_depthEnabledStencilState : m_depthDisabledStencilState, 1);
}

void GraphicsWindow::Clear(const FLOAT* rgba)
{
	ASSERT(m_context);
	ASSERT(m_swapChain);

	m_context->ClearRenderTargetView(m_renderTargetView, rgba);
	m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

}

void GraphicsWindow::Present()
{
	D3DCheck(m_swapChain->Present(0, 0), L"ID3D11SwapChain::Present");
}

void GraphicsWindow::Close()
{
	DestroyWindow(m_window);
}

void GraphicsWindow::GetSize(int& outWidth, int& outHeight) const
{
	RECT rect;
	GetClientRect(m_window, &rect);
	outWidth = rect.right - rect.left;
	outHeight = rect.bottom - rect.top;
}

bool GraphicsWindow::IsKeyDown(int key) const
{
	return (GetAsyncKeyState(key) & 0x8000) != 0;
}

void GraphicsWindow::ShowCursor(bool show)
{
	::ShowCursor(show);
}

void GraphicsWindow::Tick()
{
	MSG message;
	// Process Win32 messages
	while (PeekMessage(&message, m_window, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	m_timer.Tick();
}

LRESULT GraphicsWindow::MessageHandler(const UINT message, const WPARAM wparam, const LPARAM lparam)
{
	static int tmpWidthHolder = 0;
	static int tmpHeightHolder = 0;

	switch (message)
	{
	case WM_DESTROY:
		// Don't send WM_QUIT if there are errors because that will force the subsequent
		// error window to close prematurely
		if (!D3DErrorOccurred())
		{
			PostQuitMessage(0);
		}
		m_isOpen = false;
		break;
	case WM_PAINT:
		PAINTSTRUCT ps;
		ENSURE(BeginPaint(m_window, &ps));
		EndPaint(m_window, &ps);
		break;
	case WM_CREATE:
		_D3DInitialize();
		break;
	case WM_SIZE:
	{
		auto width = LOWORD(lparam);
		auto height = HIWORD(lparam);
		//_ResizeRenderTargetAndSwapChain();
		if (wparam == SIZE_MINIMIZED)
		{
			m_minimized = true;
			m_maximized = false;
		}
		else if (wparam == SIZE_MAXIMIZED)
		{
			m_minimized = false;
			m_maximized = true;
		}
		else if (wparam == SIZE_RESTORED)
		{
			// Restoring from minimized state?
			if (m_minimized)
			{
				m_minimized = false;
			}
			else if (m_maximized)
			{
				m_maximized = false;
			}

			else if (m_resizing)
			{
				// If user is dragging the resize bars, we do not resize 
				// the buffers here because as the user continuously 
				// drags the resize bars, a stream of WM_SIZE messages are
				// sent to the window, and it would be pointless (and slow)
				// to resize for each WM_SIZE message received from dragging
				// the resize bars.  So instead, we reset after the user is 
				// done resizing the window and releases the resize bars, which 
				// sends a WM_EXITSIZEMOVE message.
			}
			else
			{
				Event event;
				event.type = Event::Type::WindowResize;
				event.w = width;
				event.h = height;
				m_eventStack.push(event);
			}
		}
		break;
	}
	case WM_ENTERSIZEMOVE:
	{
		GetSize(tmpWidthHolder, tmpHeightHolder);
		m_timer.Stop();
		break;
	}
	// Catch this message to prevent the window from becoming too small
	case WM_GETMINMAXINFO:
	{
		reinterpret_cast<MINMAXINFO*>(lparam)->ptMinTrackSize.x = 200;
		reinterpret_cast<MINMAXINFO*>(lparam)->ptMinTrackSize.y = 200;
		break;
	}
	case WM_EXITSIZEMOVE:
	{
		m_timer.Start();
		// Resize and Move happen with same event. Here we check if size changed
		int newWidth, newHeight;
		GetSize(newWidth, newHeight);
		
		if (newWidth != tmpWidthHolder || newHeight != tmpHeightHolder)
		{
			Event event;
			event.type = Event::Type::WindowResize;
			event.w = newWidth;
			event.h = newHeight;
			m_eventStack.push(event);
		}
		break;
	}

	// The WM_MENUCHAR message is sent when a menu is active and the user presses
	// a key that does not correspond to any mnemonic or accelerator key
	case WM_MENUCHAR:
	{
		// Don't beep when we alt-enter
		MAKELRESULT(0, MNC_CLOSE);
		break;
	}

	case WM_MOUSEMOVE:
	{
		int newMouseX = static_cast<int>(LOWORD(lparam));
		int newMouseY = static_cast<int>(HIWORD(lparam));
		m_mouseDiffX = newMouseX - m_mouseX;
		m_mouseDiffY = newMouseY - m_mouseY;
		m_mouseX = newMouseX;
		m_mouseY = newMouseY;
		Event event;
		event.type = Event::Type::MouseMotion;
		event.x = m_mouseX;
		event.y = m_mouseY;
		m_eventStack.push(event);
		break;
	}

	case WM_MOUSEWHEEL:
	{
		int value = static_cast<int>(HIWORD(wparam)) / 120;
		Event event;
		if (value == 1)
			event.type = Event::Type::MouseWheelUp;
		else
			event.type = Event::Type::MouseWheelDown;
		m_eventStack.push(event);
		break;
	}
	case WM_LBUTTONDOWN:
	{
		Event event;
		event.type = Event::Type::MouseClick;
		event.code = Event::Code::MouseLeft;
		event.x = static_cast<int>(LOWORD(lparam));
		event.y = static_cast<int>(HIWORD(lparam));
		m_eventStack.push(event);

		break;
	}

	case WM_LBUTTONDBLCLK:
	{
		Event event;
		event.type = Event::Type::MouseDoubleClick;
		event.code = Event::Code::MouseLeft;
		event.x = static_cast<int>(LOWORD(lparam));
		event.y = static_cast<int>(HIWORD(lparam));
		m_eventStack.push(event);

		break;
	}
	case WM_LBUTTONUP:
	{
		Event event;
		event.type = Event::Type::MouseRelease;
		event.code = Event::Code::MouseLeft;
		event.x = static_cast<int>(LOWORD(lparam));
		event.y = static_cast<int>(HIWORD(lparam));
		m_eventStack.push(event);

		break;
	}

	case WM_RBUTTONDOWN:
	{
		Event event;
		event.type = Event::Type::MouseClick;
		event.code = Event::Code::MouseRight;
		event.x = static_cast<int>(LOWORD(lparam));
		event.y = static_cast<int>(HIWORD(lparam));
		m_eventStack.push(event);

		break;
	}
	case WM_RBUTTONUP:
	{
		Event event;
		event.type = Event::Type::MouseRelease;
		event.code = Event::Code::MouseRight;
		event.x = static_cast<int>(LOWORD(lparam));
		event.y = static_cast<int>(HIWORD(lparam));
		m_eventStack.push(event);

		break;
	}

	// Mouse wheel button down event
	case WM_MBUTTONDOWN:
	{
		Event event;
		event.type = Event::Type::MouseClick;
		event.code = Event::Code::MouseMiddle;
		event.x = static_cast<int>(LOWORD(lparam));
		event.y = static_cast<int>(HIWORD(lparam));
		m_eventStack.push(event);

		break;
	}

	// Mouse wheel button up event
	case WM_MBUTTONUP:
	{
		Event event;
		event.type = Event::Type::MouseRelease;
		event.code = Event::Code::MouseMiddle;
		event.x = static_cast<int>(LOWORD(lparam));
		event.y = static_cast<int>(HIWORD(lparam));
		m_eventStack.push(event);

		break;
	}

	// Keydown event
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	{
		bool keyrepeat = true;
		if (keyrepeat || ((HIWORD(lparam) & KF_REPEAT) == 0))
		{
			Event event;
			event.type = Event::Type::KeyPress;
			event.code = _TranslateEventCode(wparam, lparam);
			m_eventStack.push(event);

		}
		break;
	}

	// Keyup event
	case WM_KEYUP:
	case WM_SYSKEYUP:
	{
		Event event;
		event.type = Event::Type::KeyRelease;
		event.code = _TranslateEventCode(wparam, lparam);
		m_eventStack.push(event);

		break;
	}

	case WM_CHAR:
	{
		Event event;
		event.type = Event::Type::CharInput;
		event.Character = static_cast<wchar_t>(wparam);
		m_eventStack.push(event);
		break;
	}
	case WM_SETCURSOR:
	{
		::SetCursor(m_cursors[static_cast<int>(m_currentCursor)]);
		return TRUE;
	}

	default:
		return __super::MessageHandler(message, wparam, lparam);
	}

	return 0;
}

void GraphicsWindow::GetMouseFrameDifference(int& diffX, int& diffY)
{
	diffX = m_mouseDiffX;
	diffY = m_mouseDiffY;
}

void GraphicsWindow::GetMousePosition(int& mouseX, int& mouseY)
{
	mouseX = m_mouseX;
	mouseY = m_mouseY;
}

auto GraphicsWindow::GetCurrentCursor() -> Cursor
{
	return m_currentCursor;
}

auto GraphicsWindow::SetCurrentCursor(const GraphicsWindow::Cursor cursor) -> void
{
	m_currentCursor = cursor;
}

void GraphicsWindow::SetMousePosition(int x, int y)
{
	m_mouseX = x;
	m_mouseY = y;
	POINT pt;
	pt.x = x;
	pt.y = y;
	ClientToScreen(m_window, &pt);
	ENSURE(SetCursorPos(pt.x, pt.y));
	// Suppress the subsequent mouse move messages
	MSG msg;
	while (::PeekMessage(&msg, NULL, WM_MOUSEMOVE, WM_MOUSEMOVE, PM_REMOVE)) {};
	GetMousePosition(m_mouseX, m_mouseY);
	m_mouseDiffX = 0;
	m_mouseDiffY = 0;
}

bool GraphicsWindow::PollEvent(GraphicsWindow::Event& event)
{
	if (m_eventStack.empty())
	{
		return false;
	}

	event = m_eventStack.top();

	if (event.type == Event::Type::WindowResize)
	{
		_D3DResizeEvent(event.w, event.h);
	}

	m_eventStack.pop();
	return true;
}

GraphicsWindow::Event::Code GraphicsWindow::_TranslateEventCode(WPARAM key, LPARAM flags)
{
	switch (key)
	{
		// Check the scancode to distinguish between left and right shift
	case VK_SHIFT:
	{
		static UINT lShift = MapVirtualKey(VK_LSHIFT, MAPVK_VK_TO_VSC);
		auto scancode = static_cast<UINT>((flags & (0xFF << 16)) >> 16);
		return scancode == lShift ? Event::Code::ShiftLeft : Event::Code::ShiftRight;
	}
	// Check the "extended" flag to distinguish between left and right alt
	case VK_MENU: return (HIWORD(flags) & KF_EXTENDED) ? Event::Code::AltLeft : Event::Code::AltRight;
		// Check the "extended" flag to distinguish between left and right control
	case VK_CONTROL: return (HIWORD(flags) & KF_EXTENDED) ? Event::Code::CtrlRight : Event::Code::CtrlLeft;

	case VK_LWIN:       return Event::Code::SysLeft;
	case VK_RWIN:       return Event::Code::SysRight;
	case VK_APPS:       return Event::Code::Menu;
	case VK_OEM_1:      return Event::Code::Semicolon;
	case VK_OEM_2:      return Event::Code::Forwardslash;
	case VK_OEM_PLUS:   return Event::Code::Plus;
	case VK_OEM_MINUS:  return Event::Code::Dash;
	case VK_OEM_4:      return Event::Code::BracketLeft;
	case VK_OEM_6:      return Event::Code::BracketRight;
	case VK_OEM_COMMA:  return Event::Code::Comma;
	case VK_OEM_PERIOD: return Event::Code::Period;
	case VK_OEM_7:      return Event::Code::Quote;
	case VK_OEM_5:      return Event::Code::Backslash;
	case VK_OEM_3:      return Event::Code::Tilde;
	case VK_ESCAPE:     return Event::Code::Escape;
	case VK_SPACE:      return Event::Code::Space;
	case VK_RETURN:     return Event::Code::Return;
	case VK_BACK:       return Event::Code::Backspace;
	case VK_TAB:        return Event::Code::Tab;
	case VK_PRIOR:      return Event::Code::PageUp;
	case VK_NEXT:       return Event::Code::PageDown;
	case VK_END:        return Event::Code::End;
	case VK_HOME:       return Event::Code::Home;
	case VK_INSERT:     return Event::Code::Insert;
	case VK_DELETE:     return Event::Code::Delete;
	case VK_NUMLOCK:    return Event::Code::NumLock;
	case VK_ADD:        return Event::Code::Add;
	case VK_SUBTRACT:   return Event::Code::Subtract;
	case VK_MULTIPLY:   return Event::Code::Multiply;
	case VK_DIVIDE:     return Event::Code::Divide;
	case VK_PAUSE:      return Event::Code::Pause;
	case VK_F1:         return Event::Code::F1;
	case VK_F2:         return Event::Code::F2;
	case VK_F3:         return Event::Code::F3;
	case VK_F4:         return Event::Code::F4;
	case VK_F5:         return Event::Code::F5;
	case VK_F6:         return Event::Code::F6;
	case VK_F7:         return Event::Code::F7;
	case VK_F8:         return Event::Code::F8;
	case VK_F9:         return Event::Code::F9;
	case VK_F10:        return Event::Code::F10;
	case VK_F11:        return Event::Code::F11;
	case VK_F12:        return Event::Code::F12;
	case VK_F13:        return Event::Code::F13;
	case VK_F14:        return Event::Code::F14;
	case VK_F15:        return Event::Code::F15;
	case VK_LEFT:       return Event::Code::Left;
	case VK_RIGHT:      return Event::Code::Right;
	case VK_UP:         return Event::Code::Up;
	case VK_DOWN:       return Event::Code::Down;
	case VK_NUMPAD0:    return Event::Code::Numpad0;
	case VK_NUMPAD1:    return Event::Code::Numpad1;
	case VK_NUMPAD2:    return Event::Code::Numpad2;
	case VK_NUMPAD3:    return Event::Code::Numpad3;
	case VK_NUMPAD4:    return Event::Code::Numpad4;
	case VK_NUMPAD5:    return Event::Code::Numpad5;
	case VK_NUMPAD6:    return Event::Code::Numpad6;
	case VK_NUMPAD7:    return Event::Code::Numpad7;
	case VK_NUMPAD8:    return Event::Code::Numpad8;
	case VK_NUMPAD9:    return Event::Code::Numpad9;
	case 'A':           return Event::Code::A;
	case 'Z':           return Event::Code::Z;
	case 'E':           return Event::Code::E;
	case 'R':           return Event::Code::R;
	case 'T':           return Event::Code::T;
	case 'Y':           return Event::Code::Y;
	case 'U':           return Event::Code::U;
	case 'I':           return Event::Code::I;
	case 'O':           return Event::Code::O;
	case 'P':           return Event::Code::P;
	case 'Q':           return Event::Code::Q;
	case 'S':           return Event::Code::S;
	case 'D':           return Event::Code::D;
	case 'F':           return Event::Code::F;
	case 'G':           return Event::Code::G;
	case 'H':           return Event::Code::H;
	case 'J':           return Event::Code::J;
	case 'K':           return Event::Code::K;
	case 'L':           return Event::Code::L;
	case 'M':           return Event::Code::M;
	case 'W':           return Event::Code::W;
	case 'X':           return Event::Code::X;
	case 'C':           return Event::Code::C;
	case 'V':           return Event::Code::V;
	case 'B':           return Event::Code::B;
	case 'N':           return Event::Code::N;
	case '0':           return Event::Code::Num0;
	case '1':           return Event::Code::Num1;
	case '2':           return Event::Code::Num2;
	case '3':           return Event::Code::Num3;
	case '4':           return Event::Code::Num4;
	case '5':           return Event::Code::Num5;
	case '6':           return Event::Code::Num6;
	case '7':           return Event::Code::Num7;
	case '8':           return Event::Code::Num8;
	case '9':           return Event::Code::Num9;
	}

	return Event::Code::Count;
}