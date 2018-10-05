#pragma once

#include <term.h>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <vector>
#include <cpprest/json.h>

class mapping_opt;

class hierarchical_opt {
public:
  void get(web::json::value& opt);
  std::string file_in;
  std::string name;
};

class hierarchical {
public:
  void parse(hierarchical_opt& opts);
  term& get(std::string id);
  std::vector<std::string> parents_of(std::string id);
  bool in(std::string term) {
    return _terms.find(term) != _terms.end();
  }
  bool is_root(std::string s) {
    for (char c : s)
      if (c == ':')
        return false;
    return true;
  }
protected:
  std::unordered_map<std::string, term> _terms;
};


class mapping_hierarchical {
public:
  void parse(mapping_opt& opt, hierarchical& annotation);
  bool in_a(const std::string& id, bool spec = false) {
    if (spec)
      return _a_to_b_spec.find(id) != _a_to_b_spec.end();
    else
      return _a_to_b.find(id) != _a_to_b.end();
  }
  bool in_b(const std::string& id, bool spec = false) {
    if (spec)
      return _b_to_a_spec.find(id) != _b_to_a_spec.end();
    else
      return _b_to_a.find(id) != _b_to_a.end();
  }
  std::vector<std::string>& a_to_b(std::string query, bool spec = false) {
    if (spec)
      return _a_to_b_spec.at(query);
    else
      return _a_to_b.at(query);
  }
  std::vector<std::string>& b_to_a(std::string query, bool spec = false) {
    if (spec)
      return _b_to_a_spec.at(query);
    else
      return _b_to_a.at(query);
  }
  std::unordered_set<std::string> all_a(std::unordered_set<std::string>& b,
                                        bool spec = false);
  std::unordered_set<std::string> all_b(std::unordered_set<std::string>& a,
                                        bool spec = false);
  mapping_hierarchical make_background(std::string& file_in);
  mapping_hierarchical make_background(std::unordered_set<std::string>& gene_set);
  void add(std::unordered_map<std::string, std::vector<std::string>>::iterator it,
           bool spec = false);
  void dedup();
protected:
  std::unordered_map<std::string, std::vector<std::string>> _a_to_b;
  std::unordered_map<std::string, std::vector<std::string>> _b_to_a;
  std::unordered_map<std::string, std::vector<std::string>> _a_to_b_spec;
  std::unordered_map<std::string, std::vector<std::string>> _b_to_a_spec;
};

