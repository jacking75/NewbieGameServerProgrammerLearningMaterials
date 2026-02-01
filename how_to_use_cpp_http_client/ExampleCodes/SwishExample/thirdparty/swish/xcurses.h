#ifndef __lib_SWISH________________xcurses
#define __lib_SWISH________________xcurses
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

#include <cassert>
#include <cstring>

#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

namespace xcurses {

struct CursorMovement;
struct Cursor;
struct CursorDirection {
  constexpr explicit CursorDirection(char direction) : direction_(direction) {}

  friend CursorMovement;
  friend Cursor;

 private:
  char direction_;
};

enum class ClearDirection {
  End = 0,

  Start = 1,

  All = 2
};

struct Cursor {
  // \u001b[{n}B

  std::ostream* stream_ = nullptr;

  explicit Cursor(std::ostream* stream) : stream_(stream) {
    assert(stream != nullptr);
  }

  static constexpr const auto esc_ = "\u001b[";

  void Move(CursorDirection direction, size_t magnitude) {
    *stream_ << esc_ << magnitude << direction.direction_;

    stream_->flush();
  }

  void Move(size_t column) {
    // Set Column: \u001b[{n}G moves cursor to column n
    // Set Position: \u001b[{n};{m}H
    *stream_ << "\u001b[" << column << 'G';
  }
  void Move(size_t row, size_t column) {
    // Set Column: \u001b[{n}G moves cursor to column n
    // Set Position: \u001b[{n};{m}H
    *stream_ << "\u001b[" << row << ";" << column << 'H';
  }

  void ClearScreen(ClearDirection direction = ClearDirection::All) {
    // Clear Screen: \u001b[{n}J clears the screen
    // n=0 clears from cursor until end of screen,
    // n=1 clears from cursor to beginning of screen
    // n=2 clears entire screen

    *stream_ << "\u001b[" << static_cast<int>(direction) << 'J';
    stream_->flush();
  }

  void ClearLine(ClearDirection direction) {
    // Clear Line: \u001b[{n}K clears the current line
    // n=0 clears from cursor to end of line
    // n=1 clears from cursor to start of line
    // n=2 clears entire line

    *stream_ << "\u001b[" << static_cast<int>(direction) << 'K';
    stream_->flush();
  }

  void SavePosition() {
    *stream_ << "\u001b[s";
    stream_->flush();
  }

  void RestorePosition() {
    *stream_ << "\u001b[u";
    stream_->flush();
  }

  void ScrollUp(size_t n_lines) {
    *stream_ << "\u001b[" << n_lines << "S";
    stream_->flush();
  }
  void ScrollDown(size_t n_lines) {
    *stream_ << "\u001b[" << n_lines << "T";
    stream_->flush();
  }

  std::ostream& get_stream() { return *stream_; }
};

// cursor operations directions
namespace directions {
constexpr auto Up = CursorDirection('A');
constexpr auto Down = CursorDirection('B');
constexpr auto Right = CursorDirection('C');
constexpr auto Left = CursorDirection('D');
};  // namespace directions

Cursor cursor{&std::cout};
};  // namespace xcurses
#endif