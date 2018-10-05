#pragma once

#include <vector>
#include <string>
#include <cpprest/json.h>

#define GO_BP 0
#define GO_MF 1
#define GO_CC 2

class term {
public:
  void id(std::string& x) {_id = x;}
  std::string id() {return _id;}
  void def(std::string& x) {_def = x;}
  std::string def() {return _def;}
  void name(std::string& x) {_name = x;}
  std::string name() {return _name;}
  std::string to_string();
  web::json::value to_json(bool name = true, bool def = true);
protected:
  std::string _id;
  std::string _def;
  std::string _name;
};

// GO term
class term_go : public term {
public:    
  void obsolete(bool x) {_obsolete = x;}
  bool obsolete() {return _obsolete;}
  void nspace(uint8_t x) {_namespace = x;}
  uint8_t nspace() {return _namespace;}
  void parents(std::string& x) {_parents.push_back(x);}
  std::vector<std::string>& parents() {return _parents;}
  std::string to_string();
  web::json::value to_json(bool name = true, bool def = true);
protected:
  bool _obsolete;
  uint8_t _namespace;
  std::vector<std::string> _parents;
};