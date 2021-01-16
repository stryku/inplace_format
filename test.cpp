#include "infmt.hpp"

#include <type_traits>

int main()
{

  static_assert(infmt::details::stou("1") == 1u);
  static_assert(infmt::details::stou("9") == 9u);
  static_assert(infmt::details::stou("10") == 10u);
  static_assert(infmt::details::stou("1234567890") == 1234567890u);

  static_assert(infmt::details::calc_size("") == 0);
  static_assert(infmt::details::calc_size(" ") == 1);

  static_assert(infmt::details::calc_size("{uint8}") == 2);
  static_assert(infmt::details::calc_size("{int8}") == 3);

  static_assert(infmt::details::calc_size("{uint16}") == 4);
  static_assert(infmt::details::calc_size("{int16}") == 5);

  static_assert(infmt::details::calc_size("{uint32}") == 9);
  static_assert(infmt::details::calc_size("{int32}") == 10);

  static_assert(infmt::details::calc_size("{uint64}") == 19);
  static_assert(infmt::details::calc_size("{int64}") == 19);

  static_assert(infmt::details::calc_size("{str1}") == 1);
  static_assert(infmt::details::calc_size("{str9}") == 9);
  static_assert(infmt::details::calc_size("{str10}") == 10u);
  static_assert(infmt::details::calc_size("{str1234567890}") == 1234567890u);

  static_assert(infmt::details::calc_size(
                  "{uint8}{int8}{uint16}{int16}{uint32}{int32}{uint64}{int64}{"
                  "str123}999 9") ==
                2 + 3 + 4 + 5 + 9 + 10 + 19 + 19 + 123 + 5);

  // using type_t =
  //   decltype(infmt::details::format_param_from<0u>(INFMT_STRING("{int8}")));

  {
    constexpr auto s = INFMT_STRING("{uint8}");
    static_assert(
      std::is_same_v<infmt::details::format_param<std::uint8_t, 0, 2>,
                     decltype(infmt::details::format_param_from<0u>(s))>);
  }
  {
    constexpr auto s = INFMT_STRING("{int8}");
    static_assert(
      std::is_same_v<infmt::details::format_param<std::int8_t, 0, 3>,
                     decltype(infmt::details::format_param_from<0u>(s))>);
  }
  {
    constexpr auto s = INFMT_STRING("{uint16}");
    static_assert(
      std::is_same_v<infmt::details::format_param<std::uint16_t, 0, 4>,
                     decltype(infmt::details::format_param_from<0u>(s))>);
  }
  {
    constexpr auto s = INFMT_STRING("{int16}");
    static_assert(
      std::is_same_v<infmt::details::format_param<std::int16_t, 0, 5>,
                     decltype(infmt::details::format_param_from<0u>(s))>);
  }
  {
    constexpr auto s = INFMT_STRING("{uint32}");
    static_assert(
      std::is_same_v<infmt::details::format_param<std::uint32_t, 0, 9>,
                     decltype(infmt::details::format_param_from<0u>(s))>);
  }
  constexpr auto formatter = infmt::make_formatter("{int32}");
}