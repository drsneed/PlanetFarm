#pragma once

#include "StdIncludes.h"
#include <crtdbg.h>
#include <string>

#define ASSERT _ASSERTE
#ifdef _DEBUG
#define ENSURE ASSERT

#define ENSURE_(a, b) (ASSERT(a == b))

struct Tracer
{
	const char* m_filename;
	unsigned m_line;
	bool m_doPrependFileLineInfo;
	Tracer(const char* filename, unsigned line)
		: m_filename{ filename }
		, m_line{ line }
		, m_doPrependFileLineInfo{ m_filename != nullptr }
	{
	}

	template <typename... Args>
	auto operator()(const wchar_t* format, Args... args) const -> void
	{
		wchar_t buffer[512];
		if (m_doPrependFileLineInfo)
		{
			auto count = swprintf_s(buffer, L"%S(%d): ", m_filename, m_line);
			ASSERT(count != -1);
			_snwprintf_s(buffer + count, _countof(buffer) - count, _countof(buffer) - count - 1, format, args...);
		}
		else
		{
			_snwprintf_s(buffer, _countof(buffer), _countof(buffer) - 1, format, args...);
		}
		OutputDebugString(buffer);
	}
};

#define TRACE Tracer(__FILE__, __LINE__)
#define PRINTF Tracer(nullptr,0)
#else
#define TRACE wprintf
#define PRINTF __noop
#define DEBUG_OUTPUT __noop
#define ENSURE(expression) (expression)
#define ENSURE_(badvalue, expression) (expression)
#endif

#define NO_COPY(T) T(const T&) = delete; T& operator=(const T&) = delete
#define NO_MOVE(T) T(T&&) = delete; T& operator=(T&&) = delete

std::wstring GetHRESULTErrorMessage(const HRESULT code);
std::wstring GetSystemErrorMessage(const DWORD code);
bool D3DCheck(const HRESULT hr, const WCHAR* functionName);
bool D3DErrorOccurred();
void SetD3DErrorOccurred();
