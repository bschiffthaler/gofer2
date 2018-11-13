#pragma once

#include <unordered_set>
#include <string>
#include <datastructures_go.h>
#include <datastructures_flat.h>
#include <datastructures_hierarchical.h>
#include <term.h>
#include <cpprest/json.h>
#include <vector>

class set_membership_go {
public:
  void get_sets(std::unordered_set<std::string>& test_set,
                mapping_go& map,
                const std::string& id,
                go& annotation);
  web::json::value to_json();
  std::vector<std::string> mt;
  std::vector<std::string> nt;
  std::vector<std::string> mpat;
  std::vector<std::string> npat;
};

class set_membership_hierarchical {
public:
  void get_sets(std::unordered_set<std::string>& test_set,
                mapping_hierarchical& map,
                const std::string& id,
                hierarchical& annotation);
  web::json::value to_json();
  std::vector<std::string> mt;
  std::vector<std::string> nt;
  std::vector<std::string> mpat;
  std::vector<std::string> npat;
};

class set_membership_flat {
public:
  void get_sets(std::unordered_set<std::string>& test_set,
                mapping_flat& map,
                const std::string& id);
  web::json::value to_json();
  std::vector<std::string> mt;
  std::vector<std::string> nt;
  std::vector<std::string> m;
  std::vector<std::string> n;
};