#include "infmt.hpp"

#include <cassert>
#include <iostream>
#include <type_traits>

template <std::size_t N, std::size_t... Is>
constexpr std::array<char, N - 1> to_array(const char (&a)[N],
                                           std::index_sequence<Is...>)
{
  return { { a[Is]... } };
}

template <std::size_t N>
constexpr std::array<char, N - 1> to_array(const char (&a)[N])
{
  return to_array(a, std::make_index_sequence<N - 1>());
}

template <typename Array>
constexpr bool compare_array(const Array& a, const Array& b)
{
  auto first1 = a.cbegin();
  auto last1 = a.cend();
  auto first2 = b.cbegin();
  for (; first1 != last1; ++first1, ++first2) {
    if (!(*first1 == *first2)) {
      return false;
    }
  }

  return true;
}

int main()
{

  static_assert(infmt::details::stou("1") == 1u);
  static_assert(infmt::details::stou("9") == 9u);
  static_assert(infmt::details::stou("10") == 10u);
  static_assert(infmt::details::stou("1234567890") == 1234567890u);

  static_assert(infmt::details::calc_size("") == 0);
  static_assert(infmt::details::calc_size(" ") == 1);

  static_assert(infmt::details::calc_size("{uint8}") == 3);
  static_assert(infmt::details::calc_size("{int8}") == 4);

  static_assert(infmt::details::calc_size("{uint16}") == 5);
  static_assert(infmt::details::calc_size("{int16}") == 6);

  static_assert(infmt::details::calc_size("{uint32}") == 10);
  static_assert(infmt::details::calc_size("{int32}") == 11);

  static_assert(infmt::details::calc_size("{uint64}") == 20);
  static_assert(infmt::details::calc_size("{int64}") == 20);

  static_assert(infmt::details::calc_size("{str1}") == 1);
  static_assert(infmt::details::calc_size("{str9}") == 9);
  static_assert(infmt::details::calc_size("{str10}") == 10u);
  static_assert(infmt::details::calc_size("{str1234567890}") == 1234567890u);

  static_assert(infmt::details::calc_size(
                  "{uint8}{int8}{uint16}{int16}{uint32}{int32}{uint64}{int64}{"
                  "str123}999 9") ==
                3 + 4 + 5 + 6 + 10 + 11 + 20 + 20 + 123 + 5);

  // using type_t =
  //   decltype(infmt::details::format_param_from<0u>(INFMT_STRING("{int8}")));

  {
    constexpr auto s = INFMT_STRING("{uint8}");
    using s_t = decltype(s);

    static_assert(s_t::substr(0) == "{uint8}");
    static_assert(s_t::substr(1) == "uint8}");
    static_assert(s_t::substr(2) == "int8}");
    static_assert(s_t::substr(3) == "nt8}");
    static_assert(s_t::substr(4) == "t8}");
    static_assert(s_t::substr(5) == "8}");
    static_assert(s_t::substr(6) == "}");
    static_assert(s_t::substr(7) == "");
  }

  {
    constexpr auto s = INFMT_STRING("{uint8}");
    static_assert(
      std::is_same_v<infmt::details::format_param<std::uint8_t, 0, 0, 3, 7u>,
                     decltype(infmt::details::format_param_from<0u>(s))>);
  }
  {
    constexpr auto s = INFMT_STRING("{int8}");
    static_assert(
      std::is_same_v<infmt::details::format_param<std::int8_t, 0, 0, 4, 6u>,
                     decltype(infmt::details::format_param_from<0u>(s))>);
  }
  {
    constexpr auto s = INFMT_STRING("{uint16}");
    static_assert(
      std::is_same_v<infmt::details::format_param<std::uint16_t, 0, 0, 5, 8u>,
                     decltype(infmt::details::format_param_from<0u>(s))>);
  }
  {
    constexpr auto s = INFMT_STRING("{int16}");
    static_assert(
      std::is_same_v<infmt::details::format_param<std::int16_t, 0, 0, 6, 7u>,
                     decltype(infmt::details::format_param_from<0u>(s))>);
  }
  {
    constexpr auto s = INFMT_STRING("{uint32}");
    static_assert(
      std::is_same_v<infmt::details::format_param<std::uint32_t, 0, 0, 10, 8u>,
                     decltype(infmt::details::format_param_from<0u>(s))>);
  }
  {
    constexpr auto s = INFMT_STRING("{int32}");
    static_assert(
      std::is_same_v<infmt::details::format_param<std::int32_t, 0, 0, 11, 7u>,
                     decltype(infmt::details::format_param_from<0u>(s))>);
  }
  {
    constexpr auto s = INFMT_STRING("{uint64}");
    static_assert(
      std::is_same_v<infmt::details::format_param<std::uint64_t, 0, 0, 20, 8u>,
                     decltype(infmt::details::format_param_from<0u>(s))>);
  }
  {
    constexpr auto s = INFMT_STRING("{int64}");
    static_assert(
      std::is_same_v<infmt::details::format_param<std::int64_t, 0, 0, 20, 7u>,
                     decltype(infmt::details::format_param_from<0u>(s))>);
  }
  {
    constexpr auto s = INFMT_STRING("{str1234567890}");
    static_assert(
      std::is_same_v<infmt::details::format_param<infmt::details::string_param,
                                                  0, 0, 1234567890, 15u>,
                     decltype(infmt::details::format_param_from<0u>(s))>);
  }

  // collecting params
  {
    constexpr auto s = INFMT_STRING("");
    constexpr auto result = infmt::details::collect_format_info<0u>(s);
    using result_t = std::remove_cv_t<decltype(result)>;
    using expected_params_t = infmt::details::types<>;

    static_assert(result_t::full_length_v == 0u);
    static_assert(
      std::is_same_v<typename result_t::params_t, expected_params_t>);
  }
  {
    constexpr auto s = INFMT_STRING("{uint8}");
    constexpr auto result = infmt::details::collect_format_info<0u>(s);
    using result_t = std::remove_cv_t<decltype(result)>;
    using expected_params_t = infmt::details::types<
      infmt::details::format_param<std::uint8_t, 0, 0, 3, 7u>>;

    static_assert(result_t::full_length_v == 3u);
    static_assert(
      std::is_same_v<typename result_t::params_t, expected_params_t>);
  }
  {
    constexpr auto s = INFMT_STRING("{uint8}{int8}");
    constexpr auto result = infmt::details::collect_format_info<0u>(s);
    using result_t = std::remove_cv_t<decltype(result)>;
    using expected_params_t = infmt::details::types<
      infmt::details::format_param<std::uint8_t, 0, 0, 3, 7u>,
      infmt::details::format_param<std::int8_t, 7u, 3, 4, 6u>>;

    static_assert(result_t::full_length_v == 7u);
    static_assert(
      std::is_same_v<typename result_t::params_t, expected_params_t>);
  }
  {
    constexpr auto s =
      INFMT_STRING(" {uint8} {int8} {uint16} {int16} {uint32} {int32} "
                   "{uint64} {int64} {str123} ");
    constexpr auto result = infmt::details::collect_format_info<0u>(s);
    using result_t = std::remove_cv_t<decltype(result)>;
    using expected_t = infmt::details::types<
      infmt::details::format_param<std::uint8_t, 1u, 1, 3, 7u>,
      infmt::details::format_param<std::int8_t, 9u, 5, 4, 6u>,
      infmt::details::format_param<std::uint16_t, 16u, 10, 5, 8u>,
      infmt::details::format_param<std::int16_t, 25u, 16, 6, 7u>,
      infmt::details::format_param<std::uint32_t, 33u, 23, 10, 8u>,
      infmt::details::format_param<std::int32_t, 42u, 34, 11, 7u>,
      infmt::details::format_param<std::uint64_t, 50u, 46, 20, 8u>,
      infmt::details::format_param<std::int64_t, 59, 67, 20, 7u>,
      infmt::details::format_param<infmt::details::string_param, 67, 88, 123,
                                   8u>>;

    static_assert(result_t::full_length_v == 212);
    static_assert(std::is_same_v<typename result_t::params_t, expected_t>);
  }
  {
    constexpr auto s =
      INFMT_STRING("|{uint8}|{int8}|{uint16}|{int16}|{uint32}|{int32}|"
                   "{uint64}|{int64}|{str42}|");
    constexpr auto buffer = infmt::details::make_buffer(s);

    constexpr auto expected = to_array(
      "|   |    |     |      |          |           |                    |    "
      "                |                                          |");
    static_assert(compare_array(buffer, expected));
    std::cout << "'" << std::string{ buffer.cbegin(), buffer.cend() } << "'\n";
    std::cout << "'" << std::string{ expected.cbegin(), expected.cend() }
              << "'\n";
  }
  {
    auto formatter = infmt::make_formatter(INFMT_STRING("{int32}"));
    formatter.set<0>(42);
    const auto cv = formatter.to_string_view();
    assert(cv == "-2147483648");
    std::cout << "'" << cv << "'\n";
  }
  {
    auto formatter = infmt::make_formatter(INFMT_STRING("{int32}"));
    formatter.set<0>(std::numeric_limits<int32_t>::min());
    const auto cv = formatter.to_string_view();
    assert(cv == "42         ");
    std::cout << "'" << cv << "'\n";
  }
  {
    auto formatter = infmt::make_formatter(INFMT_STRING("|{int32}|{uint64}|"));
    formatter.set<0>(std::numeric_limits<int32_t>::min());
    formatter.set<1>(std::numeric_limits<uint64_t>::max());
    const auto cv = formatter.to_string_view();
    assert(cv == "|-2147483648|18446744073709551615|");
    std::cout << "'" << cv << "'\n";
  }
  {
    auto formatter = infmt::make_formatter(INFMT_STRING("|{uint64}|"));
    formatter.set<0>(std::numeric_limits<uint64_t>::max());
    formatter.set_with_fill<0>(42, '*');
    const auto cv = formatter.to_string_view();
    assert(cv == "|42******************|");
    std::cout << "'" << cv << "'\n";
  }
  {
    auto formatter = infmt::make_formatter(INFMT_STRING("|{uint64}|"));
    formatter.set<0>(std::numeric_limits<uint64_t>::max());
    formatter.set_with_fill<0>(42, '*');
    const auto cv = formatter.to_string_view();
    assert(cv == "|42******************|");
    std::cout << "'" << cv << "'\n";

    const auto max_fill_hint = formatter.set<0>(1234);
    assert(cv == "|1234****************|");
    std::cout << "'" << cv << "'\n";

    formatter.set_with_fill_hint<0>(42, '.', max_fill_hint);
    assert(cv == "|42..****************|");
    std::cout << "'" << cv << "'\n";
  }
}
