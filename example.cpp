#include "infmt.hpp"

#include <cassert>
#include <iostream>
#include <string_view>
#include <type_traits>

int main()
{
  constexpr auto format_str = INFMT_STRING(R"#(
{{
    "seq_no": {uint64},
    "value": {int32},
    "string content": "{str30},
    "msg_type": "ping"
}}
)#");

  // Preparing formatter happens at compile time
  constexpr auto formatter_ct = infmt::make_formatter(format_str);

  // Need a runtime version, because we'll change its state.
  auto formatter = formatter_ct;
  // The view is valid for the formatter lifetime.
  const auto cv = formatter.to_string_view();

  // Setting value of a formatting param. Here, the `seq_no` value.
  formatter.set<0>(1);

  // Min value takes whole space
  formatter.set<1>(std::numeric_limits<std::int32_t>::min());

  auto end_ptr = formatter.set<2>(std::string_view{ "some string content" });
  *end_ptr = '"'; // need to end string manually

  std::cout << "JSON 1:" << cv << "\n";

  // sequence number will only grow, so no need to fill remaining space
  formatter.set<0>(2);

  // Need to fill remaining space to get rid of the previous values' chars
  formatter.set_with_fill<1>(42, ' ');
  std::cout << "JSON 2:" << cv << "\n";

  // Value bigger than the prevous one. No need to fill. Let's remember where
  // the value ends, though.
  const auto max_fill_hint = formatter.set<1>(1234);
  std::cout << "JSON 3:" << cv << "\n";

  // 42 is smaller than 1234. Thanks to remembered `max_fill_hint`, we need to
  // fill only 2 chars instead of all the remaining space.
  formatter.set_with_fill_hint<1>(42, ' ', max_fill_hint);
  std::cout << "JSON 4:" << cv << "\n";

  // Max fill live demonstration. Produces invalid JSON, btw.
  formatter.set_with_fill<1>(1, '*');
  const auto max_fill_hint2 = formatter.set<1>(12345);
  formatter.set_with_fill_hint<1>(42, '.', max_fill_hint2);
  std::cout << "JSON 5:" << cv << "\n";
}
