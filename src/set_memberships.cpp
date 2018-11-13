#include <algorithm>
#include <unordered_set>
#include <string>
#include <cpprest/json.h>
#include <BSlogger.hpp>
#include <set_memberships.h>

void set_membership_go::get_sets(std::unordered_set<std::string>& test_set,
                                 mapping_go& map,
                                 const std::string& id,
                                 go& annotation) {
  logger log(std::cerr, "set_membership_go::get_sets");

  mt.reserve(map.b_to_a(id).size());
  for (auto& g : map.b_to_a(id))
    mt.push_back(g);

  for (const std::string& g : test_set)
  {
    auto begin = map.a_to_b(g).begin();
    auto end = map.a_to_b(g).end();
    if (std::binary_search(begin, end, id))
      nt.push_back(g);
  }

  // intersection of genes annotated to parents of id
  std::unordered_set<std::string> parents = annotation.parents_of(id);

  // Intersection of genes in population annotated to parents of id
  std::unordered_map<std::string, uint64_t> pop_set_parents;
  std::unordered_map<std::string, uint64_t> test_set_parents;
  for (const std::string& g : test_set)
  {
    test_set_parents[g] = 0;
  }
  uint64_t pop_max = 0;
  for (const std::string& p : parents)
  {
    pop_max++;
    //log(LOG_INFO) << "Testing parent term " << p << '\n';
    for (auto& g : map.b_to_a(p))
    {
      auto it = pop_set_parents.find(g);
      if (it == pop_set_parents.end())
        pop_set_parents[g] = 1;
      else
        it->second++;
      // In the test set, do not create new
      it = test_set_parents.find(g);
      if (it != test_set_parents.end())
        it->second++;
    }
  }
  for (auto& p : pop_set_parents)
  {
    if (p.second == pop_max)
    {
      mpat.push_back(p.first);
    }
  }
  for (auto& p : test_set_parents)
  {
    if (p.second == pop_max)
    {
      npat.push_back(p.first);
    }
  }
#ifdef DEBUG
  log(LOG_DEBUG) << "Term: " << id << ", mt: " << mt << ", nt: " << nt
                 << ", mpat: " << mpat << ", npat: " << npat << '\n';
#endif
}

void set_membership_flat::get_sets(std::unordered_set<std::string>& test_set,
                                   mapping_flat& map,
                                   const std::string& id) {
  logger log(std::cerr, "set_memberships_flat::get_sets");
  //log(LOG_INFO) << "Testing " << id << '\n';

  for (auto& p : map.b_to_a(id))
    mt.push_back(p);

  for (auto& p : test_set)
    n.push_back(p);

  for (auto& p : map.a_to_b())
    m.push_back(p.first);

  for (const std::string& g : test_set)
  {
    //log(LOG_INFO) << "Testing gene " << g << '\n';
    auto begin = map.a_to_b(g).begin();
    auto end = map.a_to_b(g).end();
    if (std::binary_search(begin, end, id))
      nt.push_back(g);
  }
#ifdef DEBUG
  log(LOG_DEBUG) << "Term: " << id << ", mt: " << mt << ", nt: " << nt
                 << ", m: " << m << ", n: " << n << '\n';
#endif
}

void set_membership_hierarchical::get_sets(std::unordered_set<std::string>& test_set,
    mapping_hierarchical& map,
    const std::string& id,
    hierarchical& annotation) {
  logger log(std::cerr, "set_membership_hierarchical::get_sets");

  mt.reserve(map.b_to_a(id).size());
  for (auto& g : map.b_to_a(id))
    mt.push_back(g);

  for (const std::string& g : test_set)
  {
    auto begin = map.a_to_b(g).begin();
    auto end = map.a_to_b(g).end();
    if (std::binary_search(begin, end, id))
      nt.push_back(g);
  }
  // intersection of genes annotated to parents of id
  std::vector<std::string> parents = annotation.parents_of(id);
  // Intersection of genes in population annotated to parents of id
  std::unordered_map<std::string, uint64_t> pop_set_parents;
  std::unordered_map<std::string, uint64_t> test_set_parents;
  for (const std::string& g : test_set)
  {
    test_set_parents[g] = 0;
  }
  uint64_t pop_max = 0;
  for (const std::string& p : parents)
  {
    pop_max++;
    //log(LOG_INFO) << "Testing parent term " << p << '\n';
    for (auto& g : map.b_to_a(p))
    {
      auto it = pop_set_parents.find(g);
      if ( it == pop_set_parents.end())
        pop_set_parents[g] = 1;
      else
        it->second++;
      // In the test set, do not create new
      it = test_set_parents.find(g);
      if (it != test_set_parents.end())
        it->second++;
    }
  }
  for (auto& p : pop_set_parents)
  {
    if (p.second == pop_max)
    {
      mpat.push_back(p.first);
    }
  }
  for (auto& p : test_set_parents)
  {
    if (p.second == pop_max)
    {
      npat.push_back(p.first);
    }
  }
#ifdef DEBUG
  log(LOG_DEBUG) << "Term: " << id << ", mt: " << mt << ", nt: " << nt
                 << ", mpat: " << mpat << ", npat: " << npat << '\n';
#endif
}

web::json::value set_membership_go::to_json(){
  web::json::value ret;
  ret["mt"] = web::json::value::array();
  ret["nt"] = web::json::value::array();
  ret["mpat"] = web::json::value::array();
  ret["npat"] = web::json::value::array();

  for (std::string& s : mt) 
    ret["mt"][ret["mt"].size()] = web::json::value::string(s);

  for (std::string& s : nt) 
    ret["nt"][ret["nt"].size()] = web::json::value::string(s);

  for (std::string& s : mpat) 
    ret["mpat"][ret["mpat"].size()] = web::json::value::string(s);

  for (std::string& s : npat) 
    ret["npat"][ret["npat"].size()] = web::json::value::string(s);

  return ret;
}

web::json::value set_membership_hierarchical::to_json(){
  web::json::value ret;
  ret["mt"] = web::json::value::array();
  ret["nt"] = web::json::value::array();
  ret["mpat"] = web::json::value::array();
  ret["npat"] = web::json::value::array();

  for (std::string& s : mt) 
    ret["mt"][ret["mt"].size()] = web::json::value::string(s);

  for (std::string& s : nt) 
    ret["nt"][ret["nt"].size()] = web::json::value::string(s);

  for (std::string& s : mpat) 
    ret["mpat"][ret["mpat"].size()] = web::json::value::string(s);

  for (std::string& s : npat) 
    ret["npat"][ret["npat"].size()] = web::json::value::string(s);

  return ret;
}

web::json::value set_membership_flat::to_json(){
  web::json::value ret;
  ret["mt"] = web::json::value::array();
  ret["nt"] = web::json::value::array();
  ret["m"] = web::json::value::array();
  ret["n"] = web::json::value::array();

  for (std::string& s : mt) 
    ret["mt"][ret["mt"].size()] = web::json::value::string(s);

  for (std::string& s : nt) 
    ret["nt"][ret["nt"].size()] = web::json::value::string(s);

  for (std::string& s : m) 
    ret["m"][ret["m"].size()] = web::json::value::string(s);

  for (std::string& s : n) 
    ret["n"][ret["n"].size()] = web::json::value::string(s);

  return ret;
}