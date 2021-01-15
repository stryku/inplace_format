#pragma once

#include <limits>
#include <string_view>
#include <tuple>

namespace infmt {

namespace details {
template <typename It, typename Value>
constexpr auto find(It begin, It end, const Value& val)
{
  while (begin != end) {
    if (*begin == val) {
      return begin;
    }
    ++begin;
  }

  return end;
}

template <typename InIt, typename OutIt>
constexpr auto copy(InIt begin, InIt end, OutIt out)
{
  while (begin != end) {
    *out++ = *begin++;
  }
}
}

template <typename... Params>
class formatter
{
public:
  constexpr formatter(std::string_view s, Params... params)
    : m_params{ params... }
  {
  }

private:
  std::tuple<Params...> m_params;
};

namespace details {

template <typename T>
struct format_param
{
  using type = T;

  constexpr format_param(unsigned begin, unsigned size)
    : begin{ begin }
    , size{ size }
  {
  }

  unsigned begin;
  unsigned size;
};

template <typename... Ts>
constexpr auto calc_size(std::string_view s)
{
  auto size = 0u;
  auto current = s;

  while (true) {
    const auto begin_pos = current.find('{');
    if (begin_pos == std::string_view::npos) {
      return size + current.size();
    }

    const auto end_pos = current.find('}', begin_pos);
    const auto subs = current.substr(begin_pos, end_pos + 1u);
    if (subs == "{uint8}") {
      size += std::numeric_limits<std::uint8_t>::digits10;
    } else if (subs == "{int8}") {
      size += std::numeric_limits<std::int8_t>::digits10 + 1u; // 1u for minus
    } else if (subs == "{uint16}") {
      size += std::numeric_limits<std::uint16_t>::digits10;
    } else if (subs == "{int16}") {
      size += std::numeric_limits<std::int16_t>::digits10 + 1u; // 1u for minus
    } else if (subs == "{uint32}") {
      size += std::numeric_limits<std::uint32_t>::digits10;
    } else if (subs == "{int32}") {
      size += std::numeric_limits<std::int32_t>::digits10 + 1u; // 1u for minus
    } else if (subs == "{uint64}") {
      size += std::numeric_limits<std::uint64_t>::digits10;
    } else if (subs == "{int64}") {
      size += std::numeric_limits<std::int64_t>::digits10 + 1u; // 1u for minus
    }
    current = current.substr(end_pos + 1u);
  }
}
}

constexpr auto make_formatter(std::string_view s)
{
  return 42;
}
}
