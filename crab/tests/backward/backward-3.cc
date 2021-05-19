#include "../program_options.hpp"
#include "../common.hpp"

using namespace std;
using namespace crab::analyzer;
using namespace crab::cfg;
using namespace crab::cfg_impl;
using namespace crab::domain_impl;

/* Example of how to build a CFG */
z_cfg_t* prog(variable_factory_t &vfac)  {

  // Defining program variables
  z_var x(vfac["x"], crab::INT_TYPE, 32);
  z_var y(vfac["y"], crab::INT_TYPE, 32);
  // entry and exit block
  auto cfg = new z_cfg_t("bb1","bb4");
  // adding blocks
  z_basic_block_t& bb1   = cfg->insert("bb1");
  z_basic_block_t& bb2   = cfg->insert("bb2");
  z_basic_block_t& bb3   = cfg->insert("bb3");
  z_basic_block_t& bb4   = cfg->insert("bb4");  
  // adding control flow
  bb1 >> bb2;
  bb2 >> bb3;
  bb3 >> bb2;
  bb2 >> bb4;
  
  // adding statements
  bb1.assign(x, 0);
  bb1.assign(y, 0);
  bb3.assume(x <= 99);
  bb3.add(x, x, 1);
  bb3.add(y, y, 1);
  bb4.assume(x >= 100);
  bb4.assertion(y <= 100);
  return cfg;
}


int main(int argc, char** argv) {
  bool stats_enabled = false;
  if (!crab_tests::parse_user_options(argc,argv,stats_enabled)) {
      return 0;
  }
  variable_factory_t vfac;
  z_cfg_t* cfg = prog(vfac);
  crab::outs() << *cfg << "\n";

  #ifdef HAVE_APRON  
  z_box_apron_domain_t initial_states;
  backward_run<z_box_apron_domain_t>
    (cfg,cfg->entry(),initial_states,1,2,20,stats_enabled);
  #endif
  #ifdef HAVE_ELINA
  z_oct_elina_domain_t initial_states;
  backward_run<z_oct_elina_domain_t>
    (cfg,cfg->entry(),initial_states,1,2,20,stats_enabled);
  #endif

  // free the CFG
  delete cfg;

  return 0;
}
