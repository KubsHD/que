///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2015 Microsoft Corporation. All rights reserved.
//
// This code is licensed under the MIT License (MIT).
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef GSL_UTIL_H
#define GSL_UTIL_H

#include "assert.hpp" // for Expects

#include <array>
#include <cstddef>          // for ptrdiff_t, size_t
#include <initializer_list> // for initializer_list
#include <type_traits>      // for is_signed, integral_constant
#include <utility>          // for exchange, forward

#if defined(__has_include) && __has_include(<version>)
#include <version>
#if defined(__cpp_lib_span) && __cpp_lib_span >= 202002L
#include <span>
#endif // __cpp_lib_span >= 202002L
#endif //__has_include(<version>)

#if defined(_MSC_VER) && !defined(__clang__)

#pragma warning(push)
#pragma warning(disable : 4127) // conditional expression is constant

#endif // _MSC_VER

#if defined(__cplusplus) && (__cplusplus >= 201703L)
#define GSL_NODISCARD [[nodiscard]]
#else
#define GSL_NODISCARD
#endif // defined(__cplusplus) && (__cplusplus >= 201703L)

namespace gsl
{
//
// GSL.util: utilities
//

// index type for all container indexes/subscripts/sizes
using index = std::ptrdiff_t;

// final_action allows you to ensure something gets run at the end of a scope
template <class F>
class final_action
{
public:
    static_assert(!std::is_reference<F>::value && !std::is_const<F>::value &&
                      !std::is_volatile<F>::value,
                  "Final_action should store its callable by value");

    explicit final_action(F f) noexcept : f_(std::move(f)) {}

    final_action(final_action&& other) noexcept
        : f_(std::move(other.f_)), invoke_(std::exchange(other.invoke_, false))
    {}

    final_action(const final_action&) = delete;
    final_action& operator=(const final_action&) = delete;
    final_action& operator=(final_action&&) = delete;

    // clang-format off
    GSL_SUPPRESS(f.6) // NO-FORMAT: attribute // terminate if throws
    // clang-format on
    ~final_action() noexcept
    {
        if (invoke_) f_();
    }

private:
    F f_;
    bool invoke_{true};
};

// finally() - convenience function to generate a final_action
template <class F>
GSL_NODISCARD final_action<typename std::remove_cv<typename std::remove_reference<F>::type>::type>
finally(F&& f) noexcept
{
    return final_action<typename std::remove_cv<typename std::remove_reference<F>::type>::type>(
        std::forward<F>(f));
}

// narrow_cast(): a searchable way to do narrowing casts of values
template <class T, class U>
// clang-format off
GSL_SUPPRESS(type.1) // NO-FORMAT: attribute
    // clang-format on
    constexpr T narrow_cast(U&& u) noexcept
{
    return static_cast<T>(std::forward<U>(u));
}

//
// at() - Bounds-checked way of accessing builtin arrays, std::array, std::vector
//
template <class T, std::size_t N>
// clang-format off
GSL_SUPPRESS(bounds.4) // NO-FORMAT: attribute
GSL_SUPPRESS(bounds.2) // NO-FORMAT: attribute
    // clang-format on
    constexpr T& at(T (&arr)[N], const index i)
{
    Expects(i >= 0 && i < narrow_cast<index>(N));
    return arr[narrow_cast<std::size_t>(i)];
}

template <class Cont>
// clang-format off
GSL_SUPPRESS(bounds.4) // NO-FORMAT: attribute
GSL_SUPPRESS(bounds.2) // NO-FORMAT: attribute
    // clang-format on
    constexpr auto at(Cont& cont, const index i) -> decltype(cont[cont.size()])
{
    Expects(i >= 0 && i < narrow_cast<index>(cont.size()));
    using size_type = decltype(cont.size());
    return cont[narrow_cast<size_type>(i)];
}

template <class T>
// clang-format off
GSL_SUPPRESS(bounds.1) // NO-FORMAT: attribute
    // clang-format on
    constexpr T at(const std::initializer_list<T> cont, const index i)
{
    Expects(i >= 0 && i < narrow_cast<index>(cont.size()));
    return *(cont.begin() + i);
}

#if defined(__cpp_lib_span) && __cpp_lib_span >= 202002L
template <class T, size_t extent = std::dynamic_extent>
constexpr auto at(std::span<T, extent> sp, const index i)
{
    Expects(i >= 0 && i < narrow_cast<i>(sp.size()));
    return sp[i];
}
#endif // __cpp_lib_span >= 202002L
} // namespace gsl

#if defined(_MSC_VER) && !defined(__clang__)

#pragma warning(pop)

#endif // _MSC_VER

#endif // GSL_UTIL_H
