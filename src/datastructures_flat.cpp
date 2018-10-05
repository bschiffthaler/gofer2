#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <boost/tokenizer.hpp>
#include <boost/filesystem.hpp>
#include <cpprest/json.h>

#include <term.h>
#include <utility.h>
#include <datastructures_flat.h>
#include <utility.h>
#include <BSlogger.hpp>
#include <term.h>

namespace fs = boost::filesystem;

void flat_opt::get(web::json::value& opt) {
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

void flat::parse(flat_opt & opts) {
  logger log(std::cerr, "flat::parse");
  log(LOG_INFO) << "Parsing " << opts.file_in << '\n';
  std::ifstream stream_in(opts.file_in, std::ios::in);
  std::string line;
  while (std::getline(stream_in, line))
  {
    std::stringstream ss(line);
    std::string id;
    std::string name;
    std::string def;
    std::getline(ss, id, '\t');
    std::getline(ss, name, '\t');
    std::getline(ss, def, '\t');
    id = clean(id);
    remove_ws_ip(id);
    name = clean(name);
    def = clean(def);
    term t;
    t.id(id);
    t.name(name);
    t.def(def);
    if (_terms.find(id) != _terms.end())
      log(LOG_WARN) << "Multiple definitions found for term " << id
                    << ". Only the most recent definition will be kept\n";
    _terms[id] = t;
  }
}

term& flat::get(std::string id) {
  logger log(std::cerr, "flat::get");
  auto it = _terms.find(id);
  if (it == _terms.end())
    throw std::runtime_error("Term " + id + " was not found in definition\n");
  return _terms[id];
}

void mapping_flat::parse(mapping_opt& opt, flat& annotation) {
  logger log(std::cerr, "mapping_flat::parse");
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
    for (std::string ter : term_vec)
    {
      if (! annotation.in(ter) )
      {
        log(LOG_WARN) << "Term: " << ter << " in mapping " <<
                      opt.file_in << " not found in annotation.\n";
        term t;
        t.id(ter);
        std::string n("Unknown");
        t.name(n);
        t.def(n);
        annotation.add(ter, t);
      }
      _a_to_b[id].push_back(ter);
      _b_to_a[ter].push_back(id);
    }
  }
  log(LOG_INFO) << "Sorting and removing duplicates\n";
  dedup();
}

void mapping_flat::add(std::unordered_map<std::string, std::vector<std::string>>::iterator it) {
  for (std::string& second : it->second)
  {
    _a_to_b[it->first].push_back(second);
    _b_to_a[second].push_back(it->first);
  }
}

void mapping_flat::dedup() {
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
}

mapping_flat mapping_flat::make_background(std::string & file_in) {
  logger log(std::cerr, "mapping_flat::make_background");
  std::unordered_set<std::string> bg_ids = read_genelist(file_in);
  mapping_flat ret = make_background(bg_ids);
  return ret;
}

mapping_flat
mapping_flat::make_background(std::unordered_set<std::string>& gene_set) {
  logger log(std::cerr, "mapping_flat::make_background");
  mapping_flat ret;
  uint64_t not_found = 0;
  for (auto& it : gene_set)
  {
    auto query = _a_to_b.find(it);
    if (query == _a_to_b.end())
      not_found++;
    else
      ret.add(query);
  }
  ret.dedup();
  log(LOG_WARN) << not_found << " IDs for background were not found in the mapping\n";
  return ret;
}

std::unordered_set<std::string> mapping_flat::all_a(std::unordered_set<std::string>& b) {
  std::unordered_set<std::string> ret;
  for (const std::string& cur_b : b)
  {
    if (_b_to_a.find(cur_b) != _b_to_a.end())
    {
      for (const std::string& cur_a : _b_to_a.at(cur_b))
        ret.insert(cur_a);
    }
  }
  return ret;
}

std::unordered_set<std::string> mapping_flat::all_b(std::unordered_set<std::string>& a) {
  std::unordered_set<std::string> ret;
  for (const std::string& cur_a : a)
  {
    if (_a_to_b.find(cur_a) != _a_to_b.end())
    {
      for (const std::string& cur_b : _a_to_b.at(cur_a))
        ret.insert(cur_b);
    }
  }
  return ret;
}
