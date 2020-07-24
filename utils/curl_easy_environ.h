#pragma once
#include <curl/curl.h>

class curl_easy_environ
{
public:

    curl_easy_environ()
        : curl_(NULL)
    {
        curl_ = curl_easy_init();
    }

    ~curl_easy_environ()
    {
        if (curl_ != NULL)
        {
            curl_easy_cleanup(curl_);
            curl_ = NULL;
        }
    }

    operator bool()
    {
        return curl_ != NULL;
    }

    operator CURL*()
    {
        return curl_;
    }

    CURL* curl()
    {
        return curl_;
    }

private:
    CURL* curl_;
};

