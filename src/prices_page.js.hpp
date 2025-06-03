#pragma once

#include <string>

// clang-format off
constexpr auto
get_prices_script() -> std::string
{
  return std::string {
    #embed "prices_page.js"
  };
}
