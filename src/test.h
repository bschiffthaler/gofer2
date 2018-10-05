#pragma once

#include <unordered_set>
#include <string>
#include <datastructures_go.h>
#include <datastructures_flat.h>
#include <datastructures_hierarchical.h>
#include <term.h>
#include <cpprest/json.h>
#include <vector>

struct prank {
  uint64_t orig_pos;
  double cmin;
  double val;
  friend bool operator<(const prank& a,
                        const prank& b);
};

inline bool operator<(const prank& a,
                      const prank& b) {
  return a.val > b.val;
}

std::vector<double> bh_adjust(std::vector< double > pvals);
double log10_binom_coef(uint64_t n, uint64_t k);
uint64_t min(uint64_t a, uint64_t b);
double log10_sigma_t(uint64_t m, uint64_t n, uint64_t mt, uint64_t nt);

class test_go {
public:
  void get_cardinality(std::unordered_set<std::string>& test_set,
                       mapping_go& map, const std::string& id, go& annotation);
  void calc_pval() {pval = log10_sigma_t(mpat, npat, mt, nt);}
  web::json::value to_json(term_go& t, bool name = true, bool def = true);
  uint64_t mpat = 0;
  uint64_t npat = 0;
  uint64_t mt = 0;
  uint64_t nt = 0;
  double pval = 0;
  double padj = 0;
};


class test_fisher {
public:
  void get_cardinality(std::unordered_set<std::string>& test_set,
                       mapping_flat& map, const std::string& id);
  void calc_pval() {pval = log10_sigma_t(m, n, mt, nt);}
  web::json::value to_json(term& t, bool name = true, bool def = true);
  uint64_t m = 0;
  uint64_t n = 0;
  uint64_t mt = 0;
  uint64_t nt = 0;
  double pval = 0;
  double padj = 0;
}; 

class test_hierarchical {
public:
  void get_cardinality(std::unordered_set<std::string>& test_set,
                       mapping_hierarchical& map, const std::string& id, 
                       hierarchical& annotation);
  void calc_pval() {pval = log10_sigma_t(mpat, npat, mt, nt);}
  web::json::value to_json(term& t, bool name = true, bool def = true);
  uint64_t mpat = 0;
  uint64_t npat = 0;
  uint64_t mt = 0;
  uint64_t nt = 0;
  double pval = 0;
  double padj = 0;
};