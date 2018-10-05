#include <test.h>
#include <algorithm>
#include <unordered_set>
#include <string>
#include <cpprest/json.h>
#include <BSlogger.hpp>

void test_go::get_cardinality(std::unordered_set<std::string>& test_set,
                              mapping_go& map,
                              const std::string& id,
                              go& annotation) {
  logger log(std::cerr, "test_go::get_cardinality");
  //log(LOG_INFO) << "Testing " << id << '\n';

  mt = map.b_to_a(id).size();

  for (const std::string& g : test_set)
  {
    //log(LOG_INFO) << "Testing gene " << g << '\n';
    auto begin = map.a_to_b(g).begin();
    auto end = map.a_to_b(g).end();
    if (std::binary_search(begin, end, id))
      nt++;
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
      mpat++;
    }
  }
  for (auto& p : test_set_parents)
  {
    if (p.second == pop_max)
    {
      npat++;
    }
  }
#ifdef DEBUG
  log(LOG_DEBUG) << "Term: " << id << ", mt: " << mt << ", nt: " << nt
                 << ", mpat: " << mpat << ", npat: " << npat << '\n';
#endif
}

void test_fisher::get_cardinality(std::unordered_set<std::string>& test_set,
                                  mapping_flat& map,
                                  const std::string& id) {
  logger log(std::cerr, "test_go::get_cardinality");
  //log(LOG_INFO) << "Testing " << id << '\n';

  mt = map.b_to_a(id).size();
  m = map.a_to_b().size();
  n = test_set.size();
  for (const std::string& g : test_set)
  {
    //log(LOG_INFO) << "Testing gene " << g << '\n';
    auto begin = map.a_to_b(g).begin();
    auto end = map.a_to_b(g).end();
    if (std::binary_search(begin, end, id))
      nt++;
  }
#ifdef DEBUG
  log(LOG_DEBUG) << "Term: " << id << ", mt: " << mt << ", nt: " << nt
                 << ", m: " << m << ", n: " << n << '\n';
#endif
}

void test_hierarchical::get_cardinality(std::unordered_set<std::string>& test_set,
                                        mapping_hierarchical& map,
                                        const std::string& id,
                                        hierarchical& annotation) {
  logger log(std::cerr, "test_hierarchical::get_cardinality");
  //log(LOG_INFO) << "Testing " << id << '\n';

  mt = map.b_to_a(id).size();

  for (const std::string& g : test_set)
  {
    //log(LOG_INFO) << "Testing gene " << g << '\n';
    auto begin = map.a_to_b(g).begin();
    auto end = map.a_to_b(g).end();
    if (std::binary_search(begin, end, id))
      nt++;
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
      mpat++;
    }
  }
  for (auto& p : test_set_parents)
  {
    if (p.second == pop_max)
    {
      npat++;
    }
  }
#ifdef DEBUG
  log(LOG_DEBUG) << "Term: " << id << ", mt: " << mt << ", nt: " << nt
                 << ", mpat: " << mpat << ", npat: " << npat << '\n';
#endif
}

std::vector<double> bh_adjust(std::vector< double > pvals)
{
  std::vector< prank > pr(pvals.size());
  std::vector< double > ret(pvals.size());
  if (pvals.size() == 0)
    return ret;
  //Assign p values and sort descending
  for ( uint64_t i = 0; i < pvals.size(); i++)
  {
    pr[i].orig_pos = i;
    pr[i].val = pvals[i];
  }
  std::sort(pr.begin(), pr.end());

  //Process first entry
  double n = pvals.size();
  pr[0].cmin = pr[0].val * 1;
  if (pr[0].cmin > 1) pr[0].cmin = 1;
  ret[pr[0].orig_pos] = pr[0].cmin;

  double prev = pr[0].cmin;

  //Process rest
  for (uint64_t i = 1; i < pvals.size(); i++)
  {
    double ix = i;
    double cur = n / (n - ix) * pr[i].val;
    if (cur < prev)
    {
      pr[i].cmin = cur > 1 ? 1 : cur;
      prev = cur;
    }
    else
    {
      pr[i].cmin = prev;
    }

    ret[pr[i].orig_pos] = pr[i].cmin;
  }
  return ret;
}


// Approximate n choose k for large numbers by logarithms
// of the data
double log10_binom_coef(uint64_t n, uint64_t k) 
{
  logger log(std::cerr, "gopher:log10_binom_coef");

  if (k > n) 
  {
    log(LOG_WARN) << " k > n in term n choose k. Cannot return log10(0).\n";
    return -1;
  }
  if (k == 0) return log10(1);
  if (k == n) return log10(1);

  double nd = n;

  double r = log10( nd );

  for (uint64_t i = 2; i <= k; i++) 
  {
    double id = i;
    r += log10( ( nd + 1 - id ) / id );
  }
  return r;
}

uint64_t min(uint64_t a, uint64_t b)
{
  return a <= b ? a : b;
}

// Probability to observe sigma_t or more terms, equ (2) in
// Grossman et al.
double log10_sigma_t(uint64_t m, uint64_t n,
                     uint64_t mt, uint64_t nt) 
{
  uint64_t limit = min(mt, n);

  double s = 0;

  for (uint64_t k = nt; k <= limit; k++) 
  {
    // We get logarithms from log10_binom_coef so products
    // become sums and divisions differences
    double t1 = log10_binom_coef(mt, k);
    double t2 = log10_binom_coef(m - mt, n - k);
    double t3 = log10_binom_coef(m, n);

    double e = t1 + t2 - t3;
    s += pow(10, e);
  }

  return s;
}

web::json::value
test_go::to_json(term_go& t, bool name, bool def) {
  web::json::value ret;
  ret["mpat"] = web::json::value::number(mpat);
  ret["npat"] = web::json::value::number(npat);
  ret["mt"] = web::json::value::number(mt);
  ret["nt"] = web::json::value::number(nt);
  ret["pval"] = web::json::value::number(pval);
  ret["padj"] = web::json::value::number(padj);
  ret["id"] = web::json::value::string(t.id());
  if (name)
    ret["name"] = web::json::value::string(t.name());
  if (def)
    ret["def"] = web::json::value::string(t.def());
  switch (t.nspace())
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
  return ret;
}

web::json::value
test_fisher::to_json(term& t, bool name, bool def) {
  web::json::value ret;
  ret["m"] = web::json::value::number(m);
  ret["n"] = web::json::value::number(n);
  ret["mt"] = web::json::value::number(mt);
  ret["nt"] = web::json::value::number(nt);
  ret["pval"] = web::json::value::number(pval);
  ret["padj"] = web::json::value::number(padj);
  ret["id"] = web::json::value::string(t.id());
  if (name)
    ret["name"] = web::json::value::string(t.name());
  if (def)
    ret["def"] = web::json::value::string(t.def());
  return ret;
}

web::json::value
test_hierarchical::to_json(term& t, bool name, bool def) {
  web::json::value ret;
  ret["mpat"] = web::json::value::number(mpat);
  ret["npat"] = web::json::value::number(npat);
  ret["mt"] = web::json::value::number(mt);
  ret["nt"] = web::json::value::number(nt);
  ret["pval"] = web::json::value::number(pval);
  ret["padj"] = web::json::value::number(padj);
  ret["id"] = web::json::value::string(t.id());
  if (name)
    ret["name"] = web::json::value::string(t.name());
  if (def)
    ret["def"] = web::json::value::string(t.def());
  return ret;
}

