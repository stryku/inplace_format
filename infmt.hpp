#pragma once

#include <limits>
#include <string_view>
#include <tuple>

#define INFMT_STRING(s)                                                       \
  [] {                                                                        \
    struct str                                                                \
    {                                                                         \
      static constexpr auto to_string_view()                                  \
      {                                                                       \
        return std::string_view{ s };                                         \
      }                                                                       \
                                                                              \
      static constexpr auto substr(std::string_view::size_type pos,           \
                                   std::string_view::size_type count)         \
      {                                                                       \
        return to_string_view().substr(pos, count);                           \
      }                                                                       \
                                                                              \
      static constexpr decltype(auto) to_raw_str() { return s; }              \
    };                                                                        \
    return str{};                                                             \
  }()

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

constexpr auto stou(std::string_view str)
{
  unsigned value{};
  unsigned mul{ 1u };

  for (const auto c : str) {
    value = value * mul + (c - '0');
    mul = 10u;
  }

  return value;
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

template <typename T, unsigned Begin, unsigned Length>
struct format_param
{
  using type_t = T;
  static constexpr auto begin_v = Begin;
  static constexpr auto length_v = Length;
};

template <typename... Ts>
struct types
{
};

constexpr unsigned length_of(std::string_view s)
{
  if (s == "{uint8}") {
    return std::numeric_limits<std::uint8_t>::digits10;
  } else if (s == "{int8}") {
    return std::numeric_limits<std::int8_t>::digits10 + 1u; // 1u for minus
  } else if (s == "{uint16}") {
    return std::numeric_limits<std::uint16_t>::digits10;
  } else if (s == "{int16}") {
    return std::numeric_limits<std::int16_t>::digits10 + 1u; // 1u for minus
  } else if (s == "{uint32}") {
    return std::numeric_limits<std::uint32_t>::digits10;
  } else if (s == "{int32}") {
    return std::numeric_limits<std::int32_t>::digits10 + 1u; // 1u for minus
  } else if (s == "{uint64}") {
    return std::numeric_limits<std::uint64_t>::digits10;
  } else if (s == "{int64}") {
    return std::numeric_limits<std::int64_t>::digits10 + 1u; // 1u for minus
  } else if (s.substr(0, 4) == "{str") {
    const auto end_pos = s.find('}');
    const auto number_subs = s.substr(4, end_pos - 4);
    return stou(number_subs);
  }

  return s.size();
}

template <unsigned CurrentPos, typename S>
constexpr auto format_param_from(S)
{
  constexpr auto length = length_of(S::to_string_view());
  if constexpr (S::substr(CurrentPos, 7u) == "{uint8}") {
    return format_param<std::uint8_t, CurrentPos, length>{};
  } else if constexpr (S::substr(CurrentPos, 6u) == "{int8}") {
    return format_param<std::int8_t, CurrentPos, length>{};
  } else if constexpr (S::substr(CurrentPos, 8u) == "{uint16}") {
    return format_param<std::uint16_t, CurrentPos, length>{};
  } else if constexpr (S::substr(CurrentPos, 7u) == "{int16}") {
    return format_param<std::int16_t, CurrentPos, length>{};
  } else if constexpr (S::substr(CurrentPos, 8u) == "{uint32}") {
    return format_param<std::uint32_t, CurrentPos, length>{};
  } else if constexpr (S::substr(CurrentPos, 7u) == "{int32}") {
    return format_param<std::int32_t, CurrentPos, length>{};
  } else if constexpr (S::substr(CurrentPos, 8u) == "{uint64}") {
    return format_param<std::uint64_t, CurrentPos, length>{};
  } else if constexpr (S::substr(CurrentPos, 7u) == "{int64}") {
    return format_param<std::int64_t, CurrentPos, length>{};
  } else {
    return true;
  }

  // else if (s == "{int8}") {
  //   return std::numeric_limits<std::int8_t>::digits10 + 1u; // 1u for minus
  // } else if (s == "{uint16}") {
  //   return std::numeric_limits<std::uint16_t>::digits10;
  // } else if (s == "{int16}") {
  //   return std::numeric_limits<std::int16_t>::digits10 + 1u; // 1u for minus
  // } else if (s == "{uint32}") {
  //   return std::numeric_limits<std::uint32_t>::digits10;
  // } else if (s == "{int32}") {
  //   return std::numeric_limits<std::int32_t>::digits10 + 1u; // 1u for minus
  // } else if (s == "{uint64}") {
  //   return std::numeric_limits<std::uint64_t>::digits10;
  // } else if (s == "{int64}") {
  //   return std::numeric_limits<std::int64_t>::digits10 + 1u; // 1u for minus
  // }
}

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
    size += length_of(subs);
    current = current.substr(end_pos + 1u);
  }
}

// template <typename... Params>
// constexpr auto collect_param_types(std::string_view s, unsigned current_pos,
//                                    Params... params)
// {
//   const auto current = s.substr(current_pos);
//   const auto begin_pos = current.find('{');
//   if (begin_pos == std::string_view::npos) {
//     return types<Params...>{};
//   }

//   const auto end_pos = current.find('}', begin_pos);
//   const auto subs = current.substr(begin_pos, end_pos + 1u);
//   size += length_of(subs);
// }

}

constexpr auto make_formatter(std::string_view s)
{
  return 42;
}
}
