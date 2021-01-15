#include "infmt.hpp"

int main()
{
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

  constexpr auto formatter = infmt::make_formatter("{int32}");
}