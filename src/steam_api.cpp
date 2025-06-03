#include <iostream>
#include <print>

#include "database.hpp"
#include "exchange_rates.hpp"
#include "steam_api.hpp"
#include "types.hpp"

#include <glaze/json/json_t.hpp>
#include <glaze/json/read.hpp>

#include <httplib.h>

auto
fetch_price_for_country(int32_t appid, std::string_view country_code)
  -> price_data
{
  price_data result {
    .country_code { country_code },
    .currency {},
    .price {},
    .usd_price {},
    .gdp_per_capita = get_gdp_per_capita(country_code),
    .affordability_ratio {},
    .is_free = false,
    .is_not_available = false,
    .success = false,
  };
  httplib::Client cli("https://store.steampowered.com");
  cli.set_connection_timeout(10, 0); // 10 seconds connection timeout
  cli.set_read_timeout(10, 0);       // 10 seconds read timeout
  cli.set_write_timeout(10, 0);      // 10 seconds write timeout
  cli.set_default_headers(
    { { "Accept", "application/json" },
      { "Accept-Charset", "utf-8" },
      { "User-Agent", "SteamPriceComparison/1.0" } });
  auto path = "/api/appdetails?appids=" + std::to_string(appid) +
    "&cc=" + country_code + "&filters=price_overview";
  auto r = cli.Get(path);

  if (!r)
  {
    std::println("Network error for country {}: Request failed", country_code);
    return result;
  }
  if (r->status != 200)
  {
    if (r->status == 429)
    {
      std::println(
        std::cerr,
        "Rate limit exceeded for country {} (HTTP 429). Steam is throttling "
        "requests.",
        country_code);
    }
    else
    {
      std::println(
        std::cerr, "HTTP error for country {}: Status {}", country_code,
        r->status);
    }
    return result;
  }
  try
  {
    auto json = glz::read_json<glz::json_t>(r->body);
    if (!json.has_value())
    {
      std::println(
        std::cerr, "Failed to parse JSON for country {}", country_code);
      return result;
    }

    auto app_id_str = std::to_string(appid);
    auto& root = json.value()[ app_id_str ];
    if (root.is_null())
    {
      std::println(
        std::cerr, "No data for app {} in country {}", appid, country_code);
      return result;
    }

    if (!root[ "success" ].get<bool>())
    {
      std::println(
        std::cerr,
        "Steam API returned success=false for country {} (not available in "
        "region)",
        country_code);
      result.is_not_available = true;
      result.success = true;
      result.currency = "-";
      return result;
    }

    auto& data = root[ "data" ];
    if (data.is_array() || data.is_null() || data[ "price_overview" ].is_null())
    {
      result.is_free = true;
      result.success = true;
      result.currency = "-";
    }
    else
    {
      result.currency =
        data[ "price_overview" ][ "currency" ].get<std::string>();
      double fin = data[ "price_overview" ][ "final" ].get<double>();
      result.price = fin / 100.0;
      result.usd_price = convert_to_usd(result.price, result.currency);

      if (result.usd_price > 0.0 && result.gdp_per_capita > 0.0)
      {
        result.affordability_ratio = result.gdp_per_capita / result.usd_price;
      }

      result.success = true;
    }
  }
  catch (const std::exception& e)
  {
    std::println(
      std::cerr, "JSON parsing error for country {}: {}", country_code,
      e.what());
    result.success = false;
  }
  catch (...)
  {
    std::println(
      std::cerr, "Unknown JSON parsing error for country {}", country_code);
    result.success = false;
  }

  return result;
}
