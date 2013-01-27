#include "substasics/platform/text.h"
#include <stdint.h>
#include <boost/shared_array.hpp>

#include <wchar.h>

#ifdef _WIN32
#include <Windows.h>

namespace substasics { namespace platform { namespace text {

	// windows-specific conversion routines using safe string functions
	std::wstring wide(const char *s, size_t len)
	{
		// figure out how many wide characters we are going to get 
		size_t converted_len;
		mbstowcs_s(&converted_len, NULL, 0, s, INT_MAX);

		// convert the narrow tstring to a wide string
		boost::shared_array<wchar_t> buf(new wchar_t[converted_len]);
		mbstowcs_s(NULL, buf.get(), converted_len, s, converted_len-1);

		if (len != (size_t)-1 && len > 0 && static_cast<uint32_t>(len) < converted_len)
		{
			converted_len = len;
		}

		std::wstring converted_string(buf.get(), converted_len);
		return converted_string;
	}

	std::string narrow(const wchar_t *s, size_t len)
	{
		// figure out how many narrow characters we are going to get
		size_t converted_len;
		wcstombs_s(&converted_len, NULL, 0, s, INT_MAX);
	
		// convert the wide tstring to a narrow string
		boost::shared_array<char> buf(new char[converted_len]);
		wcstombs_s(NULL, buf.get(), converted_len, s, converted_len-1);

		if (len != (size_t)-1 && len > 0 && static_cast<uint32_t>(len) < converted_len)
		{
			converted_len = len;
		}

		std::string converted_string(buf.get(), converted_len);
		return converted_string; 
	}

	std::string utf8(const wchar_t *s, size_t len)
	{
		int converted_len = WideCharToMultiByte(CP_UTF8, 0, s, len == static_cast<size_t>(-1) ? -1 : static_cast<int>(len), NULL, 0, NULL, NULL);

		boost::shared_array<char> buf(new char[converted_len]);
		WideCharToMultiByte(CP_UTF8, 0, s, len == static_cast<size_t>(-1) ? -1 : static_cast<int>(len), buf.get(), converted_len, NULL, NULL);
		std::string converted_string(buf.get(), converted_len);

		return converted_string;
	}

	std::string trim(const std::string &toTrim)
	{
		size_t idx = 0;
		size_t len = toTrim.size();
		size_t size = toTrim.size();

		while (idx < size && isspace(toTrim[idx]))
		{
			++idx; --len;
		}
		
		int endIdx = static_cast<int>(idx+len-1);
		while (endIdx >= 0 && isspace(toTrim[idx+len-1]))
		{
			--len;
			endIdx = static_cast<int>(idx+len-1);
		}

		if (idx > len)
		{
			return "";
		}
		return toTrim.substr(idx, len);
	}

	std::wstring text::trim(const std::wstring &toTrim)
	{
		size_t idx = 0;
		size_t len = toTrim.size();
		size_t size = toTrim.size();

		while (idx < size && iswspace(toTrim[idx]))
		{
			++idx; --len;
		}
		
		int endIdx = static_cast<int>(idx+len-1);
		while (endIdx >= 0 && iswspace(toTrim[endIdx]))
		{
			--len;
			endIdx = static_cast<int>(idx+len-1);
		}

		if (idx > len)
		{
			return L"";
		}
		return toTrim.substr(idx, len);
	}

}
}
}

#else
# error "TODO: add platform specific macros for text conversion"
#endif
