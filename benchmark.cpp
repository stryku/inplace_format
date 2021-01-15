#include <benchmark/benchmark.h>

#include <charconv>

constexpr auto value = 1234567890u;

static void Inplace(benchmark::State& state)
{
  const auto place = 10u;
  std::string s = R"#({"foo ": [                    ]})#";
  auto ptr = s.data() + place;
  for (auto _ : state) {
    std::to_chars(ptr, ptr + 20, value);
    benchmark::DoNotOptimize(s);
  }
}
BENCHMARK(Inplace);

static void StringAppending(benchmark::State& state)
{
  char buf[128];
  for (auto _ : state) {
    const auto [end, ec] = std::to_chars(buf, buf + 20, value);
    std::string s;
    s.reserve(128);
    s.append(R"#({"foo ":)#");
    s.append(buf, end);
    s.append("]}");
    benchmark::DoNotOptimize(s);
  }
}
BENCHMARK(StringAppending);

BENCHMARK_MAIN();