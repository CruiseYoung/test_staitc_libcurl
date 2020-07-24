#ifndef __XXX_DLL_H__
#define __XXX_DLL_H__

#ifndef XXX_LIB
#   ifdef XXX_EXPORTS
#       define XXX_API extern "C" __declspec(dllexport)
#   else
#       define XXX_API extern "C" __declspec(dllimport)
#   endif
#else
#   define XXX_API
#endif


XXX_API bool UpdateEnvInit();
XXX_API void UpdateEnvUninit();

#endif // !__XXX_DLL_H__
