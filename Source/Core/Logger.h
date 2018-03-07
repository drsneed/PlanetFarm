#pragma once
#include "StdIncludes.h"
#include <string>
#include <vector>

class Logger
{
public:
	enum class MessageType : uint8_t
	{
		Error,
		Info
	};

	Logger();
	const std::wstring _CurrentTime() const;
	std::vector<std::wstring> m_messages;

public:
	static Logger* GetInstance();

	void Write(MessageType messageType, const std::wstring& message);
	void WriteInfo(const std::wstring& message);
	void WriteError(const std::wstring& message);

	template <typename... Args>
	auto WriteError(const wchar_t* format, Args... args) -> void
	{
		wchar_t buffer[512];
		_snwprintf_s(buffer, _countof(buffer), _countof(buffer) - 1, format, args...);
		Write(MessageType::Error, buffer);
	}

	template <typename... Args>
	auto WriteInfo(const wchar_t* format, Args... args) -> void
	{
		wchar_t buffer[512];
		_snwprintf_s(buffer, _countof(buffer), _countof(buffer) - 1, format, args...);
		Write(MessageType::Info, buffer);
	}

	std::wstring DumpContents();

};