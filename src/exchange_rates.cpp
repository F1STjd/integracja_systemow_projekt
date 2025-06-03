
#include <chrono>
#include <fstream>
#include <iostream>
#include <iterator>
#include <print>

#include "exchange_rates.hpp"
#include "types.hpp"

#include <glaze/json/read.hpp>
#include <glaze/json/write.hpp>

#include <httplib.h>

static auto g_last_fetch_time { std::chrono::system_clock::now() };
static const std::filesystem::path //
  exchange_rate_file { SOURCE_DIR "usd_conversion.json" };

auto
fetch_exchange_rates(const std::filesystem::path& path)
  -> std::expected<std::map<std::string, double>, std::string>
{
  std::ifstream input_file { path };

  auto json = //
    std::string { std::istreambuf_iterator<char>(input_file),
                  std::istreambuf_iterator<char>() };
  auto s = glz::read_json<exchange_rates>(json);
  if (s.has_value()) { return s.value().rates; }

  return std::unexpected { "" };
}

auto
fetch_exchange_rates()
  -> std::expected<std::map<std::string, double>, std::string>
{
  httplib::Client cli("https://open.er-api.com");
  cli.set_default_headers(
    { { "Accept", "application/json" },
      { "User-Agent", "SteamPriceComparison/1.0" } });

  if (
    std::chrono::system_clock::now() - g_last_fetch_time >
    std::chrono::hours(24))
  {
    return fetch_exchange_rates(exchange_rate_file);
  }

  auto r = cli.Get("/v6/latest/USD");
  if (!r || r->status != 200)
  {
    std::println(std::cerr, "Failed to fetch exchange rates: ");
    if (r) { std::println(std::cerr, "HTTP {}", r->status); }
    else { std::println(std::cerr, "Network error"); }
    return std::unexpected { "" };
  }

  auto s = glz::read_json<exchange_rates>(r->body);
  if (s.has_value())
  {
    auto buffer = glz::write_json(s).value_or("error");
    std::ofstream output_file { exchange_rate_file };
    auto ec =
      glz::write_file_json(s, exchange_rate_file.string(), std::string {});
    g_last_fetch_time = std::chrono::system_clock::now();
    return s.value().rates;
  }

  return std::unexpected { "" };
}

auto
convert_to_usd(double price, std::string_view currency) -> double
{
  if (currency == "USD" || currency == "-") { return price; }

  auto rates = fetch_exchange_rates();
  if (!rates.has_value())
  {
    std::println(
      std::cerr, "Failed to fetch exchange rates for currency: {}", currency);
    return 0.0;
  }

  auto it = rates.value().find(std::string { currency });
  if (it != rates.value().end() && it->second > 0)
  {
    return price / it->second;
  }

  std::println(std::cerr, "Currency {} not found in exchange rates", currency);
  return 0.0;
}
