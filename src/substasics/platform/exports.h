#ifndef SUBSTASICS_EXPORTS_H
#define SUBSTASICS_EXPORTS_H


#if defined(_LIB) || defined(STATIC_LIB)
# define SUBSTASICS_API
#else
# ifdef SUBSTASICS_EXPORTS
#  ifdef _WIN32
#   define SUBSTASICS_API __declspec (dllexport)
#  endif
# else
#  ifdef _WIN32
#   define SUBSTASICS_API __declspec (dllimport)
#  endif
# endif
#endif

// private stl members generate this warning.
#pragma warning (disable : 4251)
// must compile everything with the same compiler and c++ runtime
#pragma warning (disable : 4275)


#endif