#include "http.h"
#include <cassert>
#include <algorithm>
#include "error_helper.h"

struct response_code_msg
{
    long code;
    const char* msg;
};

struct response_code_msg k_resp_code_msg[]
{
    { 100, "Continue"                          },
    { 101, "Switching Protocols"               },
    { 200, "OK"                                },
    { 201, "Created"                           },
    { 202, "Accepted"                          },
    { 203, "Non-Authoritative Information"     },
    { 204, "No Content"                        },
    { 205, "Reset Content"                     },
    { 206, "Partial Content"                   },
    { 300, "Multiple Choices"                  },
    { 301, "Moved Permanently"                 },
    { 302, "Found"                             },
    { 303, "See Other"                         },
    { 304, "Not Modified"                      },
    { 305, "Use Proxy"                         },
    { 307, "Temporary Redirect"                },
    { 400, "Bad Request"                       },
    { 401, "Unauthorized"                      },
    { 402, "Payment Required"                  },
    { 403, "Forbidden"                         },
    { 404, "Not Found"                         },
    { 405, "Method Not Allowed"                },
    { 406, "Not Acceptable"                    },
    { 407, "Proxy Authentication Required"     },
    { 408, "Request Time-out"                  },
    { 409, "Conflict"                          },
    { 410, "Gone"                              },
    { 411, "Length Required"                   },
    { 412, "Precondition Failed"               },
    { 413, "Request Entity Too Large"          },
    { 414, "Request-URI Too Large"             },
    { 415, "Unsupported Media Type"            },
    { 416, "Requested range not satisfiable"   },
    { 417, "Expectation Failed"                },
    { 500, "Internal Server Error"             },
    { 501, "Not Implemented"                   },
    { 502, "Bad Gateway"                       },
    { 503, "Service Unavailable"               },
    { 504, "Gateway Time-out"                  },
    { 505, "Version Not Supported"             }
};

http::http()
    : curl_(easy_env.curl())
    , res_(CURLE_OK)
{
    memset(err_buf_, 0, sizeof(err_buf_));
}

http::~http()
{
}

std::string http::easy_strerror() const
{
    if (res_ != CURLE_OK)
    {
        return  "[" + (std::to_string(res_) + ":") + curl_easy_strerror(res_) + "]";
    }

    if (strlen(err_buf_) != 0)
    {
        return err_buf_;
    }

    return "";
}

void http::set_strerror(long response_code)
{
    if (strlen(err_buf_) != 0)
    {
        return;
    }

    for (size_t i=0; i<sizeof(k_resp_code_msg)/sizeof(k_resp_code_msg[0]); ++i)
    {
        if (k_resp_code_msg[i].code == response_code)
        {
            sprintf_s(err_buf_, sizeof(err_buf_), "[%d:%s]", response_code, k_resp_code_msg[i].msg);
        }
    }
}

void http::easy_reset()
{
    curl_easy_reset(curl_);
}

CURLcode http::easy_perform()
{
    return (res_ = curl_easy_perform(curl_));
}

long http::get_response_code()
{
    long response_code = 0;
    res_ = curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &response_code);
    return response_code;
}

CURLcode http::set_base_option()
{
    res_ = CURLE_OK;
    if ((res_ = set_error_buffer()) != CURLE_OK ||
        (res_ = set_redirect()) != CURLE_OK ||
        (res_ = set_skip_all_signal()) != CURLE_OK ||
        (res_ = set_timeout()) != CURLE_OK)
    {
        return res_;
    }
   return CURLE_OK;
}

CURLcode http::set_error_buffer()
{
    assert(curl_ != NULL);
    memset(err_buf_, 0, sizeof(err_buf_));
    return (res_ = curl_easy_setopt(curl_, CURLOPT_ERRORBUFFER, err_buf_));
}

std::string http::get_redirect(const std::string& url)
{
    std::string empty_url;
    easy_reset();
    res_ = CURLE_OK;
    if ((res_ = set_url(url)) != CURLE_OK ||
        (res_ = easy_perform()) != CURLE_OK)
    {
        return empty_url;
    }

    uint32_t response_code = 0;
    if ((res_ = curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &response_code)) != CURLE_OK ||
        (response_code / 100) == 3)
    {
        return url; //Not a redirect.
    }

    char* new_absolute_url = NULL;
    if ((res_ = curl_easy_getinfo(curl_, CURLINFO_REDIRECT_URL, &new_absolute_url)) != CURLE_OK)
    {
        return empty_url;
    }

    return new_absolute_url;
}

std::string http::http_get(const std::string& url)
{
    std::string empty_response;

    easy_reset();
    res_ = CURLE_OK;

#ifdef _DEBUG
    std::string debug_info;
    if ((res_ = set_debug(debug_info, debug_callback)) != CURLE_OK)
    {
        return empty_response;
    }
#endif

    std::string response;
    if ((res_ = set_url(url)) != CURLE_OK ||
        (res_ = set_base_option()) != CURLE_OK ||
        (res_ = set_no_certification()) != CURLE_OK ||
        (res_ = set_write_data((void*)&response, response_write_data_save_to_string)) != CURLE_OK ||
        (res_ = easy_perform()) != CURLE_OK)
    {
        return empty_response;
    }

    if (res_ != CURLE_OK)
    {
        return empty_response;
    }

    long response_code = get_response_code();
    if (response_code != 200)
    {
        set_strerror(response_code);
        return empty_response;
    }

    return response;
}

std::string http::http_post(const std::string& url, const std::string& post_fields/* = ""*/)
{
    std::string empty_response;

    easy_reset();
    res_ = CURLE_OK;

#ifdef _DEBUG
    std::string debug_info;
    if ((res_ = set_debug(debug_info, debug_callback)) != CURLE_OK)
    {
        return empty_response;
    }
#endif

    std::string response;
    if ((res_ = set_url(url)) != CURLE_OK ||
        (res_ = set_base_option()) != CURLE_OK ||
        (res_ = set_no_certification()) != CURLE_OK ||
        (res_ = set_post(post_fields)) != CURLE_OK ||
        (res_ = set_write_data((void*)&response, response_write_data_save_to_string)) != CURLE_OK ||
        (res_ = easy_perform()) != CURLE_OK)
    {
        return empty_response;
    }

    if (res_ != CURLE_OK)
    {
        return empty_response;
    }

    long response_code = get_response_code();
    if (response_code != 200)
    {
        set_strerror(response_code);
        return empty_response;
    }

    return response;
}

uint64_t http::get_size_download(const std::string& url)
{
    uint64_t size_download_default = (std::numeric_limits<decltype(size_download_default)>::max)();

    easy_reset();
    res_ = CURLE_OK;

#ifdef _DEBUG
    std::string debug_info;
    if ((res_ = set_debug(debug_info, debug_callback)) != CURLE_OK)
    {
        return size_download_default;
    }
#endif

    //std::string response;
    if ((res_ = set_url(url)) != CURLE_OK ||
        (res_ = set_base_option()) != CURLE_OK ||
        (res_ = set_no_certification()) != CURLE_OK ||
        (res_ = curl_easy_setopt(curl_, CURLOPT_HEADER, 1L)) != CURLE_OK ||
        (res_ = curl_easy_setopt(curl_, CURLOPT_NOBODY, 1L)) != CURLE_OK ||
        (res_ = easy_perform()) != CURLE_OK)
    {
        return size_download_default;
    }

    long response_code = 0;
    res_ = curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &response_code);
    if (res_ != CURLE_OK || response_code != 200)
    {
        return size_download_default;
    }

    // size = -1 if no Content-Length return or Content-Length=0
    curl_off_t size_download = -1;
    res_ = curl_easy_getinfo(curl_, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &size_download);
    if (res_ != CURLE_OK)
    {
        return size_download_default;
    }

    return size_download;
}

bool http::http_download(const std::string& url, const std::string& dest_path,
    void* progress_context/* = NULL*/, int(*progress_cb)(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) /*= NULL*/)
{
    res_ = CURLE_OK;
    easy_reset();

#ifdef _DEBUG
    std::string debug_info;
    if ((res_ = set_debug(debug_info, debug_callback)) != CURLE_OK)
    {
        return false;
    }
#endif
    FILE* stream = fopen(dest_path.c_str(), "wb");
    if (stream == NULL)
    {
        std::string err_info =  "[" + error_helper::get_sys_err_msgA(GetLastError()) + "]";
        if (err_info.size() >= sizeof(err_buf_))
        {
            err_info.resize(sizeof(err_buf_) - 1);
        }

        memcpy(err_buf_, err_info.data(), err_info.size());

        return false;
    }

    do
    {
        if ((res_ = set_url(url)) != CURLE_OK ||
            (res_ = set_base_option()) != CURLE_OK ||
            (res_ = set_no_certification()) != CURLE_OK ||
            (res_ = set_write_data((void*)stream, response_write_data_save_to_file)) != CURLE_OK ||
            (res_ = set_progress((void*)progress_context, progress_cb)) != CURLE_OK)
        {
            break;
        }

        res_ = easy_perform();
    } while (0);

    fclose(stream);

    do
    {
        if (res_ != CURLE_OK)
        {
            break;
        }

        long response_code = get_response_code();
        if (response_code != 200)
        {
            set_strerror(response_code);
            break;
        }

        return true;
    } while (0);

    remove(dest_path.c_str());
    return false;
}

// 注意与http_get的比较
std::string http::http_download(const std::string& url)
{
    res_ = CURLE_OK;
    easy_reset();

    std::string data;
#ifdef _DEBUG
    std::string debug_info;
    if ((res_ = set_debug(debug_info, debug_callback)) != CURLE_OK)
    {
        return data;
    }
#endif
    
    if ((res_ = set_url(url)) != CURLE_OK ||
        (res_ = set_base_option()) != CURLE_OK ||
        (res_ = set_no_certification()) != CURLE_OK ||
        (res_ = set_write_data((void*)&data, response_write_data_save_to_string)) != CURLE_OK ||
        (res_ = easy_perform()) != CURLE_OK)
    {
        data.clear();
        return data;
    }

    return data;
}

CURLcode http::set_debug(std::string& debug_info, int(*debug_cb)(CURL* handle, curl_infotype type, char* data, size_t size, void* userptr))
{
    //curl_version_info
    assert(curl_ != NULL);

    res_ = CURLE_OK;
    if ((res_ = curl_easy_setopt(curl_, CURLOPT_VERBOSE, 1L)) != CURLE_OK ||
        (res_ = curl_easy_setopt(curl_, CURLOPT_DEBUGFUNCTION, debug_cb)) != CURLE_OK ||
        (res_ = curl_easy_setopt(curl_, CURLOPT_DEBUGDATA, (void*)(&debug_info))) != CURLE_OK
        )
    {
        return res_;
    }
    return CURLE_OK;
}

int http::debug_callback(CURL* handle, curl_infotype type, char* data, size_t size, void* userptr)
{
    (void)handle; /* prevent compiler warning */
    std::string& text = *(std::string *)userptr;
    switch (type)
    {
    case CURLINFO_TEXT:
        text = "== info:";
        break;
    case CURLINFO_HEADER_OUT:
        text = "=> send header:";
        break;
    case CURLINFO_DATA_OUT:
        text = "=> send data:";
        break;
    case CURLINFO_SSL_DATA_OUT:
        text = "=> send ssl data:";
        break;
    case CURLINFO_HEADER_IN:
        text = "<= recv header:";
        break;
    case CURLINFO_DATA_IN:
        text = "<= recv data:";
        break;
    case CURLINFO_SSL_DATA_IN:
        text = "<= recv ssl data:";
        break;
    default: /* in case a new one is introduced to shock us */
        text = std::to_string(type);
        //return 0;
    }

    text.append(data, size);

    return 0;
}

CURLcode http::set_url(const std::string& url)
{
    assert(curl_ != NULL);

    std::string url_tmp = url;
    std::replace(url_tmp.begin(), url_tmp.end(), '\\', '/');
    res_ = curl_easy_setopt(curl_, CURLOPT_URL, url_tmp.c_str());
    if (res_ != CURLE_OK)
    {
        return res_;
    }

    return CURLE_OK;
}

CURLcode http::set_user_pwd(const std::string& user, const std::string& pwd)
{
    assert(curl_ != NULL);

    res_ = curl_easy_setopt(curl_, CURLOPT_USERPWD, (user + ":" + pwd).c_str());
    if (res_ != CURLE_OK)
    {
        return res_;
    }

    return CURLE_OK;
}


CURLcode http::set_post(const std::string& post_fields)
{
    assert(curl_ != NULL);

    //CURLOPT_HTTPPOST
    //CURLOPT_POSTQUOTE
    res_ = CURLE_OK;
    if ((res_ = curl_easy_setopt(curl_, CURLOPT_POST, 1L)) != CURLE_OK ||
        (!post_fields.empty() &&
        ((res_ = curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, post_fields.c_str())) != CURLE_OK ||
        (res_ = curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, post_fields.size())) != CURLE_OK)))
    {
        return res_;
    }
    return CURLE_OK;
}

CURLcode http::set_write_data(void *response, std::size_t(*writedata_cb)(char *buf, size_t element_size, size_t element_count, void *user_data))
{
    assert(curl_ != NULL);
    res_ = CURLE_OK;
    if ((res_ = curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, writedata_cb)) != CURLE_OK ||
        (res_ = curl_easy_setopt(curl_, CURLOPT_WRITEDATA, response)) != CURLE_OK)
    {
        return res_;
    }
    return CURLE_OK;
}

size_t http::response_write_data_save_to_string(char* buf, size_t element_size, size_t element_count, void* user_data)
{
    if (user_data == NULL || buf == NULL)
    {
        return -1;
    }
    std::string& response = *(std::string *)user_data;
    response.append(buf, element_size * element_count);

    return element_size * element_count;
}

size_t http::response_write_data_save_to_file(char* buf, size_t element_size, size_t element_count, void* user_data)
{
#if 0
    if (user_data == NULL || buf == NULL)
    {
        return -1;
    }
#endif
    
    FILE* stream = (FILE*)user_data;
    return fwrite(buf, element_size, element_count, stream);
}

CURLcode http::set_skip_all_signal()
{
    assert(curl_ != NULL);
    /**
    * 当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作。
    * 如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出。
    */
    return (res_ = curl_easy_setopt(curl_, CURLOPT_NOSIGNAL, 1L));
}

CURLcode http::set_timeout(int32_t connect_timeout_ms /*= 3000L*/, int32_t low_speed_limit_byte_per_second/* = 10L*/, int32_t low_speed_time_s /* = 2 * 60L*/, int32_t timeout_ms /*= 0L*/)
{
    assert(curl_ != NULL);

    //if (timeout_ms == 0)
    //{
    //    timeout_ms = 5000;
    //}

    //if (connect_timeout_ms == 0)
    //{
    //    connect_timeout_ms = 3000;
    //}

    res_ = CURLE_OK;
    if (/*(res_ = curl_easy_setopt(curl_, CURLOPT_ACCEPTTIMEOUT_MS, 5000L)) != CURLE_OK ||*/
        (connect_timeout_ms > 0 && (res_ = curl_easy_setopt(curl_, CURLOPT_CONNECTTIMEOUT_MS, connect_timeout_ms)) != CURLE_OK) ||
        (low_speed_time_s > 0 && (res_ = curl_easy_setopt(curl_, CURLOPT_LOW_SPEED_TIME, low_speed_time_s)) != CURLE_OK) ||
        (low_speed_limit_byte_per_second > 0 && (res_ = curl_easy_setopt(curl_, CURLOPT_LOW_SPEED_LIMIT, low_speed_limit_byte_per_second)) != CURLE_OK) ||
        (timeout_ms > 0 && (res_ = curl_easy_setopt(curl_, CURLOPT_TIMEOUT_MS, timeout_ms)) != CURLE_OK)
        )
    {
        return res_;
    }
    return CURLE_OK;
}

CURLcode http::set_redirect(int32_t max_redirs/* = 0L*/)
{
    assert(curl_ != NULL);

    /**
    * Web服务器一般会重定向链接，比如访问http:/xxx/x1.do自动转到http:/xxx/x2.do
    * 所以一定要设置CURLOPT_FOLLOWLOCATION为1,否则重定向后的数据不会返回。
    */
    res_ = CURLE_OK;
    if ((res_ = curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, 1L)) != CURLE_OK ||
        (max_redirs > 0 && (res_ = curl_easy_setopt(curl_, CURLOPT_MAXREDIRS, max_redirs)) != CURLE_OK) ||
        (res_ = curl_easy_setopt(curl_, CURLOPT_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS)) != CURLE_OK ||
        (res_ = curl_easy_setopt(curl_, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS)) != CURLE_OK)
    {
        return res_;
    }
    return CURLE_OK;
}

CURLcode http::set_no_certification()
{
    assert(curl_ != NULL);

    res_ = CURLE_OK;
    if ((res_ = curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, 0L)) != CURLE_OK ||
        (res_ = curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYHOST, 0L)) != CURLE_OK)
    {
        return res_;
    }

    return CURLE_OK;
}

CURLcode http::set_certification(const std::string& ca_fpath, const std::string& ssl_cert_type/* = "PEM"*/)
{
    assert(curl_ != NULL);

    res_ = CURLE_OK;
    if ((res_ = curl_easy_setopt(curl_, CURLOPT_SSLCERTTYPE, ssl_cert_type.c_str())) != CURLE_OK || //"PEM" "DER" "P12"
        (res_ = curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, 1L)) != CURLE_OK ||
        (res_ = curl_easy_setopt(curl_, CURLOPT_CAINFO, ca_fpath.c_str())) != CURLE_OK)
    {
        return res_;
    }

    return CURLE_OK;
}

CURLcode http::disable_progress()
{
    assert(curl_ != NULL);

    res_ = curl_easy_setopt(curl_, CURLOPT_NOPROGRESS, 1L);
    if (res_ != CURLE_OK)
    {
        return res_;
    }

    return CURLE_OK;
}

CURLcode http::set_progress(void* progress_context/* = NULL*/, int(*progress_cb)(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)/* = NULL*/)
{
    assert(curl_ != NULL);

    res_ = CURLE_OK;
    if (progress_context == NULL || progress_cb == NULL)
    {
        return (res_ = curl_easy_setopt(curl_, CURLOPT_NOPROGRESS, 1L));
    }

    assert(progress_context != NULL);
    assert(progress_cb != NULL);
    if ((res_ = curl_easy_setopt(curl_, CURLOPT_NOPROGRESS, 0L)) != CURLE_OK ||
        (res_ = curl_easy_setopt(curl_, CURLOPT_XFERINFOFUNCTION, progress_cb)) != CURLE_OK ||
        (res_ = curl_easy_setopt(curl_, CURLOPT_XFERINFODATA, progress_context)) != CURLE_OK
        )
    {
        return res_;
    }

    return CURLE_OK;
}

#define MINIMAL_PROGRESS_FUNCTIONALITY_INTERVAL     1000000
int http::progress_callback(void *progress_context, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
    CURL* curl = (CURL*)progress_context;
    curl_off_t last_runtime = 0;
    //(*myp.res) = CURLE_OK;
    CURLcode res = curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME_T, &last_runtime);
    if (res != CURLE_OK)
    {
        return 1;
    }

#if 0
    if (dltotal != 0 && dlnow != 0/* && myp.progress_cb3 != NULL*/)
    {
        if ((last_runtime/* - myp.last_runtime*/) >= MINIMAL_PROGRESS_FUNCTIONALITY_INTERVAL || dlnow >= dltotal)
        {
            //myp.last_runtime = curtime;
            //100 * dlnow / dltotal;
        }
    }
#endif

    if (dltotal != 0 && dlnow > dltotal)
    {
        return 1;
    }

    return 0;
}

