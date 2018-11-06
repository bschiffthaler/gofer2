#include <utility.h>
#include <string>
#include <algorithm>
#include <unordered_set>
#include <cpprest/json.h>
#include <boost/filesystem.hpp>
#include <BSlogger.hpp>

namespace fs = boost::filesystem;

void system_error(std::string message, int code = 22)
{
  throw std::system_error(code, std::generic_category(), message);
}

void remove_ws_ip(std::string& s) {
  s.erase(
  std::remove_if(begin(s), end(s), [](unsigned char ch) {
    return std::isspace(ch);
  }), end(s));
}

std::string remove_ws(std::string& s) {
  std::string s2;
  s2.reserve(s.size());
  std::remove_copy_if(
    begin(s), end(s),
    std::back_inserter(s2),
  [](unsigned char ch) {
    return std::isspace(ch);
  });
  return s2;
}

std::unordered_set<std::string> read_genelist(std::string& file_in) {
  logger log(std::cerr, "read_genelist");
  log(LOG_INFO) << "Parsing " << file_in << '\n';
  std::unordered_set<std::string> bg_ids;
  std::ifstream stream_in(file_in, std::ios::in);
  std::string line;
  size_t not_found = 0;
  while (std::getline(stream_in, line))
  {
    remove_ws(line);
    bg_ids.insert(line);
  }
  return bg_ids;
}

std::string clean(std::string& s) {
  std::string ws = " \t\n\r";
  std::string ret;
  std::string tmp;
  auto begin = s.find_first_not_of(ws);
  if (begin == std::string::npos)
    return "";

  auto end = s.find_last_not_of(ws);
  auto diff = end - begin + 1;

  tmp = s.substr(begin, diff);
  auto bspace = tmp.find_first_of(ws);
  while (bspace != std::string::npos)
  {
    auto espace = tmp.find_first_not_of(ws, bspace);
    auto range = espace - bspace;

    tmp.replace(bspace, range, " ");

    auto new_s = bspace + 1;
    bspace = tmp.find_first_of(ws, new_s);
  }

  for (char c : tmp)
  {
    if (c != '"' && c != '\'')
      ret += c;
  }
  return ret;
}

std::pair<std::string, std::string>
split_first_of(std::string str, std::string pattern) {
  auto pos = str.find_first_of(pattern);
  std::string key = str.substr(0, pos);
  std::string value = str.substr(pos + 1, std::string::npos);
  remove_ws_ip(key);
  return std::pair<std::string, std::string>(key, value);
}

web::json::value parse_config(const std::string& file_in) {
  logger log(std::cerr, "parse_config");
  std::ifstream stream_in(file_in, std::ios::in);
  web::json::value ret;
  stream_in >> ret;
  std::vector<web::json::value> enrichments;
  if (! ret.has_field("port"))
    ret["port"] = 5432;

  int p = ret["port"].as_integer();

  if (p < 1 || p > 65535)
    system_error("Port " + std::to_string(p) + " is invalid.");

  if (p < 1000)
    log(LOG_WARN) << "Port " << p << " is lower than 1000. It is recommended "
                  "to choose a higher port\n";


  // Check for 'org'
  if (! ret.has_field("org"))
    system_error("You need to provide at least one 'org' block"
                             " in your config file");

  if (! ret["org"].is_array())
    system_error("The 'org' field in your config file should be a "
                             "JSON array");

  web::json::array arr = ret["org"].as_array();

  for (web::json::value& val : arr)
  {
    if (! val.has_field("name") )
      system_error("You need to provide a 'name' for each organsim");
    if (! val.has_field("uri") )
      system_error("You need to provide a 'uri' for each organsim");
    if (! val.has_field("enrichment") )
      system_error("You need to provide at least one 'enrichment' "
                               "array for each organism");
    if (! val["enrichment"].is_array() )
      system_error("Enrichment " + val["name"].as_string() +
                               " needs to be a JSON array");
    if (val["enrichment"].as_array().size() < 1 )
      system_error("You need to provide at least one enrichment");
    web::json::array enrs = val["enrichment"].as_array();
    for (web::json::value& enr : enrs)
    {
      if (! enr.has_field("name"))
        system_error("You need to provide a 'name' for each "
                                 "enrichment");
      if (! enr.has_field("test"))
        system_error("You need to provide a 'test' for each "
                                 "enrichment");
      if (! enr.has_field("annotation"))
        system_error("You need to provide a 'annotation' for each "
                                 "enrichment");
      if (! enr.has_field("mapping"))
        system_error("You need to provide a 'mapping' for each "
                                 "enrichment");
      std::string test = enr["test"].as_string();
      std::string mapping = enr["mapping"].as_string();
      if (! fs::exists(fs::path(mapping)))
        system_error("Mapping " + mapping + " does not exist");
      if (test != "go" && test != "hierarchical" && test != "fisher")
        system_error("'test' needs to be one of 'go', "
                                 "'hierarchical' or 'fisher'");
      enrichments.push_back(enr);
    }
  }

  if (! ret.has_field("annotation"))
    system_error("You need to define at least one annotation");

  if (! ret["annotation"].is_array())
    system_error("'annotation' needs to be a JSON array");

  arr = ret["annotation"].as_array();

  std::vector<std::string> annot_names;

  for (web::json::value& val : arr)
  {
    if(! val.has_field("name"))
      system_error("You need to provide a 'name' for each "
                               "annotation");
    if(! val.has_field("sourcefile"))
      system_error("You need to provide a 'sourcefile' for each "
                               "annotation");
    if(! val.has_field("type"))
      system_error("You need to provide a 'type' for each "
                               "annotation");
    std::string type = val["type"].as_string();
    if(type != "flat" && type != "hierarchical" && type != "go")
      system_error("'type' needs to be one of 'flat', 'hierarchical' or 'go'");
    std::string name = val["name"].as_string();
    annot_names.push_back(name);
  }

  for(web::json::value& enr : enrichments)
  {
    std::string source = enr["name"].as_string();
    std::string target = enr["annotation"].as_string();
    auto ptr = std::find(annot_names.begin(), annot_names.end(), target);
    if(ptr == annot_names.end())
    {
      system_error("Target annotation '" + target + "' was not found "
                               "for enrichment '" + source + "'");
    } 
  }

  return ret;
}

void mapping_opt::get(web::json::value& opt) {
  if (! opt.has_field("mapping"))
    system_error("mapping is a required mapping option");
  if (! opt.has_field("test"))
    system_error("test is a required mapping option");
  if (! opt.has_field("name"))
    system_error("name is a required mapping option");
  if (! opt.has_field("annotation"))
    system_error("annotation is a required mapping option");

  annotation = opt["annotation"].as_string();
  name = opt["name"].as_string();

  file_in = opt["mapping"].as_string();
  fs::path p(file_in);
  if (! fs::exists(p))
    system_error("File: " + file_in +
                             " does not exist for mapping " + name);

  file_in = fs::canonical(p).string();
  std::string tp = opt["test"].as_string();

  if (tp == "fisher")
    test = TEST_FISHER;
  else if (tp == "go")
    test = TEST_GO;
  else if (tp == "hierarchical")
    test = TEST_HIERARCHICAL;
  else
    system_error("Unknown test : " + tp + " for mapping " +
                             name);

}

void annotation::from_json(web::json::value& json) {
  if (! json.has_field("annotation"))
    system_error("At least one annotation is required");
  if (json["annotation"].is_array())
  {
    web::json::array arr = json["annotation"].as_array();
    for (web::json::value& v : arr)
    {
      if (! v.has_field("type"))
        system_error("'type' is a required field for annotation");
      std::string t = v["type"].as_string();
      if (t == "go")
      {
        go_opt opt;
        opt.get(v);
        go annot;
        annot.parse(opt);
        go_opts[opt.name] = opt;
        gos[opt.name] = annot;
        types[opt.name] = "go";
      }
      else if (t == "flat")
      {
        flat_opt opt;
        opt.get(v);
        flat annot;
        annot.parse(opt);
        flat_opts[opt.name] = opt;
        flats[opt.name] = annot;
        types[opt.name] = "flat";
      }
      else if (t == "hierarchical")
      {
        hierarchical_opt opt;
        opt.get(v);
        hierarchical annot;
        annot.parse(opt);
        hierarchical_opts[opt.name] = opt;
        hierarchicals[opt.name] = annot;
        types[opt.name] = "hierarchical";
      }
      else
      {
        system_error("Annotation type " + t + " is unknown.");
      }
    }
  }
  else
  {
    web::json::value v = json["annotation"];
    if (! v.has_field("type"))
      system_error("'type' is a required field for annotation");
    std::string t = v["type"].as_string();
    if (t == "go")
    {
      go_opt opt;
      opt.get(v);
      go annot;
      annot.parse(opt);
      go_opts[opt.name] = opt;
      gos[opt.name] = annot;
      types[opt.name] = "go";
    }
    else if (t == "flat")
    {
      flat_opt opt;
      opt.get(v);
      flat annot;
      annot.parse(opt);
      flat_opts[opt.name] = opt;
      flats[opt.name] = annot;
      types[opt.name] = "flat";
    }
    else if (t == "hierarchical")
    {
      hierarchical_opt opt;
      opt.get(v);
      hierarchical annot;
      annot.parse(opt);
      hierarchical_opts[opt.name] = opt;
      hierarchicals[opt.name] = annot;
      types[opt.name] = "hierarchical";
    }
    else
    {
      system_error("Annotation type " + t + " is unknown.");
    }
  }
};

void enrichment::from_json(web::json::value& json, annotation& ann) {
  if (! json.has_field("enrichment"))
    system_error("At least one enrichment is required");
  if (json["enrichment"].is_array())
  {
    web::json::array arr = json["enrichment"].as_array();
    for (web::json::value& v : arr)
    {
      if (! v.has_field("annotation"))
        system_error("'annotation' is a required field for enrichment");
      std::string annotation = v["annotation"].as_string();
      std::string t = ann.get_type(annotation);
      if (t == "go")
      {
        mapping_opt opt;
        opt.get(v);
        mapping_go map;
        map.parse(opt, ann.go_by_name(opt.annotation));
        mapping_opt_gos[opt.name] = opt;
        mapping_gos[opt.name] = map;
        types[opt.name] = "go";
      }
      else if (t == "flat")
      {
        mapping_opt opt;
        opt.get(v);
        mapping_flat map;
        map.parse(opt, ann.flat_by_name(opt.annotation));
        mapping_opt_flats[opt.name] = opt;
        mapping_flats[opt.name] = map;
        types[opt.name] = "flat";
      }
      else if (t == "hierarchical")
      {
        mapping_opt opt;
        opt.get(v);
        mapping_hierarchical map;
        map.parse(opt, ann.hierarchical_by_name(opt.annotation));
        mapping_opt_hierarchicals[opt.name] = opt;
        mapping_hierarchicals[opt.name] = map;
        types[opt.name] = "hierarchical";
      }
    }
  }
  else
  {
    web::json::value v = json["enrichment"];
    if (! v.has_field("annotation"))
      system_error("'annotation' is a required field for enrichment");
    std::string annotation = v["annotation"].as_string();
    std::string t = ann.get_type(annotation);
    if (t == "go")
    {
      mapping_opt opt;
      opt.get(v);
      mapping_go map;
      map.parse(opt, ann.go_by_name(opt.annotation));
      mapping_opt_gos[opt.name] = opt;
      mapping_gos[opt.name] = map;
      types[opt.name] = "go";
    }
    else if (t == "flat")
    {
      mapping_opt opt;
      opt.get(v);
      mapping_flat map;
      map.parse(opt, ann.flat_by_name(opt.annotation));
      mapping_opt_flats[opt.name] = opt;
      mapping_flats[opt.name] = map;
      types[opt.name] = "flat";
    }
    else if (t == "hierarchical")
    {
      mapping_opt opt;
      opt.get(v);
      mapping_hierarchical map;
      map.parse(opt, ann.hierarchical_by_name(opt.annotation));
      mapping_opt_hierarchicals[opt.name] = opt;
      mapping_hierarchicals[opt.name] = map;
      types[opt.name] = "hierarchical";
    }
  }
  if (json.has_field("background"))
  {
    if (json.at("background").is_array())
    {
      web::json::array backgrounds = json.at("background").as_array();
      for (web::json::value& background : backgrounds)
      {
        for (auto& map : mapping_gos)
        {
          std::string bg_name = background["name"].as_string();
          std::string key = map.first + "_" + bg_name;
          std::string bg_source = background["sourcefile"].as_string();
          background_gos[key] = map.second.make_background(bg_source);
        }
        for (auto& map : mapping_flats)
        {
          std::string bg_name = background["name"].as_string();
          std::string key = map.first + "_" + bg_name;
          std::string bg_source = background["sourcefile"].as_string();
          background_flats[key] = map.second.make_background(bg_source);
        }
        for (auto& map : mapping_hierarchicals)
        {
          std::string bg_name = background["name"].as_string();
          std::string key = map.first + "_" + bg_name;
          std::string bg_source = background["sourcefile"].as_string();
          background_hierarchicals[key] = map.second.make_background(bg_source);
        }
      }
    }
    else
    {
      web::json::value background = json.at("background");
      for (auto& map : mapping_gos)
      {
        std::string bg_name = background["name"].as_string();
        std::string key = map.first + "_" + bg_name;
        std::string bg_source = background["sourcefile"].as_string();
        background_gos[key] = map.second.make_background(bg_source);
      }
      for (auto& map : mapping_flats)
      {
        std::string bg_name = background["name"].as_string();
        std::string key = map.first + "_" + bg_name;
        std::string bg_source = background["sourcefile"].as_string();
        background_flats[key] = map.second.make_background(bg_source);
      }
      for (auto& map : mapping_hierarchicals)
      {
        std::string bg_name = background["name"].as_string();
        std::string key = map.first + "_" + bg_name;
        std::string bg_source = background["sourcefile"].as_string();
        background_hierarchicals[key] = map.second.make_background(bg_source);
      }
    }
  }
}

void append_error(web::json::value& json, std::string msg){
  if (! json.has_field("err"))
  {
    json["err"] = web::json::value::array();
  }
  json["err"][json["err"].size()] = web::json::value::string(msg);
}

void append_warning(web::json::value& json, std::string msg){
  if (! json.has_field("warn"))
  {
    json["warn"] = web::json::value::array();
  }
  json["warn"][json["warn"].size()] = web::json::value::string(msg);
}

std::string concat_uri(std::string prefix, std::string suffix) {
  while(prefix.back() == '/')
  {
    prefix.erase(prefix.size() - 1);
  }
  while(suffix.front() == '/')
  {
    suffix.erase(0);
  }
  return std::string(prefix + "/" + suffix);
}