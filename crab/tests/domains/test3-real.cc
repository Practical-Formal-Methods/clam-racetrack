#include "../program_options.hpp"
#include "../common.hpp"


// To run abstract domains defined over reals

using namespace crab::cfg;
using namespace crab::cfg_impl;
using namespace crab::domain_impl;

/* Example of how to build a CFG */
q_cfg_t* prog (variable_factory_t &vfac)  {

  // Definining program variables
  q_var x (vfac ["x"], crab::REAL_TYPE);
  q_var y (vfac ["y"], crab::REAL_TYPE);  
  // entry and exit block
  q_cfg_t* cfg = new q_cfg_t("entry","exit");
  // adding blocks
  q_basic_block_t& entry = cfg->insert ("entry");
  q_basic_block_t& header   = cfg->insert ("header");
  q_basic_block_t& body   = cfg->insert ("body");
  q_basic_block_t& exit   = cfg->insert ("exit");
  
  // adding control flow 
  entry >> header;
  header >> body;
  body >> header;
  header >> exit;
  
  // adding statements
  entry.assign (x, q_number(1));
  entry.assign (y, q_number(0));
  
  body.add(x, x, y);
  body.add(y, y, q_number(1));  

  exit.assertion (x >= y);
  return cfg;
}

/* Example of how to infer invariants from the above CFG */
int main (int argc, char** argv) {
  bool stats_enabled = false;
  if (!crab_tests::parse_user_options(argc,argv,stats_enabled)) {
      return 0;
  }
  variable_factory_t vfac;
  q_cfg_t* cfg = prog (vfac);
  crab::outs() << *cfg << "\n";

  run<q_interval_domain_t>(cfg,cfg->entry(),false,1,2,20,stats_enabled);
  #ifdef HAVE_APRON
  run<q_pk_apron_domain_t>(cfg,cfg->entry(),false,1,2,20,stats_enabled);
  #endif
  #ifdef HAVE_LDD
  run<q_boxes_domain_t>(cfg,cfg->entry(),false,1,2,20,stats_enabled);
  #endif   
  return 0;
}
