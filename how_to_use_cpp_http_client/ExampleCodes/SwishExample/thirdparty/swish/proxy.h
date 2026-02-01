#ifndef ______lib_SWISH___proxy_h
#define ______lib_SWISH___proxy_h
/**
 * @author Basit Ayantunde (rlamarrr@gmail.com)
 * @brief Templated CURL requests abstraction
 * @version 0.1
 * 
 * @copyright Copyright (c) 2018
 *      __                __         ____                                    __         
 *     /\ \        __    /\ \       /\  _`\                   __            /\ \        
 *     \ \ \      /\_\   \ \ \____  \ \,\L\_\    __  __  __  /\_\     ____  \ \ \___    
 *      \ \ \  __ \/\ \   \ \ '__`\  \/_\__ \   /\ \/\ \/\ \ \/\ \   /',__\  \ \  _ `\  
 *       \ \ \L\ \ \ \ \   \ \ \L\ \   /\ \L\ \ \ \ \_/ \_/ \ \ \ \ /\__, `\  \ \ \ \ \ 
 *        \ \____/  \ \_\   \ \_,__/   \ `\____\ \ \___x___/'  \ \_\\/\____/   \ \_\ \_\
 *         \/___/    \/_/    \/___/     \/_____/  \/__//__/     \/_/ \/___/     \/_/\/_/
 *                                                                                
 *                                                                                
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 */

#include <string>

#include <curl/curl.h>

#include "request.h"
#include "status_codes.h"

namespace swish {

struct ProxyCredentials {
  std::string username{};
  std::string password{};
};

enum class ProxyProtocol {
  kHTTP = CURLPROXY_HTTP,
  kHTTPS = CURLPROXY_HTTPS,
  kSOCKS5 = CURLPROXY_SOCKS5,
  kSOCKS5Hostname = CURLPROXY_SOCKS5_HOSTNAME,
  kSOCKS4 = CURLPROXY_SOCKS4,
  KSOCKS4a = CURLPROXY_SOCKS4A
};

struct Proxy {
  ProxyProtocol protocol = ProxyProtocol::kHTTP;
  std::string host{};
  ProxyCredentials credentials{};
  RequestHeader header{};

  friend struct Configuration;

 private:
  StatusCode ConfigHandle(CURL* curl_handle) {
    StatusCode status = StatusCode::OK;

    // set opt no matter what
    if (host.empty()) {
      status = static_cast<StatusCode>(
          curl_easy_setopt(curl_handle, CURLOPT_PROXY, nullptr));

    } else {
      status = static_cast<StatusCode>(
          curl_easy_setopt(curl_handle, CURLOPT_PROXY, host.c_str()));
    }
    if (!IsOK(status)) return status;

    status = static_cast<StatusCode>(
        curl_easy_setopt(curl_handle, CURLOPT_PROXYAUTH, protocol));
    if (!IsOK(status)) return status;

    // set the proxy credentials
    if (credentials.password.empty()) {
      status = static_cast<StatusCode>(
          curl_easy_setopt(curl_handle, CURLOPT_PROXYUSERNAME, nullptr));

    } else {
      status = static_cast<StatusCode>(curl_easy_setopt(
          curl_handle, CURLOPT_PROXYUSERNAME, credentials.password.c_str()));
    }
    if (!IsOK(status)) return status;

    if (credentials.username.empty()) {
      status = static_cast<StatusCode>(
          curl_easy_setopt(curl_handle, CURLOPT_PROXYUSERNAME, nullptr));

    } else {
      status = static_cast<StatusCode>(curl_easy_setopt(
          curl_handle, CURLOPT_PROXYUSERNAME, credentials.username.c_str()));
    }
    if (!IsOK(status)) return status;

    status = static_cast<StatusCode>(curl_easy_setopt(
        curl_handle, CURLOPT_PROXYHEADER, header.header_data_.get()));

    if (!IsOK(status)) return status;

    return StatusCode::OK;
  }
};
};  // namespace swish
#endif