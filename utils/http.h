#pragma once
#include <string>
#include "curl_easy_environ.h"

class http
{
public:
    http();
    ~http();

    std::string easy_strerror() const;
    std::string get_redirect(const std::string& url);

    std::string http_get(const std::string& url);
    std::string http_post(const std::string& url, const std::string& post_fields = "");

    uint64_t get_size_download(const std::string& url);

    bool http_download(const std::string& url, const std::string& dest_path,
        void* progress_context = NULL, int(*progress_cb)(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) = NULL);
    std::string http_download(const std::string& url);

private:
    void set_strerror(long response_code);
    void easy_reset();
    CURLcode easy_perform();
    long get_response_code();

    CURLcode set_base_option();
    CURLcode set_error_buffer();

    CURLcode set_debug(std::string& debug_info, int(*debug_cb)(CURL* handle, curl_infotype type, char* data, size_t size, void* userptr));
    static int debug_callback(CURL* handle, curl_infotype type, char* data, size_t size, void* userptr);

    CURLcode set_url(const std::string& url);
    CURLcode set_user_pwd(const std::string& user, const std::string& pwd);
    CURLcode set_post(const std::string& post_fields);

    CURLcode disable_progress();
    CURLcode set_progress(void* progress_context = NULL, int(*progress_cb)(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) = NULL);
    static int progress_callback(void *progress_context, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);

    CURLcode set_write_data(void* response, std::size_t(*writedata_cb)(char* buf, size_t element_size, size_t element_count, void* user_data));
    static size_t response_write_data_save_to_string(char* buf, size_t element_size, size_t element_count, void* user_data);
    static size_t response_write_data_save_to_file(char* buf, size_t element_size, size_t element_count, void* user_data);

    CURLcode set_skip_all_signal();
    CURLcode set_timeout(int32_t connect_timeout_ms = 3000L, int32_t low_speed_limit_byte_per_second = 10L, int32_t low_speed_time_s = 2 * 60L, int32_t timeout_ms = 0L);
    CURLcode set_redirect(int32_t max_redirs = 0L);
    CURLcode set_no_certification();
    CURLcode set_certification(const std::string& ca_fpath, const std::string& ssl_cert_type = "PEM");

private:
    curl_easy_environ easy_env;
    CURL* curl_;
    CURLcode res_;
    char err_buf_[CURL_ERROR_SIZE];
};

