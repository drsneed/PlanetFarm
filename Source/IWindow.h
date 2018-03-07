#pragma once
#include "StdIncludes.h"
#include "DebugTools.h"

template <typename T>
class IWindow
{
protected:
	HWND m_window = nullptr;
public:

	// Gets the class instance of a window from an HWND handle
	static T* GetThisFromHandle(HWND const window)
	{
		return reinterpret_cast<T*>(GetWindowLongPtr(window, GWLP_USERDATA));
	}

	static LRESULT __stdcall WndProc(HWND const window, UINT const message, WPARAM const wparam, LPARAM const lparam)
	{
		ASSERT(window);

		// WM_NCCREATE gets sent prior to WM_CREATE
		if (message == WM_NCCREATE)
		{
			CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lparam);
			T* that = static_cast<T*>(cs->lpCreateParams);
			ASSERT(that);
			ASSERT(!that->m_window);

			that->m_window = window;
			SetWindowLongPtr(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(that));
		}
		// All other messages get routed into the concrete class
		else if (T* that = GetThisFromHandle(window))
		{
			return that->MessageHandler(message, wparam, lparam);
		}

		return DefWindowProc(window, message, wparam, lparam);
	}

	// A default message handler, it does nothing but keep the message loop running
	virtual LRESULT MessageHandler(UINT const message, WPARAM const wparam, LPARAM const lparam)
	{
		if (message == WM_DESTROY)
		{
			PostQuitMessage(0);
		}
		else
		{
			return DefWindowProc(m_window, message, wparam, lparam);
		}
		return 0;
	}
};