#ifdef _WIN32
# pragma once
#endif

#ifndef SUBSTASICS_PLATFORM_TEXT_H
#define SUBSTASICS_PLATFORM_TEXT_H


//
// These routines were orignally taken from a stack overflow post, but have since been modified
// to add utf8 support and fix a bug or two.
//

#include <string>

#include "substasics/platform/exports.h"

namespace substasics { namespace platform { namespace text {

	// convert to wide string
	// len will limit the length of the converted string
	SUBSTASICS_API std::wstring wide(const char *s, size_t len = -1);
	inline std::wstring wide(const std::string& s) {
		return wide(s.c_str(), s.size());
	}
	inline std::wstring wide(const wchar_t *s, size_t len = -1) {
		return len == (size_t)-1 ? s : std::wstring(s, len);
	}
	inline std::wstring wide(const std::wstring &s) {
		return s;
	}

	// convert to narrow string
	// len will limit the length of the converted string
	SUBSTASICS_API std::string narrow(const wchar_t *s, size_t len = -1); 
	inline std::string narrow(const std::wstring &s) {
		return narrow(s.c_str(), s.size());
	}
	inline std::string narrow(const char *s, size_t len = -1) {
		return len == (size_t)-1 ? s : std::string(s, len);
	}
	inline std::string narrow(const std::string &s) {
		return s;
	}

	// convert to utf8 string
	// len will limit the length of the converted string
	SUBSTASICS_API std::string utf8(const wchar_t *s, size_t len = -1);
	inline std::string utf8(const std::wstring &s) {
		return utf8(s.c_str(), s.size());
	}
	inline std::string utf8(const char *s, size_t len = -1) {
		return len == (size_t)-1 ? s : std::string(s, len);
	}
	inline std::string utf8(const std::string &s) {
		return s;
	}

	SUBSTASICS_API std::string trim(const std::string &toTrim);
	SUBSTASICS_API std::wstring trim(const std::wstring &toTrim);

}
}
}

#endif