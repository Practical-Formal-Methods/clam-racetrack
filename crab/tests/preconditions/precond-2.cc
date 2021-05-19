#include "../program_options.hpp"
#include "../common.hpp"
#include <crab/analysis/bwd_analyzer.hpp>

// Simplified Cousots, Logozzo, and Fahndrich's example (VMCAI'13)
using namespace std;
using namespace crab::analyzer;
using namespace crab::cfg;
using namespace crab::cfg_impl;
using namespace crab::domain_impl;

z_cfg_t* prog(variable_factory_t &vfac)  {

  // Defining program variables
  z_var i(vfac["i"], crab::INT_TYPE, 32);
  z_var n(vfac["n"], crab::INT_TYPE, 32);  
  // entry and exit block
  auto cfg = new z_cfg_t("bb1","bb5");
  // adding blocks
  z_basic_block_t& bb1   = cfg->insert("bb1");
  z_basic_block_t& bb2   = cfg->insert("bb2");
  z_basic_block_t& bb3   = cfg->insert("bb3");
  z_basic_block_t& bb4   = cfg->insert("bb4");
  z_basic_block_t& bb5   = cfg->insert("bb5");    
  // adding control flow
  bb1 >> bb2;
  bb2 >> bb3;
  bb3 >> bb4;
  bb4 >> bb2;
  bb4 >> bb5;
  bb2 >> bb5;
  
  // adding statements
  bb1.assign(i, 0);
  bb3.assume(i <= n);
  bb3.assertion(i >= 0);  
  bb3.assertion(i <= n-1);
  bb3.add(i, i, 1);
  bb5.assume(i >= 100);
  return cfg;
}


int main(int argc, char** argv) {
#if (defined(HAVE_APRON) ||defined(HAVE_ELINA))

  bool stats_enabled = false;
  if (!crab_tests::parse_user_options(argc,argv,stats_enabled)) {
      return 0;
  }
  variable_factory_t vfac;
  z_cfg_t* cfg = prog(vfac);
  crab::outs() << *cfg << "\n";

  {
#ifdef HAVE_APRON
    typedef crab::analyzer::necessary_preconditions_fixpoint_iterator
    <z_cfg_ref_t,z_pk_apron_domain_t> analysis_t;  
    z_pk_apron_domain_t final_states = z_pk_apron_domain_t::bottom();
#endif
#ifdef HAVE_ELINA
    typedef crab::analyzer::necessary_preconditions_fixpoint_iterator
      <z_cfg_ref_t,z_pk_elina_domain_t> analysis_t;  
    z_pk_elina_domain_t final_states = z_pk_elina_domain_t::bottom();
#endif
    analysis_t analyzer(*cfg, nullptr, final_states, false /*error states*/);
    analyzer.run_backward();    
    crab::outs () << "Necessary preconditions from error states using Polyhedra:\n";
    // Print preconditions in DFS to enforce a fixed order
    std::set<crab::cfg_impl::basic_block_label_t> visited;
    std::vector<crab::cfg_impl::basic_block_label_t> worklist;
    worklist.push_back(cfg->entry());
    visited.insert(cfg->entry());
    while (!worklist.empty()) {
      auto cur_label = worklist.back();
      worklist.pop_back();
      auto inv = analyzer[cur_label];
      crab::outs() << crab::cfg_impl::get_label_str(cur_label) << "=" << inv << "\n";
      auto const &cur_node = cfg->get_node(cur_label);
      for (auto const kid_label : boost::make_iterator_range(cur_node.next_blocks())) {
	if (visited.insert(kid_label).second) {
	  worklist.push_back(kid_label);
	}
      }
    }
  }
  {
#ifdef HAVE_APRON
    typedef crab::analyzer::necessary_preconditions_fixpoint_iterator
    <z_cfg_ref_t,z_pk_apron_domain_t> analysis_t;  
    z_pk_apron_domain_t final_states = z_pk_apron_domain_t::top();
#endif
#ifdef HAVE_ELINA
    typedef crab::analyzer::necessary_preconditions_fixpoint_iterator
      <z_cfg_ref_t,z_pk_elina_domain_t> analysis_t;  
    z_pk_elina_domain_t final_states = z_pk_elina_domain_t::top();
#endif
    analysis_t analyzer(*cfg, nullptr, final_states, true /*good states*/);
    analyzer.run_backward();    
    crab::outs () << "Necessary preconditions from safe states using Polyhedra:\n";
    // Print preconditions in DFS to enforce a fixed order
    std::set<crab::cfg_impl::basic_block_label_t> visited;
    std::vector<crab::cfg_impl::basic_block_label_t> worklist;
    worklist.push_back(cfg->entry());
    visited.insert(cfg->entry());
    while (!worklist.empty()) {
      auto cur_label = worklist.back();
      worklist.pop_back();
      auto inv = analyzer[cur_label];
      crab::outs() << crab::cfg_impl::get_label_str(cur_label) << "=" << inv << "\n";
      auto const &cur_node = cfg->get_node(cur_label);
      for (auto const kid_label : boost::make_iterator_range(cur_node.next_blocks())) {
	if (visited.insert(kid_label).second) {
	  worklist.push_back(kid_label);
	}
      }
    }    
  }    
  // free the CFG
  delete cfg;
#endif
  return 0;
}
