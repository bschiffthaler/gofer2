#pragma once

#include <term.h>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <cpprest/json.h>

class mapping_opt;

class flat_opt {
public:
  void get(web::json::value& opt);
  std::string file_in;
  std::string name;
};

class flat {
public:
  void parse(flat_opt& opts);
  term& get(std::string id);
  bool in(std::string term) {
    return _terms.find(term) != _terms.end();
  }
  void add(std::string& id, term& t) {_terms[id] = t; }
protected:
  std::unordered_map<std::string, term> _terms;
};

class mapping_flat {
public:
  void parse(mapping_opt& opt, flat& annotation);
  bool in_a(const std::string& id) {return _a_to_b.find(id) != _a_to_b.end();}
  bool in_b(const std::string& id) {return _b_to_a.find(id) != _b_to_a.end();}
  std::unordered_map<std::string, std::vector<std::string>>& a_to_b() {return _a_to_b;}
  std::unordered_map<std::string, std::vector<std::string>>& b_to_a() {return _b_to_a;}
  std::vector<std::string>& a_to_b(std::string query) {return _a_to_b.at(query);}
  std::vector<std::string>& b_to_a(std::string query) {return _b_to_a.at(query);}
  std::unordered_set<std::string> all_a(std::unordered_set<std::string>& b);
  std::unordered_set<std::string> all_b(std::unordered_set<std::string>& a);
  mapping_flat make_background(std::string& file_in);
  mapping_flat make_background(std::unordered_set<std::string>& gene_set);
  void add(std::unordered_map<std::string, std::vector<std::string>>::iterator it);
  void dedup();
protected:
  std::unordered_map<std::string, std::vector<std::string>> _a_to_b;
  std::unordered_map<std::string, std::vector<std::string>> _b_to_a;
};

