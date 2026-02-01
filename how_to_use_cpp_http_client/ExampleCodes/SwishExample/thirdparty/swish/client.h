#ifndef ______lib_SWISH___client_h
#define ______lib_SWISH___client_h
/**
 * @author Basit Ayantunde (rlamarrr@gmail.com)
 * @brief Templated CURL requests abstraction
 * @version 0.1
 *
 * @copyright Copyright (c) 2018
 *      __                __         ____                                    __
 *     /\ \        __    /\ \       /\  _`\                   __            /\ \
 *     \ \ \      /\_\   \ \ \____  \ \,\L\_\    __  __  __  /\_\     ____  \ \
 * \___
 *      \ \ \  __ \/\ \   \ \ '__`\  \/_\__ \   /\ \/\ \/\ \ \/\ \   /',__\  \ \
 * _ `\
 *       \ \ \L\ \ \ \ \   \ \ \L\ \   /\ \L\ \ \ \ \_/ \_/ \ \ \ \ /\__, `\  \
 * \ \ \ \
 *        \ \____/  \ \_\   \ \_,__/   \ `\____\ \ \___x___/'  \ \_\\/\____/   \
 * \_\ \_\
 *         \/___/    \/_/    \/___/     \/_____/  \/__//__/     \/_/ \/___/
 * \/_/\/_/
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

#include <cstring>

#include <algorithm>
#include <chrono>
#include <fstream>
#include <map>
#include <numeric>
#include <string>
#include <utility>

#include <curl/curl.h>

#include "config.h"
#include "default_callbacks.h"
#include "request.h"
#include "response.h"
#include "status_codes.h"

namespace swish {

class Client;

// nice circular dep. hack
typedef hackery::MultipartFormData<Client> MultipartFormData;

/**
 * @brief HTTP Client implementation
 *
 */
class Client {
  CURL* curl_handle_ = nullptr;

 public:
  Configuration configuration{};

  Client() : curl_handle_(curl_easy_init()) { assert(curl_handle_ != nullptr); }

  /**
   * @brief Sends a POST request of Content-Type:
   * application/x-www-form-urlencoded
   *
   * [FormData] FormData type to use for form data fields in the POST request
   * [RxByteType] Type of byte to be sent to the server, char, uint8, int8, etc
   * [RxByteTraits] The char_traits of the byte type
   * [RxAllocator] Allocator for server response headers and buffers
   *
   */
  template <typename FormDataT = FormData, typename RxByteType = char,
            typename RxByteTraits = std::char_traits<RxByteType>,
            typename RxAllocator = std::allocator<RxByteType>>

  std::pair<
      Response<BasicResponseBuffer<RxByteType, RxByteTraits, RxAllocator>>,
      StatusCode>

  Post(std::string_view url, const FormDataT& post_fields) {
    using response_t =
        Response<BasicResponseBuffer<RxByteType, RxByteTraits, RxAllocator>>;

    std::string post_data{};

    if (!(post_fields.size() > 0))
      throw std::range_error{"Post Fields can not be empty"};

    using post_t = FormDataT;

    std::vector<std::string> keys{};
    std::vector<std::string> values{};

    for (const auto& kv : post_fields) keys.push_back(kv.first);
    for (const auto& kv : post_fields) values.push_back(kv.second);

    for (auto i = 0; i < (keys.size() - 1); i++) {
      post_data.append(keys[i]);
      post_data.append("=");
      post_data.append(values[i]);
      post_data.append("&");
    }
    post_data.append(keys.back());
    post_data.append("=");
    post_data.append(values.back());

    StatusCode config_status = StatusCode::OK;

    config_status = static_cast<StatusCode>(curl_easy_setopt(
        curl_handle_, CURLOPT_POSTFIELDSIZE, post_data.size()));

    if (!IsOK(config_status))
      return std::make_pair(response_t{}, config_status);

    config_status = static_cast<StatusCode>(
        curl_easy_setopt(curl_handle_, CURLOPT_POSTFIELDS, post_data.c_str()));

    if (!IsOK(config_status))
      return std::make_pair(response_t{}, config_status);

    auto [resp, status] = Get<RxByteType, RxByteTraits, RxAllocator>(url);

    // expect no error
    curl_easy_setopt(curl_handle_, CURLOPT_POSTFIELDSIZE, -1);
    curl_easy_setopt(curl_handle_, CURLOPT_POSTFIELDS, nullptr);

    // default
    curl_easy_setopt(curl_handle_, CURLOPT_HTTPGET, true);

    return std::make_pair(std::move(resp), status);
  }

  template <typename FormDataT = FormData, typename RxByteType = char,
            typename RxByteTraits = std::char_traits<RxByteType>,
            typename RxAllocator = std::allocator<RxByteType>,
            typename TxByteType = char,
            typename TxByteTraits = std::char_traits<TxByteType>,
            typename TxAllocator = std::allocator<TxByteType>>

  std::pair<
      Response<BasicResponseBuffer<RxByteType, RxByteTraits, RxAllocator>>,
      StatusCode>
  Post(std::string_view url,
     std::basic_string<TxByteType, TxByteTraits, TxAllocator>* data) {
// change to const here and in requestbuffer
           using response_t =
        Response<BasicResponseBuffer<RxByteType, RxByteTraits, RxAllocator>>;

    auto request_callback = RequestBufferCallback<
        BasicRequestBuffer<TxByteType, TxByteTraits, TxAllocator>>;
    using string_t = std::basic_string<TxByteType, TxByteTraits, TxAllocator>;
    using req_buff_t =
        BasicRequestBuffer<TxByteType, TxByteTraits, TxAllocator>;

    req_buff_t bdata{data};

    StatusCode config_status = StatusCode::OK;

    config_status = static_cast<StatusCode>(
        curl_easy_setopt(curl_handle_, CURLOPT_POST, 1L));

    if (!IsOK(config_status))
      return std::make_pair(response_t{}, config_status);

    config_status = static_cast<StatusCode>(
        curl_easy_setopt(curl_handle_, CURLOPT_READFUNCTION, request_callback));

    if (!IsOK(config_status))
      return std::make_pair(response_t{}, config_status);

    config_status = static_cast<StatusCode>(
        curl_easy_setopt(curl_handle_, CURLOPT_READDATA, &bdata));

    if (!IsOK(config_status))
      return std::make_pair(response_t{}, config_status);

    auto [resp, status] = Get<RxByteType, RxByteTraits, RxAllocator>(url);

    curl_easy_setopt(curl_handle_, CURLOPT_READDATA, nullptr);
    curl_easy_setopt(curl_handle_, CURLOPT_READFUNCTION, nullptr);
    curl_easy_setopt(curl_handle_, CURLOPT_POST, 0L);

    return std::make_pair(std::move(resp), status);
  }

  /**
   * @brief Sends a POST request of Content-Type: multipart/form-data
   *
   * [MultipartFormData] Type to use for multipart form data fields, including
   * files
   *
   */
  template <typename MultipartFormDataT = MultipartFormData,
            typename RxByteType = char,
            typename RxByteTraits = std::char_traits<RxByteType>,
            typename RxAllocator = std::allocator<RxByteType>>

  std::pair<
      Response<BasicResponseBuffer<RxByteType, RxByteTraits, RxAllocator>>,
      StatusCode>

  Post(std::string_view url, MultipartFormDataT* multip_data) {
    multip_data->ConfigHandle(curl_handle_);
    auto [resp, status] = Get<RxByteType, RxByteTraits, RxAllocator>(url);

    curl_easy_setopt(curl_handle_, CURLOPT_HTTPGET, true);

    return std::make_pair(std::move(resp), status);

    // after request clean post tag
  }

  /**
   * @brief Performs a GET request and stores the response body in a vector of
   * buffers
   *
   */

  template <typename RxByteType = char,
            typename RxByteTraits = std::char_traits<RxByteType>,
            typename RxAllocator = std::allocator<RxByteType>>
  std::pair<
      Response<BasicResponseBuffer<RxByteType, RxByteTraits, RxAllocator>>,
      StatusCode>
  Get(std::string_view url) {
    // note get by default, must not be changed nor set to get, other requests
    // use this as a template and if request method is changed, it is explicitly
    // set back to HTTPGET
    // gets in chunks

    using rx_byte_type = RxByteType;

    using rx_byte_traits = RxByteTraits;

    using rx_allocator_t = RxAllocator;
    using response_buff_t =
        BasicResponseBuffer<rx_byte_type, rx_byte_traits, rx_allocator_t>;

    using response_t = Response<response_buff_t>;

    auto config_status = configuration.ConfigHandle(curl_handle_);

    if (config_status != StatusCode::OK) {
      return std::make_pair(response_t{}, config_status);
    }

    config_status = static_cast<StatusCode>(
        curl_easy_setopt(curl_handle_, CURLOPT_URL, url.data()));
    if (!IsOK(config_status))
      return std::make_pair(response_t{}, config_status);

    response_t response{};

    response_buff_t resp_buff{};

    ResponseHeaderBuffer header{};

    config_status = static_cast<StatusCode>(
        curl_easy_setopt(curl_handle_, CURLOPT_WRITEFUNCTION,
                         ResponseBufferCallback<response_buff_t>));
    if (!IsOK(config_status)) return std::make_pair(response, config_status);

    config_status = static_cast<StatusCode>(
        curl_easy_setopt(curl_handle_, CURLOPT_WRITEDATA, &resp_buff));
    if (!IsOK(config_status)) return std::make_pair(response, config_status);

    config_status = static_cast<StatusCode>(
        curl_easy_setopt(curl_handle_, CURLOPT_HEADERFUNCTION,
                         ResponseBufferCallback<ResponseHeaderBuffer>));
    if (!IsOK(config_status)) return std::make_pair(response, config_status);

    config_status = static_cast<StatusCode>(
        curl_easy_setopt(curl_handle_, CURLOPT_HEADERDATA, &header));
    if (!IsOK(config_status)) return std::make_pair(response, config_status);

    StatusCode status = StatusCode::OK;
    status = static_cast<StatusCode>(curl_easy_perform(curl_handle_));
    // if (!IsOK(status)) return std::make_pair(response, status);

    response.Prepare(curl_handle_, std::move(resp_buff), std::move(header));

    return std::make_pair(std::move(response), status);
  }

  /**
   * @brief Performs a GET request and stores the response in [target]
   *
   * [target] Target buffer to store server response body
   */
  template <typename RxByteType = char,
            typename RxByteTraits = std::char_traits<RxByteType>,
            typename RxAllocator = std::allocator<RxByteType>>
  inline std::pair<
      Response<BasicResponseBuffer<RxByteType, RxByteTraits, RxAllocator>>,
      StatusCode>
  // Gets in memory
  Get(std::string_view url,
      std::basic_string<RxByteType, RxByteTraits, RxAllocator>* target) {
    // alias for download in memory

    return Download(url, target);
  }

  template <typename RxByteType = char,
            typename RxByteTraits = std::char_traits<RxByteType>,
            typename RxAllocator = std::allocator<RxByteType>>

  std::pair<
      Response<BasicResponseBuffer<RxByteType, RxByteTraits, RxAllocator>>,
      StatusCode>
  Head(std::string_view url) {
    // change to curlopt_nobody then get
    using rx_byte_type = RxByteType;

    using rx_byte_traits = RxByteTraits;

    using rx_allocator_t = RxAllocator;
    using response_buff_t =
        BasicResponseBuffer<rx_byte_type, rx_byte_traits, rx_allocator_t>;

    using response_t = Response<response_buff_t>;

    //
    //
    //
    //
    // implicitly casted to 1L
    auto config_status = static_cast<StatusCode>(
        curl_easy_setopt(curl_handle_, CURLOPT_NOBODY, true));
    if (!IsOK(config_status))
      return std::make_pair(response_t{}, config_status);

    //
    //
    //
    //

    auto [resp, status] =
        Get<rx_byte_type, rx_byte_traits, rx_allocator_t>(url);

    //
    //
    // undo prev mod
    // implicitly casted to 1L
    config_status = static_cast<StatusCode>(
        curl_easy_setopt(curl_handle_, CURLOPT_NOBODY, false));
    if (!IsOK(config_status))
      return std::make_pair(std::move(resp), config_status);
    //
    //
    //
    //
    //
    //
    //

    return std::make_pair(std::move(resp), status);
  }

  /**
   * @brief Performs a GET request and downloads server response into a
   * basic_ofstream parameter of the specified RxByteType
   *
   */
  template <typename RxByteType = char,
            typename RxByteTraits = std::char_traits<RxByteType>,
            typename RxAllocator = std::allocator<RxByteType>>
  // Download into output file
  std::pair<
      Response<BasicResponseBuffer<RxByteType, RxByteTraits, RxAllocator>>,
      StatusCode>
  Download(std::string_view url,
           std::basic_ofstream<RxByteType, RxByteTraits>* file) {
    using rx_byte_type = RxByteType;

    using rx_byte_traits = RxByteTraits;

    using rx_file_t = std::basic_ofstream<RxByteType, RxByteTraits>;

    using rx_allocator_t = RxAllocator;
    using response_buff_t =
        BasicResponseBuffer<rx_byte_type, rx_byte_traits, rx_allocator_t>;

    using response_t = Response<response_buff_t>;

    auto config_status = configuration.ConfigHandle(curl_handle_);
    if (config_status != StatusCode::OK) {
      return std::make_pair(response_t{}, config_status);
    }

    config_status = static_cast<StatusCode>(
        curl_easy_setopt(curl_handle_, CURLOPT_URL, url.data()));
    if (!IsOK(config_status))
      return std::make_pair(response_t{}, config_status);

    response_t response{};

    response_buff_t resp_buff{};

    ResponseHeaderBuffer header{};

    config_status = static_cast<StatusCode>(curl_easy_setopt(
        curl_handle_, CURLOPT_WRITEFUNCTION, ResponseFileCallback<rx_file_t>));
    if (!IsOK(config_status)) return std::make_pair(response, config_status);

    config_status = static_cast<StatusCode>(
        curl_easy_setopt(curl_handle_, CURLOPT_WRITEDATA, file));
    if (!IsOK(config_status)) return std::make_pair(response, config_status);

    config_status = static_cast<StatusCode>(
        curl_easy_setopt(curl_handle_, CURLOPT_HEADERFUNCTION,
                         ResponseBufferCallback<ResponseHeaderBuffer>));
    if (!IsOK(config_status)) return std::make_pair(response, config_status);

    config_status = static_cast<StatusCode>(
        curl_easy_setopt(curl_handle_, CURLOPT_HEADERDATA, &header));
    if (!IsOK(config_status)) return std::make_pair(response, config_status);

    StatusCode status = StatusCode::OK;
    status = static_cast<StatusCode>(curl_easy_perform(curl_handle_));
    // if (!IsOK(status)) return std::make_pair(response, status);

    response.Prepare(curl_handle_, std::move(resp_buff), std::move(header));

    return std::make_pair(std::move(response), status);
  }

  /**
   * @brief Performs a GET Request and stores response in memory (Specified
   * string buffer)
   *
   *
   */

  template <typename RxByteType = char,
            typename RxByteTraits = std::char_traits<RxByteType>,
            typename RxAllocator = std::allocator<RxByteType>>

  std::pair<
      Response<BasicResponseBuffer<RxByteType, RxByteTraits, RxAllocator>>,
      StatusCode>
  Download(std::string_view url,
           std::basic_string<RxByteType, RxByteTraits, RxAllocator>* target) {
    using rx_byte_type = RxByteType;

    using rx_byte_traits = RxByteTraits;

    using rx_str_t = std::basic_string<RxByteType, RxByteTraits, RxAllocator>;

    using rx_allocator_t = RxAllocator;
    using response_buff_t =
        BasicResponseBuffer<rx_byte_type, rx_byte_traits, rx_allocator_t>;

    using response_t = Response<response_buff_t>;

    auto config_status = configuration.ConfigHandle(curl_handle_);
    if (config_status != StatusCode::OK) {
      return std::make_pair(response_t{}, config_status);
    }

    config_status = static_cast<StatusCode>(
        curl_easy_setopt(curl_handle_, CURLOPT_URL, url.data()));
    if (!IsOK(config_status))
      return std::make_pair(response_t{}, config_status);

    response_t response{};

    response_buff_t resp_buff{};

    ResponseHeaderBuffer header{};

    config_status = static_cast<StatusCode>(
        curl_easy_setopt(curl_handle_, CURLOPT_WRITEFUNCTION,
                         ResponseBodyStringCallback<rx_str_t>));
    if (!IsOK(config_status)) return std::make_pair(response, config_status);

    config_status = static_cast<StatusCode>(
        curl_easy_setopt(curl_handle_, CURLOPT_WRITEDATA, target));
    if (!IsOK(config_status)) return std::make_pair(response, config_status);

    config_status = static_cast<StatusCode>(
        curl_easy_setopt(curl_handle_, CURLOPT_HEADERFUNCTION,
                         ResponseBufferCallback<ResponseHeaderBuffer>));
    if (!IsOK(config_status)) return std::make_pair(response, config_status);

    config_status = static_cast<StatusCode>(
        curl_easy_setopt(curl_handle_, CURLOPT_HEADERDATA, &header));
    if (!IsOK(config_status)) return std::make_pair(response, config_status);

    StatusCode status = StatusCode::OK;
    status = static_cast<StatusCode>(curl_easy_perform(curl_handle_));
    // if (!IsOK(status)) return std::make_pair(response, status);

    response.Prepare(curl_handle_, std::move(resp_buff), std::move(header));

    return std::make_pair(std::move(response), status);
  }

  /**
   * @brief Performs a DELETE request
   *
   */
  template <typename RxByteType = char,
            typename RxByteTraits = std::char_traits<RxByteType>,
            typename RxAllocator = std::allocator<RxByteType>>
  // simple and plain DELETE request
  std::pair<
      Response<BasicResponseBuffer<RxByteType, RxByteTraits, RxAllocator>>,
      StatusCode>
  Delete(std::string_view url) {
    curl_easy_setopt(curl_handle_, CURLOPT_CUSTOMREQUEST, "DELETE");

    auto [resp, status] = Get(url);

    curl_easy_setopt(curl_handle_, CURLOPT_CUSTOMREQUEST, nullptr);

    return std::make_pair(std::move(resp), status);
  }

  /**
   * @brief check Server availability, this does not perform any request as it
   * only connects
   *
   */
  auto Ping(std::string_view url) -> decltype(Get("url")) {
    curl_easy_setopt(curl_handle_, CURLOPT_CONNECT_ONLY, true);

    auto [resp, status] = Get(url);

    curl_easy_setopt(curl_handle_, CURLOPT_CONNECT_ONLY, false);
    return std::make_pair(std::move(resp), status);
  }

  /**
   * @brief Performs a TRACE request
   *
   */
  template <typename RxByteType = char,
            typename RxByteTraits = std::char_traits<RxByteType>,
            typename RxAllocator = std::allocator<RxByteType>>
  std::pair<
      Response<BasicResponseBuffer<RxByteType, RxByteTraits, RxAllocator>>,
      StatusCode>

  Trace(std::string_view url) {
    curl_easy_setopt(curl_handle_, CURLOPT_CUSTOMREQUEST, "TRACE");
    curl_easy_setopt(curl_handle_, CURLOPT_NOBODY, true);

    auto [resp, status] = Get<RxByteType, RxByteTraits, RxAllocator>(url);

    curl_easy_setopt(curl_handle_, CURLOPT_NOBODY, false);
    curl_easy_setopt(curl_handle_, CURLOPT_CUSTOMREQUEST, nullptr);

    return std::make_pair(std::move(resp), status);
  }

  /**
   * @brief Queries server for available method OPTIONS
   *
   *
   */

  template <typename RxByteType = char,
            typename RxByteTraits = std::char_traits<RxByteType>,
            typename RxAllocator = std::allocator<RxByteType>>
  std::pair<
      Response<BasicResponseBuffer<RxByteType, RxByteTraits, RxAllocator>>,
      StatusCode>
  Options(std::string_view url) {
    curl_easy_setopt(curl_handle_, CURLOPT_CUSTOMREQUEST, "OPTIONS");

    auto [resp, status] = Get<RxByteType, RxByteTraits, RxAllocator>(url);

    curl_easy_setopt(curl_handle_, CURLOPT_CUSTOMREQUEST, nullptr);

    return std::make_pair(std::move(resp), status);
  }

  CURL* curl_handle() { return curl_handle_; }

  friend MultipartFormData;

  Client(const Client&) = delete;

  Client& operator=(const Client&) = delete;

  Client(Client&& to_move) {
    curl_handle_ = to_move.curl_handle_;
    to_move.curl_handle_ = nullptr;
    configuration = std::move(to_move.configuration);
  }

  Client& operator=(Client&& to_move) {
    curl_handle_ = to_move.curl_handle_;
    to_move.curl_handle_ = nullptr;
    configuration = std::move(to_move.configuration);
    return *this;
  }

  ~Client() noexcept { curl_easy_cleanup(curl_handle_); };
};
};  // namespace swish
#endif