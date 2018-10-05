#pragma once

#include <boost/tokenizer.hpp>
#include <string>
#include <cpprest/json.h>
#include <unordered_map>
#include <datastructures_go.h>
#include <datastructures_flat.h>
#include <datastructures_hierarchical.h>

enum test_t {TEST_GO, TEST_FISHER, TEST_HIERARCHICAL};

/*
class go;
class flat;
class hierarchical;
class mapping_go;
class mapping_flat;
class mapping_hierarchical;
class go_opt;
class flat_opt;
class hierarchical_opt;
*/

typedef boost::tokenizer<boost::char_separator<char> > tokenizer;

void system_error(std::string message, int code);

class mapping_opt {
public:
  void get(web::json::value& opt);
  std::string file_in;
  std::string name;
  std::string annotation;
  test_t test;
};

class annotation {
public:
  void from_json(web::json::value& json);
  go& go_by_name(std::string n){return gos.at(n);}
  flat& flat_by_name(std::string n){return flats.at(n);}
  hierarchical& hierarchical_by_name(std::string n){return hierarchicals.at(n);}
  std::string get_type(std::string t){return types.at(t);}

  std::unordered_map<std::string, go_opt> go_opts;
  std::unordered_map<std::string, flat_opt> flat_opts;
  std::unordered_map<std::string, hierarchical_opt> hierarchical_opts;
  std::unordered_map<std::string, go> gos;
  std::unordered_map<std::string, flat> flats;
  std::unordered_map<std::string, hierarchical> hierarchicals;
  std::unordered_map<std::string, std::string> types;
};

class enrichment {
public:
  void from_json(web::json::value& json, annotation& ann);
  std::unordered_map<std::string, mapping_opt> mapping_opt_gos;
  std::unordered_map<std::string, mapping_opt> mapping_opt_flats;
  std::unordered_map<std::string, mapping_opt> mapping_opt_hierarchicals;
  std::unordered_map<std::string, mapping_go> mapping_gos;
  std::unordered_map<std::string, mapping_flat> mapping_flats;
  std::unordered_map<std::string, mapping_hierarchical> mapping_hierarchicals;
  std::unordered_map<std::string, mapping_go> background_gos;
  std::unordered_map<std::string, mapping_flat> background_flats;
  std::unordered_map<std::string, mapping_hierarchical> background_hierarchicals;
  std::unordered_map<std::string, std::string> types;
};

void remove_ws_ip(std::string& s);
std::string remove_ws(std::string& s);
std::string clean(std::string& s);
std::pair<std::string, std::string> 
split_first_of(std::string str, std::string pattern);
web::json::value parse_config(const std::string& file_in);
std::unordered_set<std::string> read_genelist(std::string& file_in);