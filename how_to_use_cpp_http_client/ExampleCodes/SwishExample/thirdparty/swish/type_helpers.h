#ifndef ______lib_SWISH_type___helpers_h
#define ______lib_SWISH_type___helpers_h
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

#include "utils.h"

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

// set default arguments
template <typename ResponseBodyBuffer_t>
using ResponseBodyBufferCallback_t = size_type<ResponseBodyBuffer_t> (*)(
    pointer<ResponseBodyBuffer_t> source,
    size_type<ResponseBodyBuffer_t> unit_byte_size,
    size_type<ResponseBodyBuffer_t> total_count,
    ResponseBodyBuffer_t* destination);

template <typename ResponseBodyBuffer_t>
using ResponseFileCallback_t = size_type<ResponseBodyBuffer_t> (*)(
    pointer<ResponseBodyBuffer_t> source,
    size_type<ResponseBodyBuffer_t> unit_byte_size,
    size_type<ResponseBodyBuffer_t> total_count,
    std::basic_ofstream<byte_type<ResponseBodyBuffer_t>,
                        byte_traits<ResponseBodyBuffer_t>>* destination);

template <typename RequestBodyBuffer_t>
using RequestBodyBufferCallback_t = size_type<RequestBodyBuffer_t> (*)(
    pointer<RequestBodyBuffer_t> destination,
    size_type<RequestBodyBuffer_t> unit_byte_size,
    size_type<RequestBodyBuffer_t> total_count, RequestBodyBuffer_t* source);

template <typename StringT>
using ResponseBodyStringCallback_t =
    size_t (*)(typename StringT::traits_type::char_type* contents,
               typename StringT::size_type byte_size,
               typename StringT::size_type total_count, StringT* target);

// logs to ostream
// callback with type restraint consider using functor on call back for time
using ProgressCallback = int (*)(TransferSpeedMonitor<int64_t, double>* log,
                                 size_t total_downloaded,
                                 size_t just_downloaded, size_t total_uploaded,
                                 size_t just_uploaded);

};  // namespace swish

#endif