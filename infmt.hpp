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
      static constexpr auto substr(                                           \
        std::string_view::size_type pos,                                      \
        std::string_view::size_type count = std::string_view::npos)           \
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

struct string_param
{
};

template <typename... Ts>
struct types
{
};

constexpr unsigned length_of(std::string_view s)
{
  if (s.substr(0, 7u) == "{uint8}") {
    return std::numeric_limits<std::uint8_t>::digits10;
  } else if (s.substr(0, 6u) == "{int8}") {
    return std::numeric_limits<std::int8_t>::digits10 + 1u; // 1u for minus
  } else if (s.substr(0, 8u) == "{uint16}") {
    return std::numeric_limits<std::uint16_t>::digits10;
  } else if (s.substr(0, 7u) == "{int16}") {
    return std::numeric_limits<std::int16_t>::digits10 + 1u; // 1u for minus
  } else if (s.substr(0, 8u) == "{uint32}") {
    return std::numeric_limits<std::uint32_t>::digits10;
  } else if (s.substr(0, 7u) == "{int32}") {
    return std::numeric_limits<std::int32_t>::digits10 + 1u; // 1u for minus
  } else if (s.substr(0, 8u) == "{uint64}") {
    return std::numeric_limits<std::uint64_t>::digits10;
  } else if (s.substr(0, 7u) == "{int64}") {
    return std::numeric_limits<std::int64_t>::digits10 + 1u; // 1u for minus
  } else if (s.substr(0, 4u) == "{str") {
    const auto end_pos = s.find('}');
    const auto number_subs = s.substr(4, end_pos - 4);
    return stou(number_subs);
  }

  return s.size();
}

template <unsigned CurrentPos, unsigned CurrentSize = 0u, typename S>
constexpr auto format_param_from(S)
{
  constexpr auto subs = S::substr(CurrentPos);
  constexpr auto length = length_of(subs);
  if constexpr (S::substr(CurrentPos, 7u) == "{uint8}") {
    return format_param<std::uint8_t, CurrentSize, length>{};
  } else if constexpr (S::substr(CurrentPos, 6u) == "{int8}") {
    return format_param<std::int8_t, CurrentSize, length>{};
  } else if constexpr (S::substr(CurrentPos, 8u) == "{uint16}") {
    return format_param<std::uint16_t, CurrentSize, length>{};
  } else if constexpr (S::substr(CurrentPos, 7u) == "{int16}") {
    return format_param<std::int16_t, CurrentSize, length>{};
  } else if constexpr (S::substr(CurrentPos, 8u) == "{uint32}") {
    return format_param<std::uint32_t, CurrentSize, length>{};
  } else if constexpr (S::substr(CurrentPos, 7u) == "{int32}") {
    return format_param<std::int32_t, CurrentSize, length>{};
  } else if constexpr (S::substr(CurrentPos, 8u) == "{uint64}") {
    return format_param<std::uint64_t, CurrentSize, length>{};
  } else if constexpr (S::substr(CurrentPos, 7u) == "{int64}") {
    return format_param<std::int64_t, CurrentSize, length>{};
  } else if constexpr (S::substr(CurrentPos, 4) == "{str") {
    return format_param<string_param, CurrentSize, length>{};
  } else {
    return true;
  }
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

template <typename S, std::string_view::size_type FullLength, typename Params>
struct format_info
{
  using format_t = S;
  static constexpr auto full_length_v = FullLength;
  using params_t = Params;
};

template <std::string_view::size_type CurrentPos,
          std::string_view::size_type CurrentSize = 0u, typename S,
          typename... Params>
constexpr auto collect_format_info(S, Params... params)
{
  constexpr auto current = S::substr(CurrentPos);
  constexpr auto begin_pos = current.find('{');
  if constexpr (begin_pos == std::string_view::npos) {
    return format_info<S, CurrentSize + current.size(), types<Params...>>{};
  } else {
    constexpr auto end_pos = current.find('}', begin_pos);
    constexpr auto subs =
      S::substr(CurrentPos + begin_pos,
                CurrentPos + end_pos + std::string_view::size_type{ 1 });
    constexpr auto param =
      format_param_from<CurrentPos + begin_pos, CurrentSize + begin_pos>(S{});
    return collect_format_info<CurrentPos + end_pos + 1u,
                               CurrentSize + begin_pos + param.length_v>(
      S{}, params..., param);
  }
}

}

constexpr auto make_formatter(std::string_view s)
{
  return 42;
}
}
