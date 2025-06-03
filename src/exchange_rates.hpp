#pragma once

#include <expected>
#include <filesystem>
#include <map>
#include <string>
#include <string_view>

auto
fetch_exchange_rates(const std::filesystem::path& path)
  -> std::expected<std::map<std::string, double>, std::string>;

auto
fetch_exchange_rates()
  -> std::expected<std::map<std::string, double>, std::string>;

auto
convert_to_usd(double price, std::string_view currency) -> double;
