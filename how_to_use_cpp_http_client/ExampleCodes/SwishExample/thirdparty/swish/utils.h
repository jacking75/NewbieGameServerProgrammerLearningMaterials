#ifndef ______lib_SWISH___utils_h
#define ______lib_SWISH___utils_h
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
#include <functional>
#include <type_traits>
#include <utility>

namespace swish {

inline namespace utils {
// future use
// unless specialized
template <typename T>
struct percentage_ref {
  static constexpr double ref = 100.0;
};

// allows implicit casting
template <>
struct percentage_ref<int> {
  static constexpr double ref = 100.0;
};
// allows implicit casting
template <>
struct percentage_ref<double> {
  static constexpr double ref = 100.0;
};

// not part of stl
// use double as default % reference type
struct percentage {
  // default ratioopt should be divided by 100.0
  // specified by callee
  // use logical reference
  template <typename DiffT, typename RatioOpT>
  auto operator()(const DiffT& diff, const RatioOpT& op) {
    return op(diff, percentage_ref<DiffT>::ref);
  };
};

// check this
template <bool condition, typename A, typename B>
struct ternary_select {
  using type = A;
};
template <typename A, typename B>
struct ternary_select<false, A, B> {
  using type = B;
};

template <typename T>
struct logical_copy_reference {
  static constexpr bool is_reference =
      std::greater_equal{}.operator()(sizeof(const T&), sizeof(T));
  using type = typename ternary_select<is_reference, const T&, T>::type;
};

// TODO: change name to Differential ???
// create seperate class for representing percentages
template <
    typename Y_t, typename X_t,
    typename DiffRatioOperatorT = std::divides<void>,
    typename YDiffOpT = std::minus<void>, typename XDiffOpT = std::minus<void>,
    typename YIncrOpT = std::plus<void>, typename XIncrOpT = std::plus<void>,
    typename YDiffSelfDivOpT = std::divides<void>,
    typename XDiffSelfDivOpT = std::divides<void>>
struct Rate {
  using y_type = Y_t;
  using x_type = X_t;

  // resolves copy by ref or value for optimization
  using y_copy_ref_t = typename logical_copy_reference<Y_t>::type;
  using x_copy_ref_t = typename logical_copy_reference<X_t>::type;

  // differential ratio operation type
  using diff_ratio_op_t = DiffRatioOperatorT;

  // difference operation type
  using x_diff_op_t = XDiffOpT;
  using y_diff_op_t = YDiffOpT;

  // difference type
  using x_diff_t = decltype(std::declval<x_diff_op_t>()(
      std::declval<x_type>(), std::declval<x_type>()));
  using y_diff_t = decltype(std::declval<y_diff_op_t>()(
      std::declval<y_type>(), std::declval<y_type>()));

  // FUTURE
  // difference to self division operator type
  using y_diff_self_div_op_t = YDiffSelfDivOpT;
  using x_diff_self_div_op_t = XDiffSelfDivOpT;

  // FUTURE
  // difference to self division type
  using y_diff_self_div_t = decltype(std::declval<y_diff_self_div_op_t>()(
      std::declval<y_diff_t>(), std::declval<y_type>()));
  using x_diff_self_div_t = decltype(std::declval<x_diff_self_div_op_t>()(
      std::declval<x_diff_t>(), std::declval<x_type>()));

  using y_incr_op_t = YIncrOpT;
  using x_incr_op_t = XIncrOpT;

  // FUTURE
  // diff to self ratio type
  using y_change_ratio_t = decltype(std::declval<y_diff_self_div_t>());
  using x_change_ratio_t = decltype(std::declval<x_diff_self_div_t>());

  // FUTURE
  // using y_diff_ratio_t =
  using y_percentage_diff_t = decltype(std::declval<y_diff_self_div_op_t>()(
      std::declval<y_diff_t>(), std::declval<double>()));

  using x_percentage_diff_t = decltype(std::declval<std::divides<void>>()(
      std::declval<x_diff_t>(), std::declval<double>()));

  /*
// change
using y_self_div_t = decltype(std::declval<y_diff_self_div_op_t>()(
std::declval<y_diff_t>(), std::declval<y_type>()));

using x_self_div_t = decltype(std::declval<x_diff_self_div_op_t>()(
std::declval<x_diff_t>(), std::declval<x_type>()));
*/

  using differential_t = decltype(std::declval<diff_ratio_op_t>()(
      std::declval<y_diff_t>(), std::declval<x_diff_t>()));

  // if 1,1 denotes increase from zero
  constexpr Rate(y_copy_ref_t current_y, x_copy_ref_t current_x)
      : c_x_value_(current_x), c_y_value_(current_y) {}

  constexpr Rate(y_copy_ref_t current_y, x_copy_ref_t current_x,
                 y_diff_op_t y_diff_op, x_diff_op_t x_diff_op)
      : c_y_value_(current_y),
        c_x_value_(current_x),
        y_diff_op_(y_diff_op),
        x_diff_op_(x_diff_op) {}

  constexpr Rate(y_copy_ref_t current_y, y_copy_ref_t previous_y,
                 x_copy_ref_t current_x, x_copy_ref_t previous_x)
      : c_y_value_(current_y),
        c_x_value_(current_x),

        p_y_value_(previous_y),
        p_x_value_(previous_x) {}

  constexpr Rate(y_copy_ref_t current_y, y_copy_ref_t previous_y,
                 x_copy_ref_t current_x, x_copy_ref_t previous_x,
                 y_diff_op_t y_diff_op, x_diff_op_t x_diff_op)
      : c_y_value_(current_y),
        c_x_value_(current_x),

        p_y_value_(previous_y),
        p_x_value_(previous_x),

        y_diff_op_(y_diff_op),
        x_diff_op_(x_diff_op) {}

  // constexpr dependent_type Head() const { return c_y_value_; }
  // constexpr independent_type Tail() const { return c_x_value_; }

  constexpr inline x_diff_t DX() const {
    return x_diff_op_(c_x_value_, p_x_value_);
  }
  constexpr inline y_diff_t DY() const {
    return y_diff_op_(c_y_value_, p_y_value_);
  }
  constexpr inline differential_t Differential() const {
    return ratio_op_(DY(), DX());
  }

  inline void Change(y_copy_ref_t new_y, x_copy_ref_t new_x) {
    ChangeX(new_y);
    ChangeY(new_y);
  }
  inline void ChangeX(x_copy_ref_t new_x) {
    p_x_value_ = c_x_value_;
    c_x_value_ = new_x;
  }
  inline void ChangeY(y_copy_ref_t new_y) {
    p_y_value_ = c_y_value_;
    c_y_value_ = new_y;
  }

  inline void IncrY(y_diff_t y_diff) {
    p_y_value_ = c_y_value_;
    c_y_value_ = y_incr_op_(c_y_value_, y_diff);
  }
  inline void IncrX(x_diff_t x_diff) {
    p_x_value_ = c_x_value_;
    c_x_value_ = x_incr_op_(c_x_value_, x_diff);
  }

  ~Rate() noexcept = default;
  constexpr Rate(const Rate&) = default;
  constexpr Rate(Rate&&) = default;
  constexpr Rate& operator=(const Rate&) = default;
  constexpr Rate& operator=(Rate&&) = default;

  inline constexpr bool operator==(const Rate& to_comp) const {
    // compare using ratios

    return Differential() == to_comp.Differential();
  }

  // decide const ref or value
  // in progress
  constexpr inline y_copy_ref_t current_y() const { return c_y_value_; }
  constexpr inline x_copy_ref_t current_x() const { return c_x_value_; }
  constexpr inline y_copy_ref_t previous_y() const { return p_y_value_; }
  constexpr inline x_copy_ref_t previous_x() const { return p_x_value_; }

  constexpr inline std::pair<y_copy_ref_t, x_copy_ref_t> current() const {
    return std::make_pair<y_copy_ref_t, x_copy_ref_t>(current_y(), current_x());
  }

  constexpr inline std::pair<y_copy_ref_t, x_copy_ref_t> previous() const {
    return std::make_pair<y_copy_ref_t, x_copy_ref_t>(previous_y(),
                                                      previous_x());
  }

 private:
  // current and previous x, y values
  y_type c_y_value_{};
  y_type p_y_value_{};

  x_type c_x_value_{};
  x_type p_x_value_{};

  // functors
  diff_ratio_op_t ratio_op_{};

  y_diff_op_t y_diff_op_{};
  x_diff_op_t x_diff_op_{};

  y_incr_op_t y_incr_op_{};
  x_incr_op_t x_incr_op_{};
};

template <typename T, typename TimeT>
struct TimeRateOp : public std::divides<void> {
  using first_type = T;
  using time_type = TimeT;
  using first_type_copy_ref = logical_copy_reference<first_type>;
  using time_type_copy_ref = logical_copy_reference<time_type>;
  using time_count_type = decltype(std::declval<time_type>().count());

  using result_type = decltype(

      std::declval<std::divides<void>>()(
          std::declval<first_type>(), std::declval<time_count_type>()

              ));

  constexpr result_type operator()(const first_type& t,
                                   const time_type& ttime) const {
    return std::divides<void>::operator()(t, ttime.count());
  }
};

template <typename ByteSize_t = int64_t, typename Duration_t = double,
          typename RatioOp_t = std::divides<void>,
          typename ByteSizeDiffOp_t = std::minus<void>,
          typename TimeDiffOp_t = std::minus<void>>
struct TransferSpeedMonitor {
  using byte_size_type = ByteSize_t;
  using duration_type = Duration_t;

  using ratio_op_type = RatioOp_t;

  using byte_size_diff_op_type = ByteSizeDiffOp_t;

  using duration_diff_op_type = TimeDiffOp_t;

  using rate_type = Rate<byte_size_type, duration_type, ratio_op_type,
                         byte_size_diff_op_type, duration_diff_op_type>;

  using byte_size_diff_type = typename rate_type::y_diff_t;

  /*
using instateneous_differential_type = decltype(

std::declval<ratio_op_type>()(
    std::declval<byte_size_type>(),
    std::declval<duration_diff_op_type>()(std::declval<duration_type>(),
                                          std::declval<duration_type>())

        ));
*/

  rate_type receive;
  rate_type send;

  std::chrono::time_point<std::chrono::high_resolution_clock> last_time_point;

  using byte_copy_ref_t = typename logical_copy_reference<byte_size_type>::type;

  using duration_copy_ref_t =
      typename logical_copy_reference<duration_type>::type;

  // initials
  constexpr TransferSpeedMonitor(byte_copy_ref_t tx_byte,
                                 duration_copy_ref_t tx_time,
                                 byte_copy_ref_t rx_byte,
                                 duration_copy_ref_t rx_time)
      : receive{rx_byte, rx_time}, send{tx_byte, tx_time} {}
};

};  // namespace utils
};  // namespace swish

#endif
