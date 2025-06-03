#pragma once

#include <string>

// clang-format off
constexpr auto
get_search_script() -> std::string
{
  return std::string {
    #embed "search_page.js"
  };
}
