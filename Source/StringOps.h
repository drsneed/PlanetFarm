#pragma once
#include "StdIncludes.h"

#include <cctype>
#include <locale>
#include <algorithm>
#include <functional>

namespace StringOps
{
	auto FromWideString(const wstring& input)->string;
	auto ToWideString(const string& input)->wstring;

	auto UnicodeTo1252(const wstring& input)->string;
	auto UnicodeTo1252(const wchar_t input) -> char;

	// trim from start
	static inline string& LTrim(string &s)
	{
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
		return s;
	}

	// trim from end
	static inline string &RTrim(std::string &s)
	{
		s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
		return s;
	}

	// trim from both ends
	static inline string &Trim(std::string &s)
	{
		return LTrim(RTrim(s));
	}
}