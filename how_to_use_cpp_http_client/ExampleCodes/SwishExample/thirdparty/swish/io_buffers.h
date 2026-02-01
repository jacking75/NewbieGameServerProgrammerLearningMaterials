#ifndef ______lib_SWISH___io_buffers_h
#define ______lib_SWISH___io_buffers_h
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

#include <memory>
#include <string>
#include <vector>

namespace swish {
// internal use only

// read only buffer
template <typename ByteType = char,
          typename ByteTraits = std::char_traits<ByteType>,
          typename Allocator = std::allocator<ByteType>>
class BasicResponseBuffer {
 public:
  using byte_type = ByteType;
  using byte_traits = ByteTraits;
  using allocator_type = Allocator;
  using size_type = typename allocator_type::size_type;
  using pointer = typename allocator_type::pointer;

 private:
  allocator_type allocator_{};
  size_type size_ = 0;
  std::vector<std::pair<pointer, size_type>> chunks_;

 public:
  const std::vector<std::pair<pointer, size_type>>& chunks() const {
    return chunks_;
  }

  BasicResponseBuffer() = default;

  BasicResponseBuffer(const BasicResponseBuffer& to_copy) {
    for (const auto& chunk : to_copy.chunks_)
      this->PushCopy(chunk.first, chunk.second);
  }

  BasicResponseBuffer(BasicResponseBuffer&& to_move) {
    for (auto& chunk : to_move.chunks_) {
      chunks_.emplace_back(chunk.first, chunk.second);

      chunk.first = nullptr;
      chunk.second = 0;
    }

    size_ = to_move.size_;
    to_move.size_ = 0;
  }

  BasicResponseBuffer& operator=(const BasicResponseBuffer& to_copy) {
    for (const auto& chunk : to_copy.chunks_)
      this->PushCopy(chunk.first, chunk.second);
    return *this;
  }

  BasicResponseBuffer& operator=(BasicResponseBuffer&& to_move) {
    for (auto& chunk : to_move.chunks_) {
      chunks_.emplace_back(chunk.first, chunk.second);

      chunk.first = nullptr;
      chunk.second = 0;
    }

    size_ = to_move.size_;
    to_move.size_ = 0;

    return *this;
  }

  ~BasicResponseBuffer() noexcept {
    for (auto& chunk : chunks_) {
      allocator_.deallocate(chunk.first, chunk.second);
    }
  }

  size_type total_size() const { return size_; }

  std::basic_string<byte_type, byte_traits, allocator_type> ToString() const {
    std::basic_string<byte_type, byte_traits, allocator_type> result{};
    for (const auto& [buff, count] : chunks_) {
      result.append(buff, count);
    }
    return std::move(result);
  }

  void Save(std::basic_ofstream<byte_type, byte_traits>* file) {
    for (const auto& chunk : chunks_) {
      file->write(chunk.first, chunk.second);
    }
  }

  void PushCopy(pointer data, size_type total_bytes) {
    pointer data_handle = allocator_.allocate(total_bytes);
    std::memcpy(data_handle, data, total_bytes);
    chunks_.emplace_back(data_handle, total_bytes);
    size_ += total_bytes;
  }
};

template <typename ByteType = char,
          typename ByteTraits = std::char_traits<ByteType>,
          typename Allocator = std::allocator<ByteType>>
class BasicRequestBuffer {
 public:
  using byte_type = ByteType;
  using allocator_type = Allocator;
  using byte_traits = ByteTraits;
  using pointer = typename allocator_type::pointer;
  using size_type = typename allocator_type::size_type;
  using string_type = std::basic_string<byte_type, byte_traits, allocator_type>;

 private:
  size_type write_position_ = 0;
  string_type* xbuffer_{nullptr};

 public:
  BasicRequestBuffer(string_type* source) : xbuffer_{source} {}

  BasicRequestBuffer(const BasicRequestBuffer& to_copy) = default;

  BasicRequestBuffer(BasicRequestBuffer&&) = default;

  BasicRequestBuffer& operator=(const BasicRequestBuffer& to_copy) = default;

  BasicRequestBuffer& operator=(BasicRequestBuffer&& to_copy) = default;

  size_type size() const { return xbuffer_->size(); }

  size_type Write(byte_type* destination, size_type dest_size) {
    // when read position reaches end of sequence return zero to signify end
    // ensure size_t doesn't go negative
    // caution!!!

    size_type source_size = xbuffer_->size();
    size_type left_to_write = source_size - write_position_;
    size_type current_write_range =
        dest_size > left_to_write ? left_to_write : dest_size;
    if (current_write_range == 0) return 0;

    std::memcpy(destination, xbuffer_->begin().base(), current_write_range);
    write_position_ += current_write_range;
    return current_write_range;
  }
};

template <typename T>
using ResponseBodyBuffer = BasicResponseBuffer<T>;

template <typename T>
using RequestBodyBuffer = BasicRequestBuffer<T>;

typedef BasicResponseBuffer<char> ResponseHeaderBuffer;

};  // namespace swish
#endif