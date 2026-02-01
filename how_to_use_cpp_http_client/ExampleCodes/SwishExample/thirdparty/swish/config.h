#ifndef ______lib_SWISH___config_h
#define ______lib_SWISH___config_h
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


#include <chrono>
#include <string>

#include "auth.h"
#include "cookie.h"
#include "default_callbacks.h"
#include "http.h"
#include "proxy.h"
#include "request.h"
#include "status_codes.h"
#include "type_helpers.h"
#include "utils.h"

/* curl = curl_easy_init();
if(curl) {
  curl_easy_setopt(curl, CURLOPT_URL, url);
  // abort if slower than 30 bytes/sec during 60 seconds
  curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 60L);
  curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 30L);
  res = curl_easy_perform(curl);
  if(CURLE_OPERATION_TIMEDOUT == res) {
    printf("Timeout!\n");
  }
  // always cleanup
  curl_easy_cleanup(curl);
}
*/


namespace swish {

// data structure to represent request configurations for the client
struct Configuration {
  // performs all effects on curl

  StatusCode ConfigHandle(CURL* curl_handle) {
    default_monitor_ = {0, 0, 0, 0};
    StatusCode status = StatusCode::OK;

    status = static_cast<StatusCode>(
        curl_easy_setopt(curl_handle, CURLOPT_HTTP_VERSION, http_version));
    if (!IsOK(status)) return status;

    status = static_cast<StatusCode>(curl_easy_setopt(
        curl_handle, CURLOPT_NOPROGRESS, !use_progress_callback));
    if (!IsOK(status)) return status;

    status = static_cast<StatusCode>(curl_easy_setopt(
        curl_handle, CURLOPT_XFERINFOFUNCTION, progress_callback));
    if (!IsOK(status)) return status;

    status = static_cast<StatusCode>(
        curl_easy_setopt(curl_handle, CURLOPT_XFERINFODATA, &default_monitor_));
    if (!IsOK(status)) return status;

    status = static_cast<StatusCode>(curl_easy_setopt(
        curl_handle, CURLOPT_FOLLOWLOCATION, follow_redirection));
    if (!IsOK(status)) return status;

    status = static_cast<StatusCode>(
        curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, verbose));
    if (!IsOK(status)) return status;

    status = static_cast<StatusCode>(
        curl_easy_setopt(curl_handle, CURLOPT_MAXREDIRS, maximum_redirects));
    if (!IsOK(status)) return status;

    status = static_cast<StatusCode>(curl_easy_setopt(
        curl_handle, CURLOPT_UNRESTRICTED_AUTH, forward_authorization_headers));
    if (!IsOK(status)) return status;

    if (user_agent.empty()) {
      status = static_cast<StatusCode>(
          curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, nullptr));
    } else {
      status = static_cast<StatusCode>(
          curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, user_agent.c_str()));
    }
    if (!IsOK(status)) return status;

    status = static_cast<StatusCode>(
        curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT_MS, timeout.count()));
    if (!IsOK(status)) return status;

    if (cookie_file_storage.empty()) {
      status = static_cast<StatusCode>(
          curl_easy_setopt(curl_handle, CURLOPT_COOKIEJAR, nullptr));

    } else {
      status = static_cast<StatusCode>(curl_easy_setopt(
          curl_handle, CURLOPT_COOKIEJAR, cookie_file_storage.c_str()));
    };
    if (!IsOK(status)) return status;

    //
    //
    //
    // prep
    // pointer to nullptr if not set
    curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER,
                     header.header_data_.get());
    //
    //
    //

    status = static_cast<StatusCode>(
        authentication_credentials.ConfigHandle(curl_handle));
    if (!IsOK(status)) return status;

    status = static_cast<StatusCode>(proxy.ConfigHandle(curl_handle));
    if (!IsOK(status)) return status;

    status = session_cookie.ConfigHandle(curl_handle);
    if (!IsOK(status)) return status;

    return StatusCode::OK;
  };

  // version
  http::version http_version = http::Version_2_TLS;

  // transfer and receive callback
  ProgressCallback progress_callback = DefaultProgressCallback;

  bool use_progress_callback = false;

  // example.com is redirected, so we tell libcurl to follow redirection
  bool follow_redirection = false;

  bool verbose = false;

  // unlimited
  int64_t maximum_redirects = -1;

  // Starting in 7.58.0, libcurl will specifically prevent "Authorization:"
  // headers from being sent to other hosts than the first used one, unless
  // specifically permitted with the CURLOPT_UNRESTRICTED_AUTH to 1 option.
  bool forward_authorization_headers = false;

  // must be set
  std::string user_agent = "libswish@version0.0.0";

  // known by checking against status code
  std::chrono::milliseconds timeout{0};

  // file to export cookies to
  std::string cookie_file_storage{};

  // authentication

  Credentials authentication_credentials{};

  // proxy creadentials
  Proxy proxy{};

  // cookies to use for requests
  Cookie session_cookie{};

  // request headers
  RequestHeader header{};

  // TODO(lamarrr): add forward_post on redirect
  // example.com is redirected, so we tell libcurl to send POST on 301, 302
  // and 303 HTTP response codes
  // curl_easy_setopt(curl, CURLOPT_POSTREDIR, CURL_REDIR_POST_ALL);

  friend class Client;

 private:
  TransferSpeedMonitor<int64_t, double> default_monitor_{0, 0, 0, 0};
};
};  // namespace swish

#endif