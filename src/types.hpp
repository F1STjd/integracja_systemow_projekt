#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

struct app_item
{
  int32_t appid {};
  std::string name;
};

struct app_list
{
  std::vector<app_item> apps;
};

struct steam_app_list
{
  app_list applist;
};

struct price_data
{
  std::string country_code;
  std::string currency;
  double price;
  double usd_price;
  double gdp_per_capita;
  double affordability_ratio; // GDP per capita / USD price
  bool is_free;
  bool is_not_available;
  bool success;
};

struct exchange_rates
{
  std::string result;
  std::string provider;
  std::string documentation;
  std::string terms_of_use;
  int64_t time_last_update_unix;
  std::string time_last_update_utc;
  int64_t time_next_update_unix;
  std::string time_next_update_utc;
  int64_t time_eol_unix;
  std::string base_code;
  std::map<std::string, double> rates;
};