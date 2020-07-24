#include "xxx.h"
#include <mutex>
#include "curl_environ.h"

std::shared_ptr<curl_environ> g_curl_env = nullptr;
int g_update_env_init_count = 0;
std::mutex g_update_env_init_mtx;
#include "http.h"

XXX_API bool UpdateEnvInit()
{
    std::lock_guard<std::mutex> lk(g_update_env_init_mtx);
    if (g_update_env_init_count++ != 0)
    {
        return true;
    }

    g_curl_env = std::make_shared<curl_environ>();

    http hp;
    auto context = hp.http_post("https://mall.xunyou.com/index.php/partners/index", "gameid=1616");

    return true;
}

XXX_API void UpdateEnvUninit()
{
    std::lock_guard<std::mutex> lk(g_update_env_init_mtx);
    if (g_update_env_init_count == 0)
    {
        return;
    }

    if (--g_update_env_init_count != 0)
    {
        return;
    }

    g_curl_env.reset();
}