#pragma once

#include <string>
#include <string_view>

auto
get_gdp_per_capita(std::string_view country_code) -> double;
auto
get_country_name(std::string_view country_code) -> std::string;
