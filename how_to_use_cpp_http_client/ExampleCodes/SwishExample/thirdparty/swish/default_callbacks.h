#ifndef ______lib_SWISH___default_callbacks_h
#define ______lib_SWISH___default_callbacks_h
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

#include <iostream>
#include <string>

#include "io_buffers.h"
#include "utils.h"
#include "xcurses.h"

namespace swish {

namespace {
template <typename T>
using size_type = typename T::size_type;

template <typename T>
using pointer = typename T::pointer;

template <typename T>
using byte_traits = typename T::byte_traits;

template <typename T>
using byte_type = typename T::byte_type;

};  // namespace

template <typename ResponseBodyBuffer_t = BasicResponseBuffer<char>>
size_type<ResponseBodyBuffer_t> ResponseBufferCallback(
    pointer<ResponseBodyBuffer_t> contents,
    size_type<ResponseBodyBuffer_t> byte_size,
    size_type<ResponseBodyBuffer_t> total_count,
    ResponseBodyBuffer_t* compactment) {
  size_type<ResponseBodyBuffer_t> total_size = total_count * byte_size;

  compactment->PushCopy(contents, total_size);
  return total_size;
}

template <typename RequestBodyBuffer_t = BasicRequestBuffer<char>>
size_type<RequestBodyBuffer_t> RequestBufferCallback(
    pointer<RequestBodyBuffer_t> destination,
    size_type<RequestBodyBuffer_t> byte_size,
    size_type<RequestBodyBuffer_t> total_count, RequestBodyBuffer_t* source) {
  size_type<RequestBodyBuffer_t> total_size = total_count * byte_size;

  return source->Write(destination, total_size);
}

// for downloads
// previously asserted open flags
// user to establish preconditions
// both source and destination path byte types match
// not to be overriden by user
template <typename RxFileT>
size_t ResponseFileCallback(typename RxFileT::char_type* contents,
                            size_t byte_size, size_t total_count,
                            RxFileT* file) {
  size_t total_size = total_count * byte_size;
  file->write(contents, total_size);
  return total_size;
}

template <typename StringT>
size_t ResponseBodyStringCallback(
    typename StringT::traits_type::char_type* contents,
    typename StringT::size_type byte_size,
    typename StringT::size_type total_count, StringT* target) {
  typename StringT::size_type total_size = total_count * byte_size;
  target->append(contents, total_size);
  return total_size;
}

template <typename Rep>
inline std::pair<double, const char*> BytesCountString(Rep byte_size) noexcept {
  if (byte_size <= 1000) return std::make_pair(byte_size, "bytes");

  if (byte_size <= 1000000) return std::make_pair(byte_size / 1000.0, "KB");

  if (byte_size <= 1000000000)
    return std::make_pair(byte_size / 1000000.0, "MB");

  return std::make_pair(byte_size / 1000000000.0, "GB");
}

/**
 * @brief LibSwish's default progress callback
 *
 * [log] monitor to keep track of tx and rx byte and time states
 * [to_download] total bytes to be downloaded as specified by server
 * [total_downloaded] total bytes downloaded at a specific time_point
 * [to_upload] total bytes to be uploaded as determined from file or data being
 * sent to the server [total_uploaded] total bytes uploaded at a specific
 * timepoint [int]
 */

// CAUTION!!! non zero return represents abortion
inline int DefaultProgressCallback(TransferSpeedMonitor<int64_t, double>* log,
                                   size_t to_download, size_t total_downloaded,
                                   size_t to_upload, size_t total_uploaded) {
  // [=======>                      ] 100%
  // [=====================>        ] 100%
  // [=============================>] 100%

  // ns
  auto time_point = std::chrono::high_resolution_clock::now();
  // seconds with double precision
  auto time_interval = std::chrono::duration_cast<std::chrono::nanoseconds>(
                           time_point - log->last_time_point)
                           .count() /
                       1000000000.0;

  // update
  // ns
  log->last_time_point = time_point;

  // ns
  // promote
  // double time_interval_D = time_interval.count();
  // std::cout << time_interval_D << std::endl;
  // ns to s
  // seconds

  // continuously changing
  // must be timepoint type

  log->receive.IncrX(time_interval);
  log->send.IncrX(time_interval);

  // non negative
  auto rx_prec = (total_downloaded - log->receive.current_y()) >= 1;
  auto tx_prec = (total_uploaded - log->send.current_y()) >= 1;

  if (tx_prec) {
    log->send.ChangeY(total_uploaded);
    auto tx_rate = log->send.Differential();
    auto [n_bytes, str_bytes] = BytesCountString(total_uploaded);
    auto [tn_bytes, tn_str_bytes] = BytesCountString(to_upload);
    auto [n_rate, str_rate] = BytesCountString(tx_rate);
    std::cout << n_bytes << " " << str_bytes << " of " << tn_bytes << " "
              << tn_str_bytes << " at " << n_rate << " " << str_rate << "/s";
    xcurses::cursor.Move(xcurses::directions::Left, 2000);
  }

  if (rx_prec) {
    log->receive.ChangeY(total_downloaded);
    auto rx_rate = log->receive.Differential();
    auto [n_bytes, str_bytes] = BytesCountString(total_downloaded);
    auto [tn_bytes, tn_str_bytes] = BytesCountString(to_download);
    auto [n_rate, str_rate] = BytesCountString(rx_rate);
    std::cout << n_bytes << " " << str_bytes << " of " << tn_bytes << " "
              << tn_str_bytes << " at " << n_rate << " " << str_rate << "/s";

    xcurses::cursor.Move(xcurses::directions::Left, 2000);
  }

  return 0;
}
};  // namespace swish
#endif