#include <cpprest/json.h>
#include <cpprest/http_listener.h>
#include <BSlogger.hpp>
#include <term.h>
#include <utility.h>
#include <datastructures_go.h>
#include <datastructures_flat.h>
#include <datastructures_hierarchical.h>
#include <handle_translation.h>
#include <unordered_set>

void handle_gene_to_term(enrichment& e, annotation& a, std::string& t,
                         web::json::value& ret, std::string uri,
                         const web::json::value json,
                         std::unordered_set<std::string>& test_set)
{
  logger log(std::cerr, uri);
#ifdef DEBUG  
  log(LOG_DEBUG) << "Got translation task " << t << '\n';
#endif  
  bool names = true;
  bool defs = true;
  if (e.types.find(t) == e.types.end())
  {
    ret[t] = web::json::value::string("Error: Translation type not supported"
                                      " by server.");
    return;
  }
  if (json.has_field("include_names"))
  {
    if (! json.at("include_names").is_boolean())
    {
      ret[t] = web::json::value::string("Error: 'include_names' found in JSON, "
                                        "but value was not boolean");
      return;
    }
    names = json.at("include_names").as_bool();
  }
  if (json.has_field("include_defs"))
  {
    if (! json.at("include_defs").is_boolean())
    {
      ret[t] = web::json::value::string("Error: 'include_defs' found in JSON, "
                                        "but value was not boolean");
      return;
    }
    defs = json.at("include_defs").as_bool();
  }

  std::string type = e.types.at(t);

  if (type == "go")
  {
    ret[t] = web::json::value::array();
    mapping_opt& mopt = e.mapping_opt_gos[t];
    size_t index = 0;
    for (const std::string& id : test_set)
    {
      ret[t][index] = web::json::value();
      ret[t][index]["id"] = web::json::value::string(id);
      ret[t][index]["terms"] = web::json::value::array();
      size_t tindex = 0;
      if (e.mapping_gos[t].in_a(id, true)) // Look only in specific matches
      {
        for (const std::string b : e.mapping_gos[t].a_to_b(id, true))
        {
          term_go& term = a.go_by_name(mopt.annotation).get(b);
          ret[t][index]["terms"][tindex] = term.to_json(names, defs);
          tindex++;
        }
      }
      index++;
    }
  }
  else if (type == "flat")
  {
    ret[t] = web::json::value::array();
    mapping_opt& mopt = e.mapping_opt_flats[t];
    size_t index = 0;
    for (const std::string& id : test_set)
    {
      ret[t][index] = web::json::value();
      ret[t][index]["id"] = web::json::value::string(id);
      ret[t][index]["terms"] = web::json::value::array();
      size_t tindex = 0;
      if (e.mapping_flats[t].in_a(id))
      {
        for (const std::string b : e.mapping_flats[t].a_to_b(id))
        {
          term& term = a.flat_by_name(mopt.annotation).get(b);
          ret[t][index]["terms"][tindex] = term.to_json(names, defs);
          tindex++;
        }
      }
      index++;
    }
  }
  else if (type == "hierarchical")
  {
    ret[t] = web::json::value::array();
    mapping_opt& mopt = e.mapping_opt_hierarchicals[t];
    size_t index = 0;
    for (const std::string& id : test_set)
    {
      ret[t][index] = web::json::value();
      ret[t][index]["id"] = web::json::value::string(id);
      ret[t][index]["terms"] = web::json::value::array();
      size_t tindex = 0;
      if (e.mapping_hierarchicals[t].in_a(id, true)) // Look only in specific matches
      {
        for (const std::string b : e.mapping_hierarchicals[t].a_to_b(id, true))
        {
          term& term = a.go_by_name(mopt.annotation).get(b);
          ret[t][index]["terms"][tindex] = term.to_json(names, defs);
          tindex++;
        }
      }
      index++;
    }
  }
}

void handle_term_to_gene(enrichment& e, annotation& a, web::json::value& t,
                         web::json::value& ret, std::string uri,
                         const web::json::value json)
{
  logger log(std::cerr, uri);
#ifdef DEBUG  
  log(LOG_DEBUG) << "Got translation task " << t << '\n';
#endif  
  std::string name = t["name"].as_string();
  web::json::array terms = t["terms"].as_array();
  std::string type = e.types.at(name);
  std::unordered_set<std::string> term_set;
  for (web::json::value& term : terms)
    term_set.insert(term.as_string());
  if (type == "go")
  {
    ret[name] = web::json::value::array();
    mapping_opt& mopt = e.mapping_opt_gos[name];
    size_t index = 0;
    for (const std::string& term : term_set)
    {
      ret[name][index] = web::json::value();
      ret[name][index]["term"] = web::json::value::string(term);
      ret[name][index]["ids"] = web::json::value::array();
      size_t tindex = 0;
      if (e.mapping_gos[name].in_b(term, true))
      {
        for (const std::string a : e.mapping_gos[name].b_to_a(term, true))
        {
          ret[name][index]["ids"][tindex] = web::json::value::string(a);
          tindex++;
        }
      }
      index++;
    }
  }
  else if (type == "flat")
  {
    ret[name] = web::json::value::array();
    mapping_opt& mopt = e.mapping_opt_flats[name];
    size_t index = 0;
    for (const std::string& term : term_set)
    {
      ret[name][index] = web::json::value();
      ret[name][index]["term"] = web::json::value::string(term);
      ret[name][index]["ids"] = web::json::value::array();
      size_t tindex = 0;
      if (e.mapping_flats[name].in_b(term))
      {
        for (const std::string a : e.mapping_flats[name].b_to_a(term))
        {
          ret[name][index]["ids"][tindex] = web::json::value::string(a);
          tindex++;
        }
      }
      index++;
    }
  }
  else if (type == "hierarchical")
  {
    ret[name] = web::json::value::array();
    mapping_opt& mopt = e.mapping_opt_hierarchicals[name];
    size_t index = 0;
    for (const std::string& term : term_set)
    {
      ret[name][index] = web::json::value();
      ret[name][index]["term"] = web::json::value::string(term);
      ret[name][index]["ids"] = web::json::value::array();
      size_t tindex = 0;
      if (e.mapping_hierarchicals[name].in_b(term, true))
      {
        for (const std::string a : e.mapping_hierarchicals[name].b_to_a(term, true))
        {
          ret[name][index]["ids"][tindex] = web::json::value::string(a);
          tindex++;
        }
      }
      index++;
    }
  }
}