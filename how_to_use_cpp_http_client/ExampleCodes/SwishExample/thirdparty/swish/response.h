#ifndef ______lib_SWISH___response_h
#define ______lib_SWISH___response_h
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
#include <curl/curl.h>

#include <chrono>
#include <map>
#include <string>

#include "http.h"
#include "io_buffers.h"
#include "status_codes.h"
namespace swish {

// memory allocated by curl is freed by curl_free

// fields to be filled must be known at compile time
template <typename ResponseBodyBuffer_t>
struct Response {
  //  CURLOPT_FOLLOWLOCATION
  //  CURLOPT_MAXREDIRS
  // char* //  curl_easy_getinfo(curl, CURLINFO_REDIRECT_URL, &url);
 private:
  int64_t total_duration_{};
  int64_t connection_delay_{};
  int64_t redirect_duration_{};
  int64_t response_code_{0};
  int64_t http_version_{static_cast<int64_t>(http::Version_None)};

  // ASSUMED freed by curl_easy_free
  char* redirect_url_ = nullptr;

  char* content_type_ = nullptr;

 public:
  using response_body_buffer_type = ResponseBodyBuffer_t;

  friend class Client;

  // CURLINFO_TOTAL_TIME_T

  std::chrono::microseconds total_duration() {
    return std::chrono::milliseconds(total_duration_);
  };
  // res = curl_easy_getinfo(curl_handle, CURLINFO_CONNECT_TIME_T, &val);
  // if((CURLE_OK == res) && (val>0))
  std::chrono::microseconds connection_delay() {
    return std::chrono::milliseconds(connection_delay_);
  };

  //  res = curl_easy_getinfo(curl, CURLINFO_REDIRECT_TIME_T, &redirect);
  // double seconds -> redirect
  std::chrono::microseconds redirect_duration() {
    return std::chrono::microseconds(redirect_duration_);
  };

  std::string_view redirect_url() { return redirect_url_; };

  // curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
  http::ResponseCode response_code() {
    return static_cast<http::ResponseCode>(response_code_);
  }

  // curl_easy_getinfo(curl, CURLINFO_HTTP_VERSION, &http_version);
  http::version http_version() { return http_version_; };

  // res = curl_easy_getinfo(curl, CURLINFO_SIZE_UPLOAD_T, &ul);
  size_t bytes_uploaded = 0;

  // res = curl_easy_getinfo(curl, CURLINFO_SPEED_DOWNLOAD_T, &speed);
  // bytes per second
  size_t average_download_speed = 0;

  // res = curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &ct);  
  std::string_view content_type() { return content_type_; };

  // curl_easy_getinfo(curl, CURLINFO_HEADER_SIZE, &size);
  size_t header_size = 0;

  // res = curl_easy_getinfo(curl, CURLINFO_HTTP_CONNECTCODE, &code);
  size_t connect_code = 0;

  

  // int64_t proxy_response_code{};

  // curl_easy_getinfo(curl, CURLINFO_REDIRECT_COUNT, &redirects);
  size_t redirect_count = 0;

  ResponseHeaderBuffer header{};

  // previously constructed and moved to this
  response_body_buffer_type body;

 private:
  /**
 * @brief  void Prepare(CURL* curl_handle , _,
  //            response_body_buffer_type&& body_data_)
*/
  void Prepare(CURL* curl_handle, response_body_buffer_type&& body_data,
               ResponseHeaderBuffer&& header_data) {
    //
    curl_easy_getinfo(curl_handle, CURLINFO_TOTAL_TIME_T, &total_duration_);

    curl_easy_getinfo(curl_handle, CURLINFO_CONNECT_TIME_T, &connection_delay_);

    curl_easy_getinfo(curl_handle, CURLINFO_REDIRECT_TIME_T,
                      &redirect_duration_);

    curl_easy_getinfo(curl_handle, CURLINFO_REDIRECT_URL, &redirect_url_);

    curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &response_code_);

    curl_easy_getinfo(curl_handle, CURLINFO_HTTP_VERSION, &http_version_);

    curl_easy_getinfo(curl_handle, CURLINFO_SIZE_UPLOAD_T, &bytes_uploaded);

    curl_easy_getinfo(curl_handle, CURLINFO_SPEED_DOWNLOAD_T,
                      &average_download_speed);

    curl_easy_getinfo(curl_handle, CURLINFO_CONTENT_TYPE, &content_type_);

    curl_easy_getinfo(curl_handle, CURLINFO_HEADER_SIZE, &header_size);


  curl_easy_getinfo(curl_handle, CURLINFO_HTTP_CONNECTCODE, &connect_code);

    curl_easy_getinfo(curl_handle, CURLINFO_REDIRECT_COUNT, &redirect_count);

    /**
     * @brief
     *
     */
    header = std::move(header_data);
    body = std::move(body_data);
  }
};


};  // namespace swish
#endif
