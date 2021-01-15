#include "infmt.hpp"

int main()
{
  static_assert(infmt::details::calc_size("") == 0);
  static_assert(infmt::details::calc_size(" ") == 1);
  static_assert(infmt::details::calc_size("{int32}") == 10);

  constexpr auto formatter = infmt::make_formatter("{int32}");
}