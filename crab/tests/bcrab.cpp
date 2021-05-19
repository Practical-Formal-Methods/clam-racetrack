#include "./crab_lang.hpp"
#include "./crab_dom.hpp"
#include <crab/analysis/bwd_analyzer.hpp>

#include <crab/checkers/assertion.hpp>
#include <crab/checkers/checker.hpp>

// Helper
template<typename CFG, typename Dom, typename IntraBwdAnalyzer>
void backward_run_internal(CFG* cfg,
			   crab::cfg_impl::basic_block_label_t entry,
			   Dom initial_states,
			   unsigned widening, 
			   unsigned narrowing, 
			   unsigned jump_set_size,
			   bool enable_stats){
  
  // Run backward analysis
  crab::outs() << "Invariants using " << Dom::getDomainName () << "\n";
  IntraBwdAnalyzer a(*cfg);
  typename IntraBwdAnalyzer::assumption_map_t assumptions;
  a.run(entry, initial_states, false, assumptions,
	nullptr /*liveness*/,
	widening, narrowing, jump_set_size); 
  
  // Print preconditions
  // Print invariants in DFS to enforce a fixed order
  std::set<crab::cfg_impl::basic_block_label_t> visited;
  std::vector<crab::cfg_impl::basic_block_label_t> worklist;
  worklist.push_back(cfg->entry());
  visited.insert(cfg->entry());
  while (!worklist.empty()) {
    auto cur_label = worklist.back();
    worklist.pop_back();
    auto inv = a[cur_label];
    crab::outs() << crab::cfg_impl::get_label_str(cur_label) << "=" << inv << "\n";
    auto const &cur_node = cfg->get_node(cur_label);
    for (auto const kid_label : boost::make_iterator_range(cur_node.next_blocks())) {
      if (visited.insert(kid_label).second) {
	worklist.push_back(kid_label);
      }
    }
  }
  
  // Check assertions
  const int verbose = 3;
  typedef crab::checker::intra_checker<IntraBwdAnalyzer> checker_t;
  typedef crab::checker::assert_property_checker<IntraBwdAnalyzer> property_t;
  typename checker_t::prop_checker_ptr prop(new property_t(verbose));
  checker_t checker(a,{prop});
  checker.run();
  checker.show(crab::outs());
  
  crab::outs() << "\n";
  if (enable_stats) {
    crab::CrabStats::Print(crab::outs());
    crab::CrabStats::reset();
  }

}

// To run abstract domains defined over integers
template<typename Dom>
void backward_run(crab::cfg_impl::z_cfg_t* cfg,
		  crab::cfg_impl::basic_block_label_t entry,		   
		  Dom initial_states,
		  unsigned widening, 
		  unsigned narrowing, 
		  unsigned jump_set_size,
		  bool enable_stats) {
  using namespace crab::analyzer;
  typedef intra_forward_backward_analyzer<crab::cfg_impl::z_cfg_ref_t, Dom>
    backward_analyzer_t;
  backward_run_internal<crab::cfg_impl::z_cfg_t, Dom, backward_analyzer_t>
    (cfg, entry, initial_states, widening, narrowing, jump_set_size, enable_stats);
}

////////
//// Explicit instantiations here
////////

#include "./bcrab_inst.hpp"
