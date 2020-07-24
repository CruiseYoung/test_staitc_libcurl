#pragma once
#include <Windows.h>
#include <string>
#include <LMErr.h>

//https://docs.microsoft.com/zh-cn/windows/win32/netmgmt/looking-up-text-for-error-code-numbers

class error_helper
{
public:
    error_helper() = delete;
    ~error_helper() = delete;

    static std::string get_sys_err_msgA(DWORD err_code);
    static std::wstring get_sys_err_msgW(DWORD err_code);
    static std::basic_string<TCHAR> get_sys_err_msg(DWORD err_code);
};

inline std::string error_helper::get_sys_err_msgA(DWORD err_code)
{
    std::string msg = std::to_string(err_code) + ":";
    DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS |
        FORMAT_MESSAGE_MAX_WIDTH_MASK;

    HMODULE module = NULL;
    if (NERR_BASE <= err_code && err_code <= MAX_NERR)
    {
        module = LoadLibraryExA("netmsg.dll", NULL, LOAD_LIBRARY_AS_DATAFILE);
        if (module != NULL)
        {
            flags |= FORMAT_MESSAGE_FROM_HMODULE;
        }
    }

    DWORD language_id = 0/*MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US)*//*MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL)*/;
    LPSTR msg_buf = NULL;

    DWORD msg_buf_len = FormatMessageA(flags, module, err_code, language_id, (LPSTR)&msg_buf, 0, NULL);
    if (msg_buf_len != 0)
    {
        msg.append(msg_buf, msg_buf_len);
    }

    if (msg_buf != NULL)
    {
        LocalFree(msg_buf);
    }

    if (module != NULL)
    {
        FreeLibrary(module);
    }

    return msg;
}

inline std::wstring error_helper::get_sys_err_msgW(DWORD err_code)
{
    std::wstring msg = std::to_wstring(err_code) + L":";
    DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS |
        FORMAT_MESSAGE_MAX_WIDTH_MASK;

    HMODULE module = NULL;
    if (NERR_BASE <= err_code && err_code <= MAX_NERR)
    {
        module = LoadLibraryExW(L"netmsg.dll", NULL, LOAD_LIBRARY_AS_DATAFILE);
        if (module != NULL)
        {
            flags |= FORMAT_MESSAGE_FROM_HMODULE;
        }
    }

    DWORD language_id = 0/*MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)*//*MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL)*/;
    LPWSTR msg_buf = NULL;

    DWORD msg_buf_len = FormatMessageW(flags, module, err_code, language_id, (LPWSTR)&msg_buf, 0, NULL);
    if (msg_buf_len != 0)
    {
        msg.append(msg_buf, msg_buf_len);
    }

    if (msg_buf != NULL)
    {
        LocalFree(msg_buf);
    }

    if (module != NULL)
    {
        FreeLibrary(module);
    }

    return msg;
}

inline std::basic_string<TCHAR> error_helper::get_sys_err_msg(DWORD err_code)
{
#if defined(UNICODE) || defined(_UNICODE)
    return get_sys_err_msgW(err_code);
#else
    return get_sys_err_msgA(err_code);
#endif
}
