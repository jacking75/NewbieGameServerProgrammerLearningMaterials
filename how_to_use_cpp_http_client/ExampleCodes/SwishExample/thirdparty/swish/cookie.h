#ifndef ______lib_SWISH___cookie_h
#define ______lib_SWISH___cookie_h
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

#include <fstream>
#include <map>
#include <memory>
#include <string>

#include "status_codes.h"

// we must not store reference to unowned memory

namespace swish {

enum class CookieLocation {
  None = 0,
  Memory = 1,
  File = 2,
};

template <typename ByteT = char, typename ByteTraits = std::char_traits<ByteT>,
          typename Allocator = std::allocator<ByteT>>
class BasicCookie;
// provide allocator const copy
template <typename ByteT = char, typename ByteTraits = std::char_traits<ByteT>,
          typename Allocator = std::allocator<ByteT>>
class BasicMemoryCookie : public BasicCookie<ByteT, ByteTraits, Allocator> {
  using byte_type = ByteT;
  using allocator_type = std::allocator<byte_type>;
  using byte_traits = ByteTraits;
  using size_type = typename allocator_type::size_type;

 public:
  BasicMemoryCookie(std::basic_string_view<byte_type, byte_traits> cookie_data)
      : BasicCookie<byte_type, byte_traits, allocator_type>{
            cookie_data, CookieLocation::Memory} {}

  BasicMemoryCookie(const BasicMemoryCookie& to_copy) = default;
  BasicMemoryCookie& operator=(const BasicMemoryCookie& to_copy) = default;

  BasicMemoryCookie(BasicMemoryCookie&& to_copy) = default;
  BasicMemoryCookie& operator=(BasicMemoryCookie&& to_copy) = default;

  ~BasicMemoryCookie() = default;
};

template <typename ByteT = char, typename ByteTraits = std::char_traits<ByteT>,
          typename Allocator = std::allocator<ByteT>>
class BasicFileCookie : public BasicCookie<ByteT, ByteTraits, Allocator> {
  using byte_type = ByteT;
  using allocator_type = std::allocator<byte_type>;
  using byte_traits = ByteTraits;
  using size_type = typename allocator_type::size_type;

 public:
  BasicFileCookie(std::string_view cookie_data)
      : BasicCookie<byte_type, byte_traits, allocator_type>{
            cookie_data, CookieLocation::File} {}

  BasicFileCookie(const BasicFileCookie& to_copy) = default;
  BasicFileCookie& operator=(const BasicFileCookie& to_copy) = default;

  BasicFileCookie(BasicFileCookie&& to_copy) = default;
  BasicFileCookie& operator=(BasicFileCookie&& to_copy) = default;

  ~BasicFileCookie() = default;
};

// class for session_cookies, resolves whether to load from file or from memory
template <typename ByteT, typename ByteTraits, typename Allocator>
class BasicCookie {
 public:
  using byte_type = ByteT;
  using allocator_type = Allocator;
  using byte_traits = ByteTraits;
  using size_type = typename allocator_type::size_type;

 private:
  CookieLocation location_ = CookieLocation::None;
  std::basic_string<ByteT, byte_traits, allocator_type> cookie_data_;

  BasicCookie(std::basic_string_view<ByteT, byte_traits> rep,
              CookieLocation location)
      : location_{location} {
    cookie_data_ = rep;
  }

 public:
  BasicCookie(){};

  ~BasicCookie() = default;

  BasicCookie(const BasicCookie& to_copy) = default;

  BasicCookie& operator=(const BasicCookie& to_copy) = default;

  BasicCookie(BasicCookie&& to_move) = default;

  BasicCookie& operator=(BasicCookie&& to_move) = default;

  inline const bool& Empty() const { return cookie_data_.empty(); }

  inline const size_type size() const { return cookie_data_.size(); }

  StatusCode ConfigHandle(CURL* curl_handle) {
    StatusCode status = StatusCode::OK;

    if (location_ == CookieLocation::None) {
      //
      //
      status = static_cast<StatusCode>(
          curl_easy_setopt(curl_handle, CURLOPT_COOKIELIST, nullptr));
      if (!IsOK(status)) return status;

      status = static_cast<StatusCode>(
          curl_easy_setopt(curl_handle, CURLOPT_COOKIEFILE, nullptr));
      if (!IsOK(status)) return status;

    } else {
      //
      if (cookie_data_.empty()) {
        status = static_cast<StatusCode>(
            curl_easy_setopt(curl_handle, CURLOPT_COOKIEFILE, nullptr));
        if (!IsOK(status)) return status;

        status = static_cast<StatusCode>(
            curl_easy_setopt(curl_handle, CURLOPT_COOKIELIST, nullptr));
        if (!IsOK(status)) return status;

      } else {
        //
        if (location_ == CookieLocation::File)
          status = static_cast<StatusCode>(curl_easy_setopt(
              curl_handle, CURLOPT_COOKIEFILE, cookie_data_.c_str()));
        else
          status = static_cast<StatusCode>(curl_easy_setopt(
              curl_handle, CURLOPT_COOKIELIST, cookie_data_.c_str()));
      }
    }

    return status;
  }

  friend class BasicMemoryCookie<byte_type, byte_traits, allocator_type>;
  friend class BasicFileCookie<byte_type, byte_traits, allocator_type>;
};

typedef BasicCookie<char, std::char_traits<char>, std::allocator<char>> Cookie;

typedef BasicMemoryCookie<char, std::char_traits<char>, std::allocator<char>>
    MemoryCookie;

typedef BasicFileCookie<char, std::char_traits<char>, std::allocator<char>>
    FileCookie;

};  // namespace swish

#endif