#include <term.h>
#include <string>
#include <cpprest/json.h>

std::string term::to_string() {
  std::string r("id: ");
  r += _id;
  r += "\ndef: ";
  r += _def;
  r += "\nname: ";
  r += _name;
  r += "\n";
  return r;
}

web::json::value term::to_json(bool name, bool def)
{
  web::json::value ret;
  ret["id"] = web::json::value::string(_id);
  if (name)
    ret["name"] = web::json::value::string(_name);
  if (def)
    ret["def"] = web::json::value::string(_def);
  return ret;
}

std::string term_go::to_string() {
  std::string r("id: ");
  r += _id;
  r += "\ndef: ";
  r += _def;
  r += "\nname: ";
  r += _name;
  if (_obsolete)
    r += "\nis_obsolete: true";
  if (_parents.size()) {
    for (std::string p : _parents)
    {
      r += "\nis_a: ";
      r += p;
    }
  }
  return r;
}

web::json::value term_go::to_json(bool name, bool def)
{
  web::json::value ret;
  ret["id"] = web::json::value::string(_id);
  switch (_namespace)
  {
  case GO_BP:
    ret["namespace"] = web::json::value::string("BP");
    break;
  case GO_MF:
    ret["namespace"] = web::json::value::string("MF");
    break;
  case GO_CC:
    ret["namespace"] = web::json::value::string("CC");
    break;
  default:
    break; //TODO: throw error  
  }
  if (name)
    ret["name"] = web::json::value::string(_name);
  if (def)
    ret["def"] = web::json::value::string(_def);
  return ret;
}