#pragma once
#include <curl/curl.h>

class curl_environ
{
public:

    curl_environ()
        : sslset_result_(CURLSSLSET_OK)
        , init_result_(CURLE_OK)
    {
        sslset_result_ = curl_global_sslset(CURLSSLBACKEND_SCHANNEL, NULL, NULL);
        init_result_ = curl_global_init(CURL_GLOBAL_ALL);
    }

    ~curl_environ()
    {
        curl_global_cleanup();
    }

    operator bool() const
    {
        return sslset_result_ == CURLSSLSET_OK && init_result_ == CURLE_OK;
    }
private:
    CURLsslset sslset_result_;
    CURLcode init_result_;
};

