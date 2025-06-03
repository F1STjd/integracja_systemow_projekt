#pragma once

#include <string>

// clang-format off
constexpr auto
get_chart_script() -> std::string
{
  return std::string {
    #embed "chart.js"
  };
}
