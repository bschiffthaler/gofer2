#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <boost/tokenizer.hpp>
#include <boost/filesystem.hpp>
#include <cpprest/json.h>

#include <term.h>
#include <utility.h>
#include <datastructures_go.h>
#include <utility.h>
#include <BSlogger.hpp>

namespace fs = boost::filesystem;

void go_opt::get(web::json::value& opt) {
  if (opt.has_field("parent"))
  {
    if (opt["parent"].is_array())
    {
      web::json::array arr = opt["parent"].as_array();
      for (web::json::value& v : arr)
      {
        std::string q = v.as_string();
        if (q == "part_of")
        {
          part_of = true;
        }
        else if (q == "regulates")
        {
          regulates = true;
        }
        else if (q == "negatively_regulates")
        {
          negatively_regulates = true;
        }
        else if (q == "positively_regulates")
        {
          positively_regulates = true;
        }
        else if (q == "occurs_in")
        {
          occurs_in = true;
        }
        else
        {
          throw std::runtime_error("Unsupported relationship in OBO parent "
                                   "option: " + q);
        }
      }
    }
    else if (opt["parent"].is_string())
    {
      std::string q = opt["parent"].as_string();
      if (q == "all")
        all_relationships = true;
      else
        throw std::runtime_error("Unsupported relationship in OBO parent "
                                 "option: " + q);
    }
    else
    {
      throw std::runtime_error("Parent option of OBO sections must be either "
                               "string or array.");
    }
  }

  if (opt.has_field("sourcefile"))
  {
    file_in = opt["sourcefile"].as_string();
    fs::path p(file_in);
    file_in = fs::canonical(p).string();
  }
  else
  {
    throw std::runtime_error("sourcefile is a required option");
  }

  if (opt.has_field("name"))
    name = opt["name"].as_string();
  else
    throw std::runtime_error("sourcefile is a required annotation option");

}

void go::parse(go_opt & opts) {
  logger log(std::cerr, "go::parse");
  log(LOG_INFO) << "Parsing " << opts.file_in << '\n';
  std::ifstream stream_in(opts.file_in, std::ios::in);
  std::string line;
  // Skip to first [Term]
  while (std::getline(stream_in, line)) {
    remove_ws_ip(line);
    if (line == "[Term]") break;
  }
  term_go go;
  bool term = true;
  while (std::getline(stream_in, line)) {
    std::string line_no_ws = remove_ws(line);
    if (line_no_ws[0] == '[')
    {
      if (term)
      {
        if (go.nspace() == GO_BP)
        {
          _bp[go.id()] = go;
        }
        else if (go.nspace() == GO_MF)
        {
          _mf[go.id()] = go;
        }
        else if (go.nspace() == GO_CC)
        {
          _cc[go.id()] = go;
        }
        _namespaces[go.id()] = go.nspace();
        go = term_go();
      }
      if (line_no_ws == "[Term]")
      {
        term = true;
        continue;
      }
      else
      {
        term = false;
      }
    }
    if (! term) continue;
    auto item = split_first_of(line, ": ");
    if (item.first == "id")
    {
      std::string id = remove_ws(item.second);
      go.id(id);
    }
    else if (item.first == "name")
    {
      std::string name = clean(item.second);
      go.name(name);
    }
    else if (item.first == "def")
    {
      std::string def = clean(item.second);
      go.def(def);
    }
    else if (item.first == "is_obsolete")
    {
      go.obsolete(true);
    }
    else if (item.first == "alt_id")
    {
      std::string id = remove_ws(item.second);
      _alt_id[id] = go.id();
    }
    else if (item.first == "relationship")
    {
      std::stringstream ss(item.second);
      std::string rel;
      std::string par;
      ss >> rel;
      ss >> par;
      if (opts.all_relationships)
        go.parents(par);
      else if (rel == "part_of" && opts.part_of)
        go.parents(par);
      else if (rel == "regulates" && opts.regulates)
        go.parents(par);
      else if (rel == "negatively_regulates" && opts.negatively_regulates)
        go.parents(par);
      else if (rel == "positively_regulates" && opts.positively_regulates)
        go.parents(par);
      else if (rel == "occurs_in" && opts.occurs_in)
        go.parents(par);
    }
    else if (item.first == "is_a")
    {
      std::string id = remove_ws(item.second);
      std::string cl;
      for (char c : id)
      {
        if (c == '!')
          break;
        else
          cl += c;
      }
      go.parents(cl);
    }
    else if (item.first == "namespace")
    {
      std::string id = remove_ws(item.second);
      if (id == "molecular_function")
      {
        go.nspace(GO_MF);
      }
      else if (id == "biological_process")
      {
        go.nspace(GO_BP);
      }
      else if (id == "cellular_component")
      {
        go.nspace(GO_CC);
      }
      else
      {
        throw std::runtime_error("Encountered GO term with unknown "
                                 "namespace: " + id);
      }
    }
    else
    {
      continue;
    }
  }
  log(LOG_INFO) << "Have " << _bp.size() << " biological process terms\n";
  log(LOG_INFO) << "Have " << _mf.size() << " molecular function terms\n";
  log(LOG_INFO) << "Have " << _cc.size() << " cellular component terms\n";
}

term_go& go::get(std::string id) {
  logger log(std::cerr, "go::get");
  auto it = _alt_id.find(id);
  if (it != _alt_id.end())
  {
    id = it->second;
  }
  uint8_t nspace = _namespaces[id];
  if (nspace == GO_BP)
    return _bp[id];
  else if (nspace == GO_MF)
    return _mf[id];
  else if (nspace == GO_CC)
    return _cc[id];
  else
    throw std::runtime_error("Term " + id + " not in annotation");
}

std::unordered_set<std::string> go::parents_of(std::string id) {
  std::unordered_set<std::string> ret;
  _rec_parents_of(ret, id);
  return ret;
}

// Recursive iteration through term's parents in order to add them to the
// set
void go::_rec_parents_of(std::unordered_set<std::string>& s, std::string & id) {

  term_go& t = get(id);
  for (std::string& p : t.parents())
  {
    //if (! std::binary_search(_root.begin(), _root.end(), p))
    s.insert(p);
    _rec_parents_of(s, p);
  }
}

void mapping_go::parse(mapping_opt& opt, go& annotation) {
  logger log(std::cerr, "mapping_go::parse");
  log(LOG_INFO) << "Parsing " << opt.file_in << '\n';
  std::ifstream stream_in(opt.file_in, std::ios::in);
  std::string line;
  boost::char_separator<char> sep(",;|");
  while (std::getline(stream_in, line))
  {
    std::stringstream ss(line);
    std::string id;
    std::string terms;
    std::vector<std::string> term_vec;
    ss >> id;
    ss >> terms;
    tokenizer tokens(terms, sep);
    for (auto it = tokens.begin(); it != tokens.end(); it++)
      term_vec.push_back(*it);
    for (std::string term : term_vec)
    {
      // Check if we have annotation for this
      if(! annotation.in(term) )
        throw std::runtime_error("Term: " + term + " in mapping " +
                                 opt.file_in + " not found in annotation.");
      // Add most pecific term only to _spec map
      _a_to_b_spec[id].push_back(term);
      _b_to_a_spec[term].push_back(id);
      // Add most specific and parent terms to true path math
      _a_to_b[id].push_back(term);
      _b_to_a[term].push_back(id);
      for (const std::string& p : annotation.parents_of(term))
      {
        _a_to_b[id].push_back(p);
        _b_to_a[p].push_back(id);
      }
    }
  }
  log(LOG_INFO) << "Sorting and removing duplicates\n";
  dedup();
}

void mapping_go::add(std::unordered_map<std::string, std::vector<std::string>>::iterator it,
                     bool spec) {
  for (std::string& second : it->second)
  {
    if (spec)
    {
      _a_to_b_spec[it->first].push_back(second);
      _b_to_a_spec[second].push_back(it->first);
    }
    else
    {
      _a_to_b[it->first].push_back(second);
      _b_to_a[second].push_back(it->first);
    }
  }
}

void mapping_go::dedup() {
  for (auto& it : _a_to_b)
  {
    std::sort(it.second.begin(), it.second.end());
    auto uit = std::unique(it.second.begin(), it.second.end());
    it.second.resize(std::distance(it.second.begin(), uit));
  }
  for (auto& it : _b_to_a)
  {
    std::sort(it.second.begin(), it.second.end());
    auto uit = std::unique(it.second.begin(), it.second.end());
    it.second.resize(std::distance(it.second.begin(), uit));
  }
  for (auto& it : _a_to_b_spec)
  {
    std::sort(it.second.begin(), it.second.end());
    auto uit = std::unique(it.second.begin(), it.second.end());
    it.second.resize(std::distance(it.second.begin(), uit));
  }
  for (auto& it : _b_to_a_spec)
  {
    std::sort(it.second.begin(), it.second.end());
    auto uit = std::unique(it.second.begin(), it.second.end());
    it.second.resize(std::distance(it.second.begin(), uit));
  }
}

mapping_go
mapping_go::make_background(std::string & file_in) {
  logger log(std::cerr, "mapping_go::make_background");
  std::unordered_set<std::string> bg_ids = read_genelist(file_in);
  mapping_go ret = make_background(bg_ids);
  return ret;
}


mapping_go
mapping_go::make_background(std::unordered_set<std::string>& gene_set) {
  logger log(std::cerr, "mapping_go::make_background");
  mapping_go ret;
  uint64_t not_found = 0;
  for (auto& it : gene_set)
  {
    auto query = _a_to_b.find(it);
    if (query == _a_to_b.end())
      not_found++;
    else
    {
      ret.add(query);
      query = _a_to_b_spec.find(it);
      ret.add(query, true);
    }
  }
  ret.dedup();
  if (not_found > 0)
    log(LOG_WARN) << not_found << " IDs for background were not found in the mapping\n";
  return ret;
}

std::unordered_set<std::string> mapping_go::all_a(std::unordered_set<std::string>& b,
    bool spec) {
  std::unordered_set<std::string> ret;
  for (const std::string& cur_b : b)
  {
    if (spec)
    {
      if (_b_to_a_spec.find(cur_b) != _b_to_a_spec.end())
      {
        for (const std::string& cur_a : _b_to_a_spec.at(cur_b))
          ret.insert(cur_a);
      }
    }
    else
    {
      if (_b_to_a.find(cur_b) != _b_to_a.end())
      {
        for (const std::string& cur_a : _b_to_a.at(cur_b))
          ret.insert(cur_a);
      }
    }
  }
  return ret;
}

std::unordered_set<std::string> mapping_go::all_b(std::unordered_set<std::string>& a,
    bool spec) {
  std::unordered_set<std::string> ret;
  for (const std::string& cur_a : a)
  {
    if (spec)
    {
      if (_a_to_b_spec.find(cur_a) != _a_to_b_spec.end())
      {
        for (const std::string& cur_b : _a_to_b_spec.at(cur_a))
          ret.insert(cur_b);
      }
    }
    else
    {
      if (_a_to_b.find(cur_a) != _a_to_b.end())
      {
        for (const std::string& cur_b : _a_to_b.at(cur_a))
          ret.insert(cur_b);
      }
    }
  }
  return ret;
}
