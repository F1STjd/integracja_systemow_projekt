#pragma once

#include <httplib.h>

void
handle_search(const httplib::Request& req, httplib::Response& res);
void
handle_prices(const httplib::Request& req, httplib::Response& res);
