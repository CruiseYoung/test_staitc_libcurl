#include <future>
#include "dll_parser.h"
#include <tchar.h>

int main(int argc, char *argv[])
{
    {
        const auto test_future = std::async([]()
        {
            dll_parser_win dll;
            if (!dll.load(_T("xxx.dll"))) //LoadLibrary
            {
                return;
            }

            dll.excecute_function<bool()>("UpdateEnvInit");   // Curl_ossl_init
            dll.excecute_function<void()>("UpdateEnvUninit"); // Curl_ossl_cleanup

            return; // FreeLibrary
        });

        test_future.wait();
    }

    if (!DeleteFile(_T("xxx.dll")))
    {
        MessageBox(NULL, _T("xxx.dll cannot be deleted."), _T("Delete xxx.dll result"), 0);
    }
    else
    {
        MessageBox(NULL, _T("xxx.dll be deleted."), _T("Delete xxx.dll result"), 0);
    }
    //system("pause");
    return 0;
}

