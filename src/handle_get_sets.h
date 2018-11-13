#pragma once

#include <cpprest/json.h>
#include <cpprest/http_listener.h>
#include <BSlogger.hpp>
#include <term.h>
#include <utility.h>
#include <datastructures_go.h>
#include <datastructures_flat.h>
#include <datastructures_hierarchical.h>

void handle_get_sets(enrichment& e, annotation& a, std::string& t,
                     web::json::value& ret, std::string uri,
                     const web::json::value json,
                     std::unordered_set<std::string>& test_set,
                     std::string& test_term);