# Inplace formatting

* C++17
* Aims for max performance. No allocations, reusable buffer, memcpying etc.

No benchmarks yet. Don't look into `benchmark.cpp`.

# Example
Please see `example.cpp`.

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
