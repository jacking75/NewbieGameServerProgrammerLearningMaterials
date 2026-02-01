#ifndef ______lib_SWISH___auth_h
#define ______lib_SWISH___auth_h
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

#include "status_codes.h"

namespace swish {

enum class Authentication : uint64_t {
  Basic = CURLAUTH_BASIC,
  Digest = CURLAUTH_DIGEST,
  DigestIE = CURLAUTH_DIGEST_IE,
  NTLM = CURLAUTH_NTLM,
  Any = CURLAUTH_ANY,
  Safe = CURLAUTH_ANYSAFE,
  Bearer = CURLAUTH_BEARER,
  Negotiate = CURLAUTH_NEGOTIATE,
  None = CURLAUTH_NONE,
  GSS = CURLAUTH_GSSAPI,
  GSSNegotiate = CURLAUTH_GSSNEGOTIATE
};

// data structure for authorization credentials
struct Credentials {
  Authentication authentication_type = Authentication::None;
  std::string username{};
  std::string password{};

  friend struct Configuration;

 private:
  StatusCode ConfigHandle(CURL* curl_handle) {
    StatusCode status = StatusCode::OK;

    status = static_cast<StatusCode>(
        curl_easy_setopt(curl_handle, CURLOPT_HTTPAUTH, authentication_type));
    if (!IsOK(status)) return status;

    if (username.empty()) {
      status = static_cast<StatusCode>(
          curl_easy_setopt(curl_handle, CURLOPT_USERNAME, nullptr));
    } else {
      status = static_cast<StatusCode>(
          curl_easy_setopt(curl_handle, CURLOPT_USERNAME, username.c_str()));
    };
    if (!IsOK(status)) return status;

    if (password.empty()) {
      status = static_cast<StatusCode>(
          curl_easy_setopt(curl_handle, CURLOPT_PASSWORD, nullptr));
    } else {
      status = static_cast<StatusCode>(
          curl_easy_setopt(curl_handle, CURLOPT_PASSWORD, password.c_str()));
    };
    if (!IsOK(status)) return status;

    return StatusCode::OK;
  };
};

};  // namespace swish

#endif