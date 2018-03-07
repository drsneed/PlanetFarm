#include "DebugTools.h"
#include "UniqueHandle.h"
#include "Logger.h"
#include <sstream>
#include <iomanip>

namespace
{
	bool _d3dErrorOccurred = false;
}

std::wstring GetHRESULTErrorMessage(const HRESULT code)
{
	auto local = UniqueHandle<LocalMessageTraits>{};
	const auto flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
	auto localBuf = local.Get();
	auto size = FormatMessage(flags, nullptr, code, 0, reinterpret_cast<wchar_t*>(local.GetAddrOf()), 0, nullptr);
	while (size && iswspace(*(local.Get() + size - 1)))
	{
		--size;
	}

	if (size == 0)
	{
		return L"I blame the weather!";
	}

	return std::wstring{ local.Get(), local.Get() + size };
}

std::wstring GetSystemErrorMessage(const DWORD code)
{
	// Retrieve the system error message for the last-error code
	LPVOID lpMsgBuf;
	DWORD lastError = GetLastError();
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		lastError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPTSTR>(&lpMsgBuf),
		0,
		nullptr);
	std::wstring output{ reinterpret_cast<LPTSTR>(lpMsgBuf) };
	LocalFree(lpMsgBuf);
	return output;
}

bool D3DCheck(const HRESULT hr, const WCHAR* functionName)
{
	if (hr != S_OK)
	{
		auto hrString = GetHRESULTErrorMessage(hr);
		auto logger = Logger::GetInstance();
		logger->WriteError(L"%s: HRESULT 0x%x, %s", functionName, hr, hrString);
		_d3dErrorOccurred = true;
		return false;
	}

	return true;
}

bool D3DErrorOccurred()
{
	return _d3dErrorOccurred;
}

void SetD3DErrorOccurred()
{
	_d3dErrorOccurred = true;
}