#pragma once

#include "types.hpp"
#include <string_view>

auto
fetch_price_for_country(int32_t appid, std::string_view country_code)
  -> price_data;
