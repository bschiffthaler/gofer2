#include <cpprest/json.h>
#include <cpprest/http_listener.h>
#include <BSlogger.hpp>
#include <term.h>
#include <utility.h>
#include <datastructures_go.h>
#include <datastructures_flat.h>
#include <datastructures_hierarchical.h>
#include <set_memberships.h>
#include <handle_get_sets.h>

void handle_get_sets(enrichment& e, annotation& a, std::string& t,
                     web::json::value& ret, std::string uri,
                     const web::json::value json,
                     std::unordered_set<std::string>& test_set,
                     std::string& test_term)
{
  logger log(std::cerr, uri);
  log(LOG_INFO) << "Got get_sets task " << t << '\n';
  ret[test_term] = web::json::value();
  if (e.types.find(t) == e.types.end())
  {
    append_error(ret[test_term], "Error: Mapping type not supported by server.");
    return;
  }

  std::string type = e.types[t];

  if (type == "go")
  {
    mapping_opt& mopt = e.mapping_opt_gos[t];

    std::shared_ptr<mapping_go> map;
    mapping_go custom;
    if (json.has_field("background"))
    {
      if (json.at("background").is_string())
      {
        std::string key = t + "_" + json.at("background").as_string();
        if (e.background_gos.find(key) == e.background_gos.end())
        {
          append_error(ret[t], "Background specified, but not found in memory."
                       "Did you mean to provide an array of genes?");
          return;
        }
        map = std::make_shared<mapping_go>(e.background_gos[key]);
      }
      else if (json.at("background").is_array())
      {
        web::json::array arr = json.at("background").as_array();
        std::unordered_set<std::string> gene_set;
        for (web::json::value& gene : arr)
          gene_set.insert(gene.as_string());
        for (const std::string& gene : test_set)
          gene_set.insert(gene);
        // FIX ME: Check sanity of background
        custom = e.mapping_gos[t].make_background(gene_set);
        map = std::make_shared<mapping_go>(custom);
      }
    }
    else
    {
      map = std::make_shared<mapping_go>(e.mapping_gos[t]);
    }

    std::unordered_set<std::string> filter_set;
    for (const std::string& id : test_set)
      if ( map->in_a(id) )
        filter_set.insert(id);

    set_membership_go term_set;
    term_set.get_sets(filter_set, *map, test_term,
                      a.go_by_name(mopt.annotation));

    ret[test_term]["sets"] = term_set.to_json();

  }
  else if (type == "flat")
  {
    mapping_opt& mopt = e.mapping_opt_flats[t];

    std::shared_ptr<mapping_flat> map;
    mapping_flat custom;
    if (json.has_field("background"))
    {
      if (json.at("background").is_string())
      {
        std::string key = t + "_" + json.at("background").as_string();
        if (e.background_flats.find(key) == e.background_flats.end())
        {
          ret[t] = web::json::value::string("Background specified, but not "
                                            "found in memory");
          return;
        }
        map = std::make_shared<mapping_flat>(e.background_flats[key]);
      }
      else if (json.at("background").is_array())
      {
        web::json::array arr = json.at("background").as_array();
        std::unordered_set<std::string> gene_set;
        for (web::json::value& gene : arr)
          gene_set.insert(gene.as_string());
        for (const std::string& gene : test_set)
          gene_set.insert(gene);
        // FIX ME: Check sanity of background
        custom = e.mapping_flats[t].make_background(gene_set);
        map = std::make_shared<mapping_flat>(custom);
      }
    }
    else
    {
      map = std::make_shared<mapping_flat>(e.mapping_flats[t]);
    }

    std::unordered_set<std::string> filter_set;
    for (const std::string& id : test_set)
      if ( map->in_a(id) )
        filter_set.insert(id);

    set_membership_flat term_set;
    term_set.get_sets(filter_set, *map, test_term);
    ret[test_term]["sets"] = term_set.to_json();

  }
  else if (type == "hierarchical")
  {
    mapping_opt& mopt = e.mapping_opt_hierarchicals[t];

    std::shared_ptr<mapping_hierarchical> map;
    mapping_hierarchical custom;
    if (json.has_field("background"))
    {
      if (json.at("background").is_string())
      {
        std::string key = t + "_" + json.at("background").as_string();
        if (e.background_hierarchicals.find(key) == e.background_hierarchicals.end())
        {
          ret[t] = web::json::value::string("Background specified, but not "
                                            "found in memory");
          return;
        }
        map = std::make_shared<mapping_hierarchical>(e.background_hierarchicals[key]);
      }
      else if (json.at("background").is_array())
      {
        web::json::array arr = json.at("background").as_array();
        std::unordered_set<std::string> gene_set;
        for (web::json::value& gene : arr)
          gene_set.insert(gene.as_string());
        for (const std::string& gene : test_set)
          gene_set.insert(gene);
        // FIX ME: Check sanity of background
        custom = e.mapping_hierarchicals[t].make_background(gene_set);
        map = std::make_shared<mapping_hierarchical>(custom);
      }
    }
    else
    {
      map = std::make_shared<mapping_hierarchical>(e.mapping_hierarchicals[t]);
    }

    std::unordered_set<std::string> filter_set;
    for (const std::string& id : test_set)
      if ( map->in_a(id) )
        filter_set.insert(id);

    set_membership_hierarchical term_set;
    term_set.get_sets(filter_set, *map, test_term,
                      a.hierarchical_by_name(mopt.annotation));
    ret[test_term]["sets"] = term_set.to_json();
  }
}