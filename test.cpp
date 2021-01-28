#include "infmt.hpp"

#include <cassert>
#include <iostream>
#include <type_traits>

template <class T>
struct dependent_false : std::false_type
{
};

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

template <typename T>
constexpr auto type_dependent_calc_size(std::string_view s)
{
  return infmt::details::calc_size(s);
};

int main()
{

  static_assert(infmt::details::stou("1") == 1u);
  static_assert(infmt::details::stou("9") == 9u);
  static_assert(infmt::details::stou("10") == 10u);
  static_assert(infmt::details::stou("1234567890") == 1234567890u);

  static_assert(infmt::details::calc_size("") == 0);
  static_assert(infmt::details::calc_size(" ") == 1);

  static_assert(infmt::details::calc_size("{uint8_t}") == 3);
  static_assert(infmt::details::calc_size("{int8_t}") == 4);

  static_assert(infmt::details::calc_size("{uint16_t}") == 5);
  static_assert(infmt::details::calc_size("{int16_t}") == 6);

  static_assert(infmt::details::calc_size("{uint32_t}") == 10);
  static_assert(infmt::details::calc_size("{int32_t}") == 11);

  static_assert(infmt::details::calc_size("{uint64_t}") == 20);
  static_assert(infmt::details::calc_size("{int64_t}") == 20);

  static_assert(infmt::details::calc_size("{str1}") == 1);
  static_assert(infmt::details::calc_size("{str9}") == 9);
  static_assert(infmt::details::calc_size("{str10}") == 10u);
  static_assert(infmt::details::calc_size("{str1234567890}") == 1234567890u);

  // Test int
  const auto int_tester = [](auto val) {
    using value_t = decltype(val);

    if constexpr (sizeof(value_t) == 4u) {
      static_assert(type_dependent_calc_size<value_t>("{int}") == 11);
    } else if constexpr (sizeof(value_t) == 8u) {
      static_assert(type_dependent_calc_size<value_t>("{int}") == 20);
    } else {
      static_assert(dependent_false<decltype(val)>::value,
                    "Architecture not supported");
    }
  };

  int_tester(int{});

  // Test unsigned
  const auto unsigned_tester = [](auto val) {
    using value_t = decltype(val);

    if constexpr (sizeof(value_t) == 4u) {
      static_assert(type_dependent_calc_size<value_t>("{unsigned}") == 10);
    } else if constexpr (sizeof(value_t) == 8u) {
      static_assert(type_dependent_calc_size<value_t>("{unsigned}") == 20);
    } else {
      static_assert(dependent_false<value_t>::value,
                    "Architecture not supported");
    }
  };

  unsigned_tester(unsigned{});

  static_assert(
    infmt::details::calc_size("{uint8_t}{int8_t}{uint16_t}{int16_t}{uint32_t}{"
                              "int32_t}{uint64_t}{int64_t}{"
                              "str123}999 9") ==
    3 + 4 + 5 + 6 + 10 + 11 + 20 + 20 + 123 + 5);

  // using type_t =
  //   decltype(infmt::details::format_param_from<0u>(INFMT_STRING("{int8_t}")));

  {
    constexpr auto s = INFMT_STRING("{uint8_t}");
    using s_t = decltype(s);

    static_assert(s_t::substr(0) == "{uint8_t}");
    static_assert(s_t::substr(1) == "uint8_t}");
    static_assert(s_t::substr(2) == "int8_t}");
    static_assert(s_t::substr(3) == "nt8_t}");
    static_assert(s_t::substr(4) == "t8_t}");
    static_assert(s_t::substr(5) == "8_t}");
    static_assert(s_t::substr(6) == "_t}");
    static_assert(s_t::substr(7) == "t}");
    static_assert(s_t::substr(8) == "}");
    static_assert(s_t::substr(9) == "");
  }

  {
    constexpr auto s = INFMT_STRING("{uint8_t}");
    static_assert(
      std::is_same_v<infmt::details::format_param<std::uint8_t, 0, 0, 3, 9u>,
                     decltype(infmt::details::format_param_from<0u>(s))>);
  }
  {
    constexpr auto s = INFMT_STRING("{int8_t}");
    static_assert(
      std::is_same_v<infmt::details::format_param<std::int8_t, 0, 0, 4, 8u>,
                     decltype(infmt::details::format_param_from<0u>(s))>);
  }
  {
    constexpr auto s = INFMT_STRING("{uint16_t}");
    static_assert(
      std::is_same_v<infmt::details::format_param<std::uint16_t, 0, 0, 5, 10u>,
                     decltype(infmt::details::format_param_from<0u>(s))>);
  }
  {
    constexpr auto s = INFMT_STRING("{int16_t}");
    static_assert(
      std::is_same_v<infmt::details::format_param<std::int16_t, 0, 0, 6, 9u>,
                     decltype(infmt::details::format_param_from<0u>(s))>);
  }
  {
    constexpr auto s = INFMT_STRING("{uint32_t}");
    static_assert(std::is_same_v<
                  infmt::details::format_param<std::uint32_t, 0, 0, 10, 10u>,
                  decltype(infmt::details::format_param_from<0u>(s))>);
  }
  {
    constexpr auto s = INFMT_STRING("{int32_t}");
    static_assert(
      std::is_same_v<infmt::details::format_param<std::int32_t, 0, 0, 11, 9u>,
                     decltype(infmt::details::format_param_from<0u>(s))>);
  }
  {
    constexpr auto s = INFMT_STRING("{uint64_t}");
    static_assert(std::is_same_v<
                  infmt::details::format_param<std::uint64_t, 0, 0, 20, 10u>,
                  decltype(infmt::details::format_param_from<0u>(s))>);
  }
  {
    constexpr auto s = INFMT_STRING("{int64_t}");
    static_assert(
      std::is_same_v<infmt::details::format_param<std::int64_t, 0, 0, 20, 9u>,
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
    constexpr auto s = INFMT_STRING("{uint8_t}");
    constexpr auto result = infmt::details::collect_format_info<0u>(s);
    using result_t = std::remove_cv_t<decltype(result)>;
    using expected_params_t = infmt::details::types<
      infmt::details::format_param<std::uint8_t, 0, 0, 3, 9u>>;

    static_assert(result_t::full_length_v == 3u);
    static_assert(
      std::is_same_v<typename result_t::params_t, expected_params_t>);
  }
  {
    constexpr auto s = INFMT_STRING("{uint8_t}{int8_t}");
    constexpr auto result = infmt::details::collect_format_info<0u>(s);
    using result_t = std::remove_cv_t<decltype(result)>;
    using expected_params_t = infmt::details::types<
      infmt::details::format_param<std::uint8_t, 0, 0, 3, 9u>,
      infmt::details::format_param<std::int8_t, 9u, 3, 4, 8u>>;

    static_assert(result_t::full_length_v == 7u);
    static_assert(
      std::is_same_v<typename result_t::params_t, expected_params_t>);
  }
  {
    constexpr auto s = INFMT_STRING(
      " {uint8_t} {int8_t} {uint16_t} {int16_t} {uint32_t} {int32_t} "
      "{uint64_t} {int64_t} {str123} ");
    constexpr auto result = infmt::details::collect_format_info<0u>(s);
    using result_t = std::remove_cv_t<decltype(result)>;
    using expected_t = infmt::details::types<
      infmt::details::format_param<std::uint8_t, 1u, 1, 3, 9u>,
      infmt::details::format_param<std::int8_t, 11u, 5, 4, 8u>,
      infmt::details::format_param<std::uint16_t, 20u, 10, 5, 10u>,
      infmt::details::format_param<std::int16_t, 31u, 16, 6, 9u>,
      infmt::details::format_param<std::uint32_t, 41u, 23, 10, 10u>,
      infmt::details::format_param<std::int32_t, 52u, 34, 11, 9u>,
      infmt::details::format_param<std::uint64_t, 62u, 46, 20, 10u>,
      infmt::details::format_param<std::int64_t, 73u, 67, 20, 9u>,
      infmt::details::format_param<infmt::details::string_param, 83, 88, 123,
                                   8u>>;

    static_assert(result_t::full_length_v == 212);
    static_assert(std::is_same_v<typename result_t::params_t, expected_t>);
  }
  {
    constexpr auto s = INFMT_STRING(
      "|{uint8_t}|{int8_t}|{uint16_t}|{int16_t}|{uint32_t}|{int32_t}|"
      "{uint64_t}|{int64_t}|{str42}|");
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
    auto formatter = infmt::make_formatter(INFMT_STRING("{int32_t}"));
    formatter.set<0>(42);
    const auto cv = formatter.to_string_view();
    assert(cv == "-2147483648");
    std::cout << "'" << cv << "'\n";
  }
  {
    auto formatter = infmt::make_formatter(INFMT_STRING("{int32_t}"));
    formatter.set<0>(std::numeric_limits<int32_t>::min());
    const auto cv = formatter.to_string_view();
    assert(cv == "42         ");
    std::cout << "'" << cv << "'\n";
  }
  {
    auto formatter =
      infmt::make_formatter(INFMT_STRING("|{int32_t}|{uint64_t}|"));
    formatter.set<0>(std::numeric_limits<int32_t>::min());
    formatter.set<1>(std::numeric_limits<uint64_t>::max());
    const auto cv = formatter.to_string_view();
    assert(cv == "|-2147483648|18446744073709551615|");
    std::cout << "'" << cv << "'\n";
  }
  {
    auto formatter = infmt::make_formatter(INFMT_STRING("|{uint64_t}|"));
    formatter.set<0>(std::numeric_limits<uint64_t>::max());
    formatter.set_with_fill<0>(42, '*');
    const auto cv = formatter.to_string_view();
    assert(cv == "|42******************|");
    std::cout << "'" << cv << "'\n";
  }
  {
    auto formatter = infmt::make_formatter(INFMT_STRING("|{uint64_t}|"));
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
  {
    constexpr auto format_str = INFMT_STRING(R"#(
{{
    "seq_no": {uint64_t},
}}
)#");
    auto formatter = infmt::make_formatter(format_str);
  }
}
