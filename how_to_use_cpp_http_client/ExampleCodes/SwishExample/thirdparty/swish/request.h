#ifndef ______lib_SWISH___request_h
#define ______lib_SWISH___request_h
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

#include <cstring>

#include <curl/curl.h>

#include <algorithm>
#include <forward_list>
#include <map>
#include <memory>
#include <string>

#include "client.h"

namespace swish {
template <typename CharT = char, typename CharTraits = std::char_traits<CharT>,
          typename CharAllocT = std::allocator<CharT>,
          typename AllocT = std::allocator<
              std::pair<std::basic_string<CharT, CharTraits, CharAllocT>,
                        std::basic_string<CharT, CharTraits, CharAllocT>>>>

using BasicFormData =
    std::map<std::basic_string<CharT, CharTraits, CharAllocT>,
             std::basic_string<CharT, CharTraits, CharAllocT>>;

typedef BasicFormData<char> FormData;

namespace {
/**
 * @brief constructs curl style header from std::map
 *
 * [header] map to construct slist from
 */

template <typename MapT>

inline curl_slist* ConstructRequestHeaderSList(const MapT& header) {
  curl_slist* handle = nullptr;

  for (const auto& field : header) {
    handle =
        curl_slist_append(handle, (field.first + ": " + field.second).c_str());
  }

  return handle;
}
};  // namespace

struct CurlSListDeleter {
  inline void operator()(curl_slist* to_free) { curl_slist_free_all(to_free); }
};

template <typename ByteType = char,
          typename ByteTraits = std::char_traits<ByteType>,
          typename Allocator = std::allocator<ByteType>>
class BasicRequestHeader {
 public:
  using byte_type = ByteType;
  using allocator_type = Allocator;
  using byte_traits = ByteTraits;
  using size_type = typename allocator_type::size_type;
  using pointer = typename allocator_type::pointer;
  using string_type = std::basic_string<byte_type, byte_traits, allocator_type>;

 private:
  std::unique_ptr<curl_slist, CurlSListDeleter> header_data_{nullptr};

  std::map<string_type, string_type, std::less<string_type>,
           std::allocator<std::pair<const string_type, string_type>>>
      header_view{};

 public:
  static constexpr const byte_type field_seperator[] = ": ";

  BasicRequestHeader() = default;

  // not allowed, type curl_slist* indeterminate
  BasicRequestHeader(const BasicRequestHeader& h) {
    header_view = h.header_view;
    header_data_.reset(ConstructRequestHeaderSList(header_view));
  };

  BasicRequestHeader(BasicRequestHeader&& header) = default;

  BasicRequestHeader& operator=(const BasicRequestHeader& h) {
    header_view = h.header_view;
    header_data_.reset(ConstructRequestHeaderSList(header_view));
    return *this;
  };

  BasicRequestHeader& operator=(BasicRequestHeader&&) = default;

  inline void Emplace(std::string_view key, std::string_view value) {
    auto new_handle = curl_slist_append(
        header_data_.get(),
        string_type{key.data()}.append(field_seperator).append(value).c_str());

    header_view.emplace(key, value);

    if (new_handle == nullptr)
      throw std::runtime_error{
          "Unable to append header data, CURL SLIST APPEND error occured"};

    header_data_.release();
    header_data_.reset(new_handle);
  }

  // range checked
  inline const string_type& operator[](const std::string& key) {
    return header_view.at(key);
  }

  // costly
  inline void Pop(const string_type& key) {
    Clear();
    header_view.erase(key);
    // deletes previous data, and recreates a new one

    header_data_.reset(ConstructRequestHeaderSList(header_view));
  }

  //  resets to nullptr
  inline void Clear() { header_data_.reset(); }

  inline bool Empty() const { return header_view.empty(); }

  // destructor slist free already specified
  ~BasicRequestHeader() = default;

  friend class Configuration;
  friend class Proxy;
};

namespace hackery {
struct FormDeleter {
  inline void operator()(curl_mime*& form) {
    curl_mime_free(form);

    form = nullptr;
  }
};
struct FieldDeleter {
  inline void operator()(curl_mimepart*& field) { field = nullptr; }
};

template <typename T>
class MultipartFormData {
 private:
  std::unique_ptr<curl_mime, FormDeleter> form_{nullptr};
  std::unique_ptr<curl_mimepart, FieldDeleter> field_{nullptr};
  size_t field_count_ = 0;

  StatusCode ConfigHandle(CURL* handle) {
    return static_cast<StatusCode>(
        curl_easy_setopt(handle, CURLOPT_MIMEPOST, form_.get()));
  }

  explicit MultipartFormData() {}

 public:
  explicit MultipartFormData(T* client) {
    form_.reset(curl_mime_init(client->curl_handle_));
  }

  MultipartFormData& AttachFile(std::string_view mime_form_key,
                                std::string_view local_file_path)

  {
    field_.release();
    field_.reset(curl_mime_addpart(form_.get()));

    curl_mime_name(field_.get(), mime_form_key.data());
    curl_mime_filedata(field_.get(), local_file_path.data());
    field_count_++;

    return *this;
  }

  MultipartFormData& PushField(std::string_view mime_form_key,
                               std::string_view mime_form_value) {
    //
    field_.release();

    field_.reset(curl_mime_addpart(form_.get()));
    curl_mime_name(field_.get(), mime_form_key.data());
    curl_mime_data(field_.get(), mime_form_value.data(), CURL_ZERO_TERMINATED);

    field_count_++;

    return *this;
  }

  inline size_t field_size() const { return field_count_; }

  MultipartFormData(const MultipartFormData&) = delete;

  MultipartFormData& operator=(const MultipartFormData&) = delete;

  MultipartFormData(MultipartFormData&&) = default;

  MultipartFormData& operator=(MultipartFormData&&) = default;

  ~MultipartFormData() = default;

  inline void Reset() {
    curl_mime_free(form_.get());
    form_.release();
    form_.reset(nullptr);
    field_.release();
    field_count_ = 0;
  }

  friend T;
};

};  // namespace hackery

typedef BasicRequestHeader<char> RequestHeader;
};  // namespace swish
#endif