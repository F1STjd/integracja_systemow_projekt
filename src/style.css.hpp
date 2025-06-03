#pragma once

#include <string>

// clang-format off
constexpr auto
get_style() -> std::string
{
  return std::string {
    #embed "style.css"
  };
}