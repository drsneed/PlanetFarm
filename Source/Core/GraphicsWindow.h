#pragma once

#include "StdIncludes.h"
#include "IWindow.h"
#include <d3d11.h>
#include <wrl.h>
#include <stack>
#include "Timer.h"
#include <sciter-x.h>

#define MAX_LOADSTRING 100
using namespace Microsoft::WRL;

class GraphicsWindow : public IWindow<GraphicsWindow>
{
public:
	enum class Cursor
	{
		Arrow,
		Cross,
		Wait,
		Text
	};

	struct Event
	{
		enum class Type : int8_t
		{
			WindowResize,
			MouseMotion,
			MouseWheelUp,
			MouseWheelDown,
			KeyPress,
			KeyRelease,
			MouseClick,
			MouseDoubleClick,
			MouseRelease,
			Quit,
			CharInput
		};

		Type type;

		enum class Code : int8_t
		{
			A, B, C, D, E, F, G, H, I, J, K, L, M,
			N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
			Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
			F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, F13, F14, F15,
			Dash, Plus, Backspace, Tilde, Tab, Escape,
			Space, CtrlLeft, CtrlRight, AltLeft, AltRight, ShiftLeft,
			ShiftRight, SysLeft, SysRight, Menu, Insert, Delete,
			End, Home, PageUp, PageDown, Up, Left, Right, Down,
			PrintScreen, ScrollLock, Pause, NumLock, Forwardslash,
			Backslash, BracketLeft, BracketRight, Semicolon, Comma, Period,
			Quote, Return, Add, Subtract, Multiply, Divide,
			Numpad0, Numpad1, Numpad2, Numpad3, Numpad4, Numpad5,
			Numpad6, Numpad7, Numpad8, Numpad9,
			MouseLeft, MouseMiddle, MouseRight, Count
		};

		Code code;
		union
		{
			struct { int x, y; };
			struct { int w, h; };
			struct { wchar_t Character, _unused_1, _unused_2, _unused_3; };
		};
	};

private:

	HINSTANCE m_hInst;                   // current instance
	WCHAR m_title[MAX_LOADSTRING];       // The title bar text
	WCHAR m_windowClass[MAX_LOADSTRING]; // the main window class name
	bool m_isOpen = false;


	struct DomEventHandler : public sciter::event_handler 
	{
		BEGIN_FUNCTION_MAP
			FUNCTION_1("setRotationSpeed", setRotationSpeed)
			FUNCTION_1("setColorSpeed", setColorSpeed)
		END_FUNCTION_MAP

		sciter::value setRotationSpeed(sciter::value speed)
		{
			//g_rotationSpeed = (FLOAT)speed.get(1.0);
			return sciter::value(true);
		}
		sciter::value setColorSpeed(sciter::value speed)
		{
			//g_colorSpeed = (FLOAT)speed.get(1.0);
			return sciter::value(true);
		}

	};
	
	DomEventHandler m_dom_event_handler;
	//HELEMENT m_dom_back_layer;
	HELEMENT m_layer1;

	D3D_DRIVER_TYPE m_driverType;
	D3D_FEATURE_LEVEL m_featureLevel;
	ID3D11Device* m_device;
	ID3D11DeviceContext* m_context;
	IDXGISwapChain* m_swapChain;
	ID3D11BlendState* m_blendState;
	ID3D11BlendState* m_blendStateWithAlpha;
	ID3D11DepthStencilState* m_depthEnabledStencilState;
	ID3D11DepthStencilState* m_depthDisabledStencilState;
	ID3D11SamplerState* m_standardSamplerState;
	ID3D11SamplerState* m_repeatingSamplerState;
	ID3D11RasterizerState* m_wireframeRasterizerState;
	ID3D11RasterizerState* m_standardRasterizerState;
	ID3D11RasterizerState* m_scissorRasterizerState;
	ID3D11RenderTargetView* m_renderTargetView;
	ID3D11DepthStencilView* m_depthStencilView;
	ID3D11Texture2D* m_depthStencilBuffer;
	D3D11_VIEWPORT m_screenViewport;
	UINT m_MSAAQuality;
	int m_width, m_height;
	bool m_active;
	bool m_minimized;
	bool m_maximized;
	bool m_resizing;
	int m_mouseX, m_mouseY;
	int m_mouseDiffX, m_mouseDiffY;
	std::stack<Event> m_eventStack;
	HCURSOR m_cursors[4];
	GraphicsWindow::Cursor m_currentCursor;
	Timer m_timer;
	void _D3DInitialize();
	bool _D3DCreateDevice();
	bool _D3DDeviceSupportsD3D11();
	bool _D3DRetrieveMSAAQuality();
	bool _D3DCreateSwapChain();
	bool _D3DCreateBlendStates();
	bool _D3DCreateDepthStencilStates();
	bool _D3DCreateSamplerStates();
	bool _D3DCreateRasterizerStates();
	void _D3DSetDefaultStates();
	void _D3DResizeEvent(int width, int height);
	GraphicsWindow(int width, int height, bool fullscreen);

	Event::Code _TranslateEventCode(WPARAM key, LPARAM flags);

	BOOL _InitSciterEngine();


public:
	static GraphicsWindow* GetInstance();
	static GraphicsWindow* CreateInstance(int width, int height, bool fullscreen);
	static void DestroyInstance();
	~GraphicsWindow();

	void Show();
	void Close();
	void Tick();
	const Timer* GetTimer() const { return &m_timer; }
	void Clear(const FLOAT* rgba);
	BOOL RenderSciterUI();
	void Present();
	bool IsOpen() const { return m_isOpen; }
	bool IsMinimized() const { return m_minimized; }
	HWND GetHandle() const { return m_window; }
	HINSTANCE GetHInstance() const { return m_hInst; }
	void GetSize(int& outWidth, int& outHeight) const;
	virtual LRESULT MessageHandler(const UINT message, const WPARAM wparam, const LPARAM lparam) override;
	bool PollEvent(Event& event);
	auto GetCurrentCursor() -> Cursor;
	auto SetCurrentCursor(const GraphicsWindow::Cursor cursor) -> void;
	void SetMousePosition(int x, int y);
	bool IsKeyDown(int key) const;
	static void ShowCursor(bool show);
	enum class RasterizerState : uint8_t
	{
		Fill,
		Wireframe,
		Scissor
	};
	void GetMouseFrameDifference(int& diffX, int& diffY);
	void GetMousePosition(int& mouseX, int& mouseY);
	void SetRasterizerState(const RasterizerState state);
	void EnableAlphaBlending(bool enable);
	ID3D11SamplerState* GetStandardSamplerState() const { return m_standardSamplerState; }
	void EnableDepthTest(const bool enable);
	void SetRenderTarget(ID3D11RenderTargetView* renderTargetView, int width, int height);
	ID3D11Device* GetDevice() const { return m_device; }
	ID3D11DeviceContext* GetContext() const { return m_context; }
};