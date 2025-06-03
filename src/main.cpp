#include <string>

#include <glaze/json/read.hpp>
#include <glaze/json/write.hpp>

// #define CPPHTTPLIB_OPENSSL_SUPPORT
#include <glaze/glaze.hpp>
#include <glaze/json/json_t.hpp>

#include <httplib.h>

#include <sqlite3.h>

#include "handlers.hpp"
#include "style.css.hpp"

auto
main() -> int32_t
{
  get_style();
  httplib::Server svr;
  svr.Get(
    "/",
    [](const httplib::Request&, httplib::Response& res)
    {
      std::string html = R"(
          <html>
          <head>
            <meta charset="UTF-8">
            <title>Steam Price Comparison</title>
            <link rel="stylesheet" href="/styles.css">
          </head>
          <body>
            <div class="container">
              <h1>Steam Price Comparison</h1>
              <p>Search for games and compare prices across different
  regions</p> <form action="/search" method="get" class="search-form"> <input
  name="q" placeholder="Enter game name..." class="search-input" /> <input
  type="submit" value="Search" class="search-button">
              </form>
            </div>
          </body></html>
        )";
      res.set_content(html, "text/html; charset=utf-8");
    });
  svr.Get("/search", handle_search);
  svr.Get("/prices", handle_prices);

  svr.Get(
    "/styles.css",
    [](const httplib::Request&, httplib::Response& res)
    {
      const auto css = get_style();
      res.set_content(css, "text/css");
    });

  svr.listen("0.0.0.0", 8080);
}
