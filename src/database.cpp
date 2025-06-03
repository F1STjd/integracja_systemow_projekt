#include <iostream>
#include <print>
#include <string>
#include <string_view>

#include "database.hpp"

#include <sqlite3.h>

auto
get_gdp_per_capita(std::string_view country_code) -> double
{
  sqlite3* db { nullptr };
  int rc = sqlite3_open(SOURCE_DIR "data.sqlite", &db);
  if (rc != SQLITE_OK)
  {
    std::println(std::cerr, "Cannot open database: {}", sqlite3_errmsg(db));
    sqlite3_close(db);
    return 0.0;
  }
  const char* sql {
    "SELECT (CAST(gdp AS REAL) * 1000000) / CAST(population AS REAL) AS gdp_pc "
    "FROM countries WHERE iso2 = ?"
  };

  sqlite3_stmt* stmt { nullptr };
  rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
  if (rc != SQLITE_OK)
  {
    std::println(
      std::cerr, "Failed to prepare statement: {}", sqlite3_errmsg(db));
    sqlite3_close(db);
    return 0.0;
  }
  sqlite3_bind_text(
    stmt, 1, std::string { country_code }.c_str(), -1, SQLITE_STATIC);

  double gdp_per_capita = 0.0;
  if (sqlite3_step(stmt) == SQLITE_ROW)
  {
    gdp_per_capita = sqlite3_column_double(stmt, 0);
  }
  else
  {
    std::println(std::cerr, "No data found for country: {}", country_code);
  }

  sqlite3_finalize(stmt);
  sqlite3_close(db);
  return gdp_per_capita;
}

auto
get_country_name(std::string_view country_code) -> std::string
{
  sqlite3* db { nullptr };
  int rc = sqlite3_open(SOURCE_DIR "data.sqlite", &db);
  if (rc != SQLITE_OK)
  {
    std::println(std::cerr, "Cannot open database: {}", sqlite3_errmsg(db));
    sqlite3_close(db);
    return std::string { country_code };
  }
  const char* sql { "SELECT country FROM countries WHERE iso2 = ?" };

  sqlite3_stmt* stmt = nullptr;
  rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
  if (rc != SQLITE_OK)
  {
    std::println(
      std::cerr, "Failed to prepare statement: {}", sqlite3_errmsg(db));
    sqlite3_close(db);
    return std::string { country_code };
  }
  sqlite3_bind_text(
    stmt, 1, std::string { country_code }.c_str(), -1, SQLITE_STATIC);

  std::string country_name { country_code };
  if (sqlite3_step(stmt) == SQLITE_ROW)
  {
    const char* name =
      reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    if (name != nullptr) { country_name = std::string(name); }
  }

  sqlite3_finalize(stmt);
  sqlite3_close(db);
  return country_name;
}
