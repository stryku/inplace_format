#pragma once

#include <array>
#include <charconv>
#include <limits>
#include <optional>
#include <string_view>
#include <tuple>
#include <type_traits>

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

template <typename T>
constexpr unsigned max_chars_in_type(unsigned base = 10)
{
  unsigned count{};
  auto value = std::numeric_limits<T>::max();

  while (value > 0u) {
    ++count;
    value /= base;
  }

  return count + std::is_signed_v<T>;
}

template <typename T>
constexpr unsigned max_chars_in_floating_type()
{
  if constexpr (std::is_same_v<T, float>) {
    return 48u;
  } else if constexpr (std::is_same_v<T, double>) {
    return 327u;
  } else {
    return 4954u;
  }
}
}

namespace details {

template <typename T>
class span
{
public:
  constexpr span() = default;
  constexpr span(T* ptr, unsigned size)
    : m_ptr{ ptr }
    , m_size{ size }
  {
  }

  auto begin() { return m_ptr; }
  auto begin() const { return m_ptr; }
  auto end() { return std::next(m_ptr, m_size); }
  auto end() const { return std::next(m_ptr, m_size); }

private:
  T* m_ptr{};
  unsigned m_size{};
};

template <typename T, unsigned PosInOriginal, unsigned PosInOutputBuffer,
          unsigned Length, unsigned FormatSize>
struct format_param
{
  using type_t = T;
  static constexpr auto pos_in_original_v = PosInOriginal;
  static constexpr auto pos_in_output_buffer_v = PosInOutputBuffer;
  static constexpr auto length_v = Length;
  static constexpr auto format_size_v = FormatSize;

  template <typename Buffer>
  static auto to_span(Buffer& buffer)
  {
    return span{ std::next(buffer.data(), pos_in_output_buffer_v), length_v };
  }
};

struct string_param
{
};

template <typename... Ts>
struct types
{
};

enum class param_kind
{
  bool_,
  char_,
  signed_char,
  unsigned_char,
  short_int,
  unsigned_short_int,
  int_,
  unsigned_int,
  long_int,
  unsigned_long_int,
  long_long_int,
  unsigned_long_long_int,
  float_,
  double_,
  long_double,
  uint8,
  int8,
  uint16,
  int16,
  uint32,
  int32,
  uint64,
  int64,
  str
};

constexpr std::optional<param_kind> format_str_to_kind(std::string_view s)
{
  if (s.substr(0, 4u) == "{str") {
    return param_kind::str;
  }
  if (s == "{bool}") {
    return param_kind::bool_;
  }
  if (s == "{char}") {
    return param_kind::char_;
  }
  if (s == "{signed char}") {
    return param_kind::signed_char;
  }
  if (s == "{unsigned char}") {
    return param_kind::unsigned_char;
  }
  // Todo measure whether compilation time decreases whether `ifs` for
  // uncommonly used types (like signed) are moved to the end of function
  if (s == "{short}" || s == "{short int}" || s == "{signed short}" ||
      s == "{signed short int}") {
    return param_kind::short_int;
  }
  if (s == "{unsigned short}" || s == "{unsigned short int}") {
    return param_kind::unsigned_short_int;
  }
  if (s == "{int}" || s == "{signed}" || s == "{signed int}") {
    return param_kind::int_;
  }
  if (s == "{unsigned}" || s == "{unsigned int}") {
    return param_kind::unsigned_int;
  }
  if (s == "{long}" || s == "{long int}" || s == "{signed long}" ||
      s == "{signed long int}") {
    return param_kind::long_int;
  }
  if (s == "{unsigned long}" || s == "{unsigned long int}") {
    return param_kind::unsigned_long_int;
  }
  if (s == "{long long}" || s == "{long long int}" ||
      s == "{signed long long}" || s == "{signed long long int}") {
    return param_kind::long_long_int;
  }
  if (s == "{unsigned long long}" || s == "{unsigned long long int}") {
    return param_kind::unsigned_long_long_int;
  }
  if (s == "{float}") {
    return param_kind::float_;
  }
  if (s == "{double}") {
    return param_kind::double_;
  }
  if (s == "{long double}") {
    return param_kind::long_double;
  }
  if (s == "{uint8_t}") {
    return param_kind::uint8;
  }
  if (s == "{int8_t}") {
    return param_kind::int8;
  }
  if (s == "{uint16_t}") {
    return param_kind::uint16;
  }
  if (s == "{int16_t}") {
    return param_kind::int16;
  }
  if (s == "{uint32_t}") {
    return param_kind::uint32;
  }
  if (s == "{int32_t}") {
    return param_kind::int32;
  }
  if (s == "{uint64_t}") {
    return param_kind::uint64;
  }
  if (s == "{int64_t}") {
    return param_kind::int64;
  }

  return std::nullopt;
}

constexpr unsigned max_length_of(param_kind kind, std::string_view s)
{
  switch (kind) {
    case param_kind::bool_: {
      // 'true' or 'false'
      return 5u;
    }
    case param_kind::char_: {
      return max_chars_in_type<char>();
    }
    case param_kind::signed_char: {
      return max_chars_in_type<signed char>();
    }
    case param_kind::unsigned_char: {
      return max_chars_in_type<unsigned char>();
    }
    case param_kind::short_int: {
      return max_chars_in_type<short int>();
    }
    case param_kind::unsigned_short_int: {
      return max_chars_in_type<unsigned short int>();
    }
    case param_kind::int_: {
      return max_chars_in_type<int>();
    }
    case param_kind::unsigned_int: {
      return max_chars_in_type<unsigned int>();
    }
    case param_kind::long_int: {
      return max_chars_in_type<long int>();
    }
    case param_kind::unsigned_long_int: {
      return max_chars_in_type<unsigned long int>();
    }
    case param_kind::long_long_int: {
      return max_chars_in_type<long long int>();
    }
    case param_kind::unsigned_long_long_int: {
      return max_chars_in_type<unsigned long long int>();
    }
    case param_kind::float_: {
      return max_chars_in_floating_type<float>();
    }
    case param_kind::double_: {
      return max_chars_in_floating_type<double>();
    }
    case param_kind::long_double: {
      return max_chars_in_floating_type<long double>();
    }
    case param_kind::uint8: {
      return max_chars_in_type<std::uint8_t>();
    }
    case param_kind::int8: {
      return max_chars_in_type<std::int8_t>();
    }
    case param_kind::uint16: {
      return max_chars_in_type<std::uint16_t>();
    }
    case param_kind::int16: {
      return max_chars_in_type<std::int16_t>();
    }
    case param_kind::uint32: {
      return max_chars_in_type<std::uint32_t>();
    }
    case param_kind::int32: {
      return max_chars_in_type<std::int32_t>();
    }
    case param_kind::uint64: {
      return max_chars_in_type<std::uint64_t>();
    }
    case param_kind::int64: {
      return max_chars_in_type<std::int64_t>();
    }
    case param_kind::str: {
      const auto end_pos = s.find('}');
      const auto number_subs = s.substr(4, end_pos - 4);
      return stou(number_subs);
    }
  }
}

constexpr unsigned max_length_of(std::string_view s)
{
  const auto kind = format_str_to_kind(s);

  if (kind == std::nullopt) {
    // Handle properly
    return 0;
  }

  return max_length_of(*kind, s);
}

template <unsigned CurrentPos, unsigned CurrentSize = 0u, typename S>
constexpr auto format_param_from(S)
{
  constexpr auto subs = S::substr(CurrentPos);
  constexpr auto format_length = subs.find('}') + 1u;
  constexpr auto param_format_string = S::substr(CurrentPos, format_length);
  constexpr auto kind = format_str_to_kind(param_format_string);
  if constexpr (kind == std::nullopt) {
    // Todo: handle properly
    return true;
  }

  constexpr auto max_length = max_length_of(*kind, param_format_string);

  if constexpr (kind == param_kind::uint8) {
    return format_param<std::uint8_t, CurrentPos, CurrentSize, max_length,
                        format_length>{};
  } else if constexpr (kind == param_kind::int8) {
    return format_param<std::int8_t, CurrentPos, CurrentSize, max_length,
                        format_length>{};
  } else if constexpr (kind == param_kind::uint16) {
    return format_param<std::uint16_t, CurrentPos, CurrentSize, max_length,
                        format_length>{};
  } else if constexpr (kind == param_kind::int16) {
    return format_param<std::int16_t, CurrentPos, CurrentSize, max_length,
                        format_length>{};
  } else if constexpr (kind == param_kind::uint32) {
    return format_param<std::uint32_t, CurrentPos, CurrentSize, max_length,
                        format_length>{};
  } else if constexpr (kind == param_kind::int32) {
    return format_param<std::int32_t, CurrentPos, CurrentSize, max_length,
                        format_length>{};
  } else if constexpr (kind == param_kind::uint64) {
    return format_param<std::uint64_t, CurrentPos, CurrentSize, max_length,
                        format_length>{};
  } else if constexpr (kind == param_kind::int64) {
    return format_param<std::int64_t, CurrentPos, CurrentSize, max_length,
                        format_length>{};
  } else if constexpr (kind == param_kind::str) {
    return format_param<string_param, CurrentPos, CurrentSize, max_length,
                        format_length>{};
  } else {
    return true;
  }
}
constexpr auto calc_size(std::string_view s)
{
  auto size = 0u;

  for (auto i = 0u; i < s.size();) {
    const auto c = s[i];

    if (c == '}') {
      // Found ending brace without begin brace. Next char can only be another
      // ending brace.
      if (i + 1u >= s.size()) {
        // Todo report malformed format stirng. } without {
      }

      if (s[i + 1u] != '}') {
        // Todo report malformed format stirng. } without {
      }

      // Got }}. Treat it as one char } in output string
      ++size;
      i += 2u; // Omit both }}
      continue;
    }

    if (c == '{') {
      // Two possibilities - {{ or a format parameter

      if (i + 1u >= s.size()) {
        // Todo report malformed format string. A single { at the end of format
        // string
      }

      if (s[i + 1u] == '{') {
        // Got {{. Treat it as one char { in output string
        ++size;
        i += 2u; // Omit both {{
        continue;
      }

      // Should be a format parameter

      const auto begin_pos = i;

      const auto end_pos = s.find('}', begin_pos);
      if (end_pos == std::string_view::npos) {
        // Todo report malformed format parameter. { without }
      }

      const auto subs = s.substr(begin_pos, end_pos - begin_pos + 1u);
      size += max_length_of(subs);
      i = end_pos + 1u;
      continue;
    }

    // Got a regular char.
    ++size;
    ++i;
  }

  return size;
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
  } else if constexpr (current[begin_pos + 1u] == '{') {
    return collect_format_info<CurrentPos + begin_pos + 2, CurrentSize + 3>(
      S{}, params...);
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

    current_original = param_begin_in_original_it;
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

template <typename S, typename... Params>
class formatter
{
public:
  constexpr formatter() = default;

  template <unsigned N, typename Value>
  auto set(const Value& val)
  {
    using param_t = std::decay_t<decltype(std::get<N>(m_params))>;
    const auto param_buf = param_t::to_span(m_buffer);
    const auto [ptr, ec] =
      std::to_chars(param_buf.begin(), param_buf.end(), val);
    return ptr;
  }

  template <unsigned N>
  auto set(std::string_view value)
  {
    using param_t = std::decay_t<decltype(std::get<N>(m_params))>;
    const auto param_buf = param_t::to_span(m_buffer);
    std::copy(value.cbegin(), value.cend(), param_buf.begin());
    return std::next(param_buf.begin(), value.size());
  }

  template <unsigned N, typename Value>
  void set_with_fill(const Value& val, char fill)
  {
    using param_t = std::decay_t<decltype(std::get<N>(m_params))>;
    const auto param_buf = param_t::to_span(m_buffer);
    const auto [ptr, ec] =
      std::to_chars(param_buf.begin(), param_buf.end(), val);
    std::fill(ptr, param_buf.end(), fill);
  }

  template <unsigned N, typename Value>
  void set_with_fill_hint(const Value& val, char fill, char* max_fill_hint)
  {
    using param_t = std::decay_t<decltype(std::get<N>(m_params))>;
    const auto param_buf = param_t::to_span(m_buffer);
    const auto [ptr, ec] =
      std::to_chars(param_buf.begin(), param_buf.end(), val);

    if (ptr < max_fill_hint) {
      std::fill(ptr, max_fill_hint, fill);
    }
  }

  decltype(auto) operator[](unsigned n) { return m_buffer[n]; }

  constexpr auto to_string_view() const
  {
    return std::string_view{ m_buffer.data(), m_buffer.size() };
  }

private:
  std::tuple<Params...> m_params{};
  decltype(make_buffer(S{})) m_buffer = make_buffer(S{});
};

template <typename S, std::string_view::size_type FullLength,
          typename... Params>
constexpr auto make_formatter_impl(
  format_info<S, FullLength, types<Params...>>)
{
  return formatter<S, Params...>{};
}
}

template <typename S>
constexpr auto make_formatter(S)
{
  constexpr auto info = details::collect_format_info<0u, 0u>(S{});
  return details::make_formatter_impl(info);
}
}
