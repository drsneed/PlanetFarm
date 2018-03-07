#include "StringOps.h"
#include <cvt/wstring>
#include <codecvt>


namespace
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
}

namespace StringOps
{

	auto FromWideString(const wstring& input) -> string
	{
		return converter.to_bytes(input);
	}

	auto ToWideString(const string& input) -> wstring
	{
		return converter.from_bytes(input);
	}

	auto UnicodeTo1252(const wstring& input)-> string
	{
		string output(input.size(), 0);
		if (!input.empty())
		{

			int success;
			WideCharToMultiByte(1252U, WC_NO_BEST_FIT_CHARS, &input[0], -1,
				&output[0], static_cast<int>(output.size()), "?", &success);
		}
		return output;
	}

	auto UnicodeTo1252(const wchar_t input)-> char
	{
		wstring tmp(1, input);
		return UnicodeTo1252(tmp)[0];
	}
}