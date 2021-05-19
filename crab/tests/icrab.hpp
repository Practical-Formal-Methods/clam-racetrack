#ifndef __CRAB_INTER_ANALYZERS__
#define __CRAB_INTER_ANALYZERS__

#include "./crab_lang.hpp"

// To run abstract domains defined over integers
template<typename BUDom, typename TDDom>
extern void inter_run (crab::cg_impl::z_cg_t* cg, 
		       bool run_liveness,
		       unsigned widening, 
		       unsigned narrowing, 
		       unsigned jump_set_size,
		       bool enable_stats);
   
#endif 

