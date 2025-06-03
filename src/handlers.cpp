#include <algorithm>
#include <chrono>
#include <format>
#include <future>
#include <iostream>
#include <print>
#include <ranges>
#include <string>
#include <thread>
#include <vector>

#include "chart.js.hpp"
#include "prices_page.js.hpp"
#include "search_page.js.hpp"

#include "countries.hpp"
#include "database.hpp"
#include "handlers.hpp"
#include "steam_api.hpp"
#include "types.hpp"

#include <glaze/glaze.hpp>
#include <glaze/json/read.hpp>

#include <httplib.h>

void
handle_search(const httplib::Request& req, httplib::Response& res)
{
  auto game_name = req.get_param_value("q");
  std::string page_str = req.get_param_value("page");
  int32_t page = page_str.empty() ? 1 : std::stoi(page_str);
  page = std::max(page, 1);

  const int32_t items_per_page { 9 };

  httplib::Client client("https://api.steampowered.com");
  client.set_default_headers(
    { { "Accept", "application/json" },
      { "Accept-Charset", "utf-8" },
      { "User-Agent", "SteamPriceComparison/1.0" } });

  auto result = client.Get("/ISteamApps/GetAppList/v2");
  if (!result || result->status != 200)
  {
    res.set_content("Search failed", "text/plain");
    return;
  }

  steam_app_list all_apps;
  auto ec = glz::read<glz::opts { .error_on_unknown_keys = false }>(
    all_apps, result->body);
  if (ec)
  {
    res.set_content("Failed to parse app list", "text/plain");
    return;
  }

  std::vector<app_item> filtered_apps;
  std::string search_lower = game_name;
  std::ranges::transform(search_lower, search_lower.begin(), ::tolower);

  for (const auto& app : all_apps.applist.apps)
  {
    std::string app_name_lower = app.name;
    std::ranges::transform(app_name_lower, app_name_lower.begin(), ::tolower);

    if (app_name_lower.find(search_lower) != std::string::npos)
    {
      filtered_apps.push_back(app);
    }
  }

  size_t total_results = filtered_apps.size();
  size_t total_pages = (total_results + items_per_page - 1) / items_per_page;
  size_t start_index = (page - 1) * items_per_page;
  size_t end_index = std::min(start_index + items_per_page, total_results);

  std::string html = R"(
    <html>
    <head>
      <meta charset="UTF-8">
      <title>Search Results - Steam Price Comparison</title>
      <link rel="stylesheet" href="/styles.css">
    </head>
    <body>
      <div class="container">
        <h1>Search Results</h1>
        <a href="/" class="back-link">&larr; Back to Search</a>
        <div class="results-info">
          <p>Showing )" +
    std::to_string(end_index - start_index) + R"( of )" +
    std::to_string(total_results) + R"( results (Page )" +
    std::to_string(page) + R"( of )" + std::to_string(total_pages) + R"()</p>
          <p class="search-tip">Tip: Use more specific search terms to find the game you're looking for</p>
        </div>
        <div class="results-grid">
  )";
  for (size_t i = start_index; i < end_index; ++i)
  {
    const auto& item = filtered_apps[ i ];
    html += R"(<div class="game-card">)";
    html += "<h3>" + item.name + "</h3>";
    html += "<a href=\"/prices?appid=" + std::to_string(item.appid) +
      R"(" class="price-button">Compare Prices</a>)";
    html += "</div>";
  }

  html += R"(
        </div>
        <div class="pagination">
  )";

  // Add pagination controls
  if (page > 1)
  {
    html += "<a href=\"/search?q=" + game_name +
      "&page=" + std::to_string(page - 1) +
      R"(" class="page-button">&larr; Previous</a>)";
  }
  // Show page numbers (max 5 around current page)
  size_t start_page =
    std::max(static_cast<size_t>(1), static_cast<size_t>(page - 2));
  size_t end_page = std::min(total_pages, static_cast<size_t>(page + 2));

  for (size_t p = start_page; p <= end_page; ++p)
  {
    if (static_cast<int>(p) == page)
    {
      html +=
        "<span class=\"page-button current\">" + std::to_string(p) + "</span>";
    }
    else
    {
      html += "<a href=\"/search?q=" + game_name +
        "&page=" + std::to_string(p) + R"(" class="page-button">)" +
        std::to_string(p) + "</a>";
    }
  }
  if (static_cast<size_t>(page) < total_pages)
  {
    html += "<a href=\"/search?q=" + game_name +
      "&page=" + std::to_string(page + 1) +
      R"(" class="page-button">Next &rarr;</a>)";
  }
  html += R"(
        </div>
      </div>
        <!-- Loading Overlay -->
      <div id="loadingOverlay" class="loading-overlay">
        <div class="loading-container">
          <div class="loading-spinner"></div>
          <div class="loading-text">Comparing Prices</div>
          <div class="loading-subtext">Fetching prices from Steam stores worldwide...</div>        </div>      </div>      <script>
)" + get_search_script() +
    R"(
      </script>
    </body>
    </html>
  )";
  res.set_content(html, "text/html; charset=utf-8");
}

void
handle_prices(const httplib::Request& req, httplib::Response& res)
{
  int appid = std::stoi(req.get_param_value("appid"));
  std::println("{}", appid);

  std::vector<price_data> results;
  results.reserve(countries.size());

  const size_t batch_size { 4 };
  const int32_t delay_ms { 150 };
  std::println(
    "Starting price comparison for {} countries (processing 4 at a time with "
    "150ms delays)",
    countries.size());
  for (size_t i = 0; i < countries.size(); i += batch_size)
  {
    size_t end = std::min(i + batch_size, countries.size());
    std::vector<std::future<price_data>> batch_futures;
    batch_futures.reserve(end - i);

    // Process only the current batch of countries
    for (size_t j = i; j < end; ++j)
    {
      batch_futures.emplace_back(
        std::async(
          std::launch::async, fetch_price_for_country, appid, countries[ j ]));
    }

    for (auto& future : batch_futures)
    {
      auto result = future.get();
      if (result.success) { results.push_back(result); }
    }

    if (end < countries.size())
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    }
  }

  std::string html = "<html><head>";
  html +=
    R"(
    <script src="https://code.jquery.com/jquery-3.7.1.min.js" integrity="sha256-/JqT3SQfawRcv/BIHPThkBvs0OEvtFFmqPF/lYI/Cxo=" crossorigin="anonymous"></script>
    <link rel="stylesheet" href="https://cdn.datatables.net/2.3.1/css/dataTables.dataTables.css" />
    <script src="https://cdn.datatables.net/2.3.1/js/dataTables.js"></script>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>)";
  html += "<meta charset=\"UTF-8\"><title>Price "
          "Comparison - Steam Price Comparison</title>";
  html += R"(<link rel="stylesheet" href="/styles.css"></head><body>)";
  html += R"(
    <div id="pageLoadingOverlay" class="loading-overlay active">
      <div class="loading-container">
        <div class="loading-spinner"></div>
        <div class="loading-text">Processing Results</div>
        <div class="loading-subtext">Generating price comparison table and charts...</div>
      </div>
    </div>
  )";

  html += "<div class=\"container\"><h1>Regional Price Comparison</h1>";
  html += "<h2>App ID: " + std::to_string(appid) + "</h2>";
  html += "<a href=\"javascript:history.back()\" class=\"back-link\">&larr; "
          "Back to Results</a>";

  html += R"(
    <div class="chart-section" style="margin: 20px 0;">
      <h3>Affordability Comparison (Copies per Year)</h3>
      <p style="font-size: 14px; color: #666;">Shows how many copies an average person in each country could buy with their annual GDP per capita</p>
      <div style="max-width: 100%; height: 400px; margin: 20px 0;">
        <canvas id="affordabilityChart"></canvas>
      </div>
    </div>
  )";
  html +=
    R"(<div class="price-table-container"><table id="result-table" class="price-table">)";
  html += "<thead><tr><th>Country</th><th>Currency</th><th>Price</th><th>USD "
          "Price</th><th>GDP per Capita</th><th>Affordability Ratio</th></tr></"
          "thead><tbody>";
  for (const auto& result : results)
  {
    html += "<tr><td>" + result.country_code + "</td>";

    if (result.is_not_available)
    {
      html += "<td>-</td><td class=\"not-available\">Not "
              "Available</td><td>-</td>";
      if (result.gdp_per_capita > 0.0)
      {
        html +=
          "<td>$" + std::format("{:.0f}", result.gdp_per_capita) + "</td>";
      }
      else { html += "<td>-</td>"; }
      html += "<td>-</td>";
    }
    else if (result.is_free)
    {
      html += "<td>-</td><td class=\"free-price\">Free</td><td>-</td>";
      if (result.gdp_per_capita > 0.0)
      {
        html +=
          "<td>$" + std::format("{:.0f}", result.gdp_per_capita) + "</td>";
      }
      else { html += "<td>-</td>"; }
      html += "<td>∞</td>";
    }
    else
    {
      html += "<td>" + result.currency + "</td>";
      html += "<td>" + std::format("{:.2f}", result.price) + "</td>";
      html += "<td>" + std::format("{:.2f}", result.usd_price) + "</td>";

      if (result.gdp_per_capita > 0.0)
      {
        html += "<td>" + std::format("{:.0f}", result.gdp_per_capita) + "</td>";
      }
      else { html += "<td>-</td>"; }

      if (result.affordability_ratio > 0.0)
      {
        html +=
          "<td>" + std::format("{:.1f}", result.affordability_ratio) + "</td>";
      }
      else { html += "<td>-</td>"; }
    }

    html += "</tr>";
  }
  html += "</tbody></table></div></div>";
  html += "<script>";
  html += "const ctx = document.getElementById('affordabilityChart');";
  html += "const chartLabels = [";

  bool first = true;
  for (const auto& result : results)
  {
    if (
      !result.is_not_available && !result.is_free &&
      result.affordability_ratio > 0.0)
    {
      if (!first) { html += ", "; }
      html += "'" + result.country_code + "'";
      first = false;
    }
  }
  html += "];";

  html += "const countryNames = [";
  first = true;
  for (const auto& result : results)
  {
    if (
      !result.is_not_available && !result.is_free &&
      result.affordability_ratio > 0.0)
    {
      if (!first) { html += ", "; }
      std::string country_name = get_country_name(result.country_code);
      html += "'" + country_name + "'";
      first = false;
    }
  }
  html += "];";

  html += "const chartData = [";
  first = true;
  for (const auto& result : results)
  {
    if (
      !result.is_not_available && !result.is_free &&
      result.affordability_ratio > 0.0)
    {
      if (!first) { html += ", "; }
      html += std::format("{:.1f}", result.affordability_ratio);
      first = false;
    }
  }
  html += "];";
  html += get_chart_script();
  html += "createAffordabilityChart(chartLabels, chartData, countryNames);";
  html += "</script>";

  html += "<script>";
  html += get_prices_script();
  html += "</script>";
  html += "</body></html>";
  res.set_content(html, "text/html; charset=utf-8");
}
