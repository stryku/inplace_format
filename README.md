# Inplace formatting

* C++17
* Aims for max performance. No allocations, reusable buffer, memcpying etc.

No benchmarks yet. Don't look into `benchmark.cpp`.

# Abstract
The idea is to based on format string, prepare buffer at compile time. Buffer has enough space for every of the format parameters. E.g. if you specify `{int8}`, buffer has place for `4` characters, to fit all possible values of `std::int8_t` (including minus). If you have `{uint32}`, you have place for `10` digits. And so on. For strings, you need to specify the capacity, e.g. `{str128}` will prepare space for `128` characters.

## Possible format params
* `{int8}` - prepares space for all possible values of `std::int8_t` (including minus).
* `{uint8}` - prepares space for all possible values of `std::uint8_t`.
* `{int16}` - analogical to above, for `std::int16_t`.
* `{uint16}` - analogical to above, for `std::uint16_t`.
* `{int32}` - analogical to above, for `std::int32_t`.
* `{uint32}` - analogical to above, for `std::uint32_t`.
* `{int64}` - analogical to above, for `std::int64_t`.
* `{uint64}` - analogical to above, for `std::uint64_t`.
* `{strN}` - prepares space for `N` chars


# Example
Please see `example.cpp`.
```cpp
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
```

Output:
```
JSON 1:
{{
    "seq_no": 1                   ,
    "value": -2147483648,
    "string content": "some string content"          ,
    "msg_type": "ping"
}}

JSON 2:
{{
    "seq_no": 2                   ,
    "value": 42         ,
    "string content": "some string content"          ,
    "msg_type": "ping"
}}

JSON 3:
{{
    "seq_no": 2                   ,
    "value": 1234       ,
    "string content": "some string content"          ,
    "msg_type": "ping"
}}

JSON 4:
{{
    "seq_no": 2                   ,
    "value": 42         ,
    "string content": "some string content"          ,
    "msg_type": "ping"
}}

JSON 5:
{{
    "seq_no": 2                   ,
    "value": 42...******,
    "string content": "some string content"          ,
    "msg_type": "ping"
}}
```