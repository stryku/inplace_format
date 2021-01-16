#pragma once

#include <array>
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

template <typename InIt, typename Value>
constexpr auto fill(InIt begin, InIt end, Value val)
{
  while (begin != end) {
    *begin++ = val;
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

template <typename T, unsigned PosInOriginal, unsigned PosInOutputBuffer,
          unsigned Length, unsigned FormatSize>
struct format_param
{
  using type_t = T;
  static constexpr auto pos_in_original_v = PosInOriginal;
  static constexpr auto pos_in_output_buffer_v = PosInOutputBuffer;
  static constexpr auto length_v = Length;
  static constexpr auto format_size_v = FormatSize;
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
  constexpr auto format_length = subs.find('}') + 1u;
  if constexpr (S::substr(CurrentPos, 7u) == "{uint8}") {
    return format_param<std::uint8_t, CurrentPos, CurrentSize, length,
                        format_length>{};
  } else if constexpr (S::substr(CurrentPos, 6u) == "{int8}") {
    return format_param<std::int8_t, CurrentPos, CurrentSize, length,
                        format_length>{};
  } else if constexpr (S::substr(CurrentPos, 8u) == "{uint16}") {
    return format_param<std::uint16_t, CurrentPos, CurrentSize, length,
                        format_length>{};
  } else if constexpr (S::substr(CurrentPos, 7u) == "{int16}") {
    return format_param<std::int16_t, CurrentPos, CurrentSize, length,
                        format_length>{};
  } else if constexpr (S::substr(CurrentPos, 8u) == "{uint32}") {
    return format_param<std::uint32_t, CurrentPos, CurrentSize, length,
                        format_length>{};
  } else if constexpr (S::substr(CurrentPos, 7u) == "{int32}") {
    return format_param<std::int32_t, CurrentPos, CurrentSize, length,
                        format_length>{};
  } else if constexpr (S::substr(CurrentPos, 8u) == "{uint64}") {
    return format_param<std::uint64_t, CurrentPos, CurrentSize, length,
                        format_length>{};
  } else if constexpr (S::substr(CurrentPos, 7u) == "{int64}") {
    return format_param<std::int64_t, CurrentPos, CurrentSize, length,
                        format_length>{};
  } else if constexpr (S::substr(CurrentPos, 4) == "{str") {
    return format_param<string_param, CurrentPos, CurrentSize, length,
                        format_length>{};
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

template <typename S, typename Buffer, typename... Params>
constexpr auto fill_buffer(Buffer& buffer, types<Params...>)
{
  // using a = typename types<Params...>::asdads;
  constexpr auto s = S::to_string_view();
  auto current_original = s.cbegin();
  auto current_buffer = buffer.begin();

  auto filling_impl = [&](auto param) {
    const auto param_begin_in_original_it =
      std::next(s.cbegin(), param.pos_in_original_v);

    // Copy plain string that's before parameter
    const auto plain_begin = current_original;
    const auto plain_end = param_begin_in_original_it;
    const auto plain_length = std::distance(plain_begin, plain_end);
    copy(plain_begin, plain_end, current_buffer);
    // const auto to_advance = plain_length + param.format;
    current_original = param_begin_in_original_it;
    // std::advance(current_original, to_advance);
    std::advance(current_buffer, plain_length);

    // Fill param place with spaces
    const auto param_begin = current_buffer;
    const auto param_end = std::next(current_buffer, param.length_v);
    fill(param_begin, param_end, ' ');
    std::advance(current_original, param.format_size_v);
    std::advance(current_buffer, param.length_v);
  };

  auto filler = [&](auto... params) { (filling_impl(params), ...); };
  filler(Params{}...);

  // Fill last part of plain
  copy(current_original, s.cend(), current_buffer);
}

template <typename S>
constexpr auto make_buffer(S)
{
  constexpr auto info = collect_format_info<0u, 0u>(S{});
  std::array<char, info.full_length_v> buffer{};

  fill_buffer<S>(buffer, typename decltype(info)::params_t{});

  return buffer;
}

}

constexpr auto make_formatter(std::string_view s)
{
  return 42;
}
}
