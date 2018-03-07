#include "StdIncludes.h"
#include "Logger.h"
#include <sstream>
#include <ctime>
#include <cstdio>

Logger::Logger() {}

Logger* Logger::GetInstance()
{
	static Logger logger;
	return &logger;
}

const std::wstring Logger::_CurrentTime() const
{
	time_t now = time(0);
	return _wctime(&now);
}

void Logger::Write(Logger::MessageType messageType, const std::wstring& message)
{
	std::wstringstream format;
	format << L"[" << (messageType == MessageType::Info ? L"Info" : L"Error") << "]";
	format << L"(" << _CurrentTime() << L") ";
	format << message;
	m_messages.push_back(format.str());
}

void Logger::WriteInfo(const std::wstring& message)
{
	Write(MessageType::Info, message);
}

void Logger::WriteError(const std::wstring& message)
{
	Write(MessageType::Error, message);
}

std::wstring Logger::DumpContents()
{
	std::wstringstream stream;

	for (auto& message : m_messages)
	{
		stream << message << L"\r\n";
	}

	return stream.str();
}