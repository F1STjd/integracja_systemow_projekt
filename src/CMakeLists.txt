# CMakeLists.txt for source files

# Add source files to the target using target_sources
target_sources(is_app PRIVATE
  main.cpp
  handlers.cpp
  exchange_rates.cpp
  database.cpp
  steam_api.cpp
)
