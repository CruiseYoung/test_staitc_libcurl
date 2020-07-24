#ifndef XUNYOU_DLL_PARSER_WIN_H__
#define XUNYOU_DLL_PARSER_WIN_H__

#if defined(_MSC_VER) && (_MSC_VER > 1000)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER > 1000)

#include <map>
#include <string>
#include <cassert>
#if 1
#include <functional>
#include <type_traits>
#endif


#if defined(_WIN32) || defined(WINCE) || defined(MSDOS)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

//#define DllError() strerror(errno)
#define DllError() strerror(GetLastError())
//char* DllError()
//{
//    return strerror(errno);
//}
#else

#include <dlfcn.h>
typedef void *HMODULE;
typedef void *FARPROC;

#define LoadLibrary(x) dlopen(x, RTLD_NOW | RTLD_LOCAL)
#define FreeLibrary(x) dlclose(x)
#define GetProcAddress(x, y) dlsym(x, y)
#define DllError() dlerror()
//char* DllError()
//{
//    return dlerror();
//}
#endif


//namespace XY
//{

class dll_parser_win
{


public:

    dll_parser_win():handle_(NULL)
    {
    }

    ~dll_parser_win()
    {
        unload();
    }

    bool loadA(const std::string& dll_path)
    {
        handle_ = LoadLibraryA(dll_path.c_str());
        if (handle_ == NULL)
        {
            return false;
        }

        return true;
    }

    bool loadW(const std::wstring& dll_path)
    {
        handle_ = LoadLibraryW(dll_path.c_str());
        if (handle_ == NULL)
        {
            return false;
        }

        return true;
    }

#if defined(UNICODE) || defined(_UNICODE)
#define load loadW
#else
#define load loadA
#endif

    bool unload()
    {
        if (handle_ != NULL && !FreeLibrary(handle_))
        {
            return false;
        }

        handle_ = NULL;
        return true;
    }

    template <typename T, typename std::enable_if<
        std::is_pointer<T>::value &&
        !std::is_member_object_pointer<T>::value &&
        !std::is_member_function_pointer<T>::value, T>::type = nullptr>
    T get_function_pointee(const std::string& function_name)
    {
        assert(handle_ != NULL);
        std::map<std::string, FARPROC>::iterator it = function_pointee_map_.find(function_name);
        if (it == function_pointee_map_.end())
        {
            FARPROC addr = GetProcAddress(handle_, function_name.c_str());
            if (addr == NULL)
            {
                return NULL;
            }
            function_pointee_map_.insert(std::make_pair(function_name, addr));
            it = function_pointee_map_.find(function_name);
        }
        return reinterpret_cast<T>(it->second);
    }

    template <typename T, typename std::enable_if<std::is_function<T>::value, T>::type* = nullptr>
    std::function<T> get_function(const std::string& function_name)
    {
        T* function_pointee = get_function_pointee<T*>(function_name);
        if (function_pointee == NULL)
        {
            return nullptr;
        }
        return std::function<T>(function_pointee);
    }

#if 1
    template <typename T, typename... Args>
    typename std::result_of<std::function<T>(Args...)>::type
    excecute_function(const std::string& function_name, Args&&... args)
    {
        std::function<T> f = get_function<T>(function_name);
        if (f == nullptr)
        {
            std::string s = "can not find this function " + function_name;
            throw std::exception(s.c_str());
        }

        return f(std::forward<Args>(args)...);
    }
#endif

    std::string get_last_error()
    {
#pragma warning(disable: 4996)
        return DllError();
#pragma warning(default: 4996)
    }
private:
    HMODULE handle_;
    std::map<std::string, FARPROC> function_pointee_map_;
};

//} // namespace XY

#endif // XUNYOU_DLL_PARSER_WIN_H__
