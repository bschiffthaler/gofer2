#pragma once
#include <cpprest/json.h>
#include <cpprest/http_listener.h>
#include <BSlogger.hpp>
#include <term.h>
#include <utility.h>
#include <datastructures_go.h>
#include <datastructures_flat.h>
#include <datastructures_hierarchical.h>
#include <unordered_set>

void handle_gene_to_term(enrichment& e, annotation& a, std::string& t,
                         web::json::value& ret, std::string uri,
                         const web::json::value json,
                         std::unordered_set<std::string>& test_set);
void handle_term_to_gene(enrichment& e, annotation& a, web::json::value& t,
                         web::json::value& ret, std::string uri,
                         const web::json::value json);