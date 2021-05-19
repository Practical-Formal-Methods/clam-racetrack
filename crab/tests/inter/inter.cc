#include "../program_options.hpp"
#include "../common.hpp"

#include <crab/cg/cg_bgl.hpp>
#include <crab/analysis/graphs/sccg_bgl.hpp>

using namespace std;
using namespace crab::analyzer;
using namespace crab::cfg;
using namespace crab::cfg_impl;
using namespace crab::domain_impl;
using namespace crab::cg;

z_cfg_t* foo (variable_factory_t &vfac) {
  // Defining program variables
  z_var x (vfac ["x"], crab::INT_TYPE, 32);
  z_var y (vfac ["y"], crab::INT_TYPE, 32);
  z_var z (vfac ["z"], crab::INT_TYPE, 32);
  
  function_decl<z_number, varname_t> decl ("foo", {x}, {z});
  // entry and exit block
  z_cfg_t* cfg = new z_cfg_t("entry", "exit", decl);
  // adding blocks
  z_basic_block_t& entry = cfg->insert ("entry");
  z_basic_block_t& exit   = cfg->insert ("exit");
  // adding control flow
  entry >> exit;
  // adding statements
  entry.add (y, x, 1);
  exit.add (z , y , 2);
  exit.ret (z);
  return cfg;
}

z_cfg_t* rec1 (variable_factory_t &vfac) {
  // Defining program variables
  z_var r (vfac ["r"], crab::INT_TYPE, 32);
  z_var s (vfac ["s"], crab::INT_TYPE, 32);
  z_var t (vfac ["t"], crab::INT_TYPE, 32);
  
  function_decl<z_number, varname_t> decl ("rec1", {s},{t});
  // entry and exit block
  z_cfg_t* cfg  = new z_cfg_t("entry", "exit", decl);
  // adding blocks
  z_basic_block_t& entry = cfg->insert ("entry");
  z_basic_block_t& exit   = cfg->insert ("exit");
  // adding control flow
  entry >> exit;
  // adding statements
  entry.sub (r, s, 1);
  exit.callsite ("rec2", {t}, {r});
  exit.ret (t);
  return cfg;
}

z_cfg_t* rec2 (variable_factory_t &vfac) {
  // Defining program variables
  z_var r (vfac ["r1"], crab::INT_TYPE, 32);
  z_var s (vfac ["s1"], crab::INT_TYPE, 32);
  z_var t (vfac ["t1"], crab::INT_TYPE, 32);
  
  function_decl<z_number, varname_t> decl ("rec2", {s},{t});
  // entry and exit block
  z_cfg_t* cfg = new z_cfg_t("entry", "exit", decl);
  // adding blocks
  z_basic_block_t& entry = cfg->insert ("entry");
  z_basic_block_t& exit   = cfg->insert ("exit");
  // adding control flow
  entry >> exit;
  // adding statements
  entry.sub (r, s, 1);
  exit.callsite ("rec1", {t}, {r});
  //exit.callsite ("foo", {t}, {t});
  exit.ret (t);
  return cfg;
}


z_cfg_t* bar (variable_factory_t &vfac) {
  // Defining program variables
  z_var a (vfac ["a"], crab::INT_TYPE, 32);
  z_var x (vfac ["x1"], crab::INT_TYPE, 32);
  z_var y (vfac ["y1"], crab::INT_TYPE, 32);
  z_var w (vfac ["w1"], crab::INT_TYPE, 32);
  
  function_decl<z_number, varname_t> decl ("bar",{a},{y});
  // entry and exit block
  z_cfg_t* cfg = new z_cfg_t("entry", "exit", decl);
  // adding blocks
  z_basic_block_t& entry = cfg->insert ("entry");
  z_basic_block_t& exit   = cfg->insert ("exit");
  // adding control flow
  entry >> exit;
  // adding statements
  exit.callsite ("foo", {y}, {x});
  entry.assign (x, a);
  entry.assign (w, 5);
  exit.ret (y);
  return cfg;
}

z_cfg_t* m (variable_factory_t &vfac)  {
  // Defining program variables
  z_var x (vfac ["x2"], crab::INT_TYPE, 32);
  z_var y (vfac ["y2"], crab::INT_TYPE, 32);
  z_var z (vfac ["z2"], crab::INT_TYPE, 32);
  z_var z1 (vfac ["z3"], crab::INT_TYPE, 32);  
  z_var w (vfac ["w2"], crab::INT_TYPE, 32);  
  
  function_decl<z_number, varname_t> decl ("main", {}, {w});
				 
  // entry and exit block
  z_cfg_t* cfg = new z_cfg_t("entry", "exit", decl);
  // adding blocks
  z_basic_block_t& entry = cfg->insert ("entry");
  z_basic_block_t& exit   = cfg->insert ("exit");
  // adding control flow
  entry >> exit;
  // adding statements
  entry.assign(x, 3);
  entry.callsite ("bar", {y}, {x});
  /////
  entry.callsite ("rec1", {z1}, {y});
  /////
  exit.add (z, y, 2);
  exit.callsite ("foo", {w}, {z});
  exit.ret (w);
  return cfg;
}

int main (int argc, char** argv) {
  bool stats_enabled = false;
  if (!crab_tests::parse_user_options(argc,argv,stats_enabled)) {
      return 0;
  }
  variable_factory_t vfac;
  z_cfg_t* t1 = foo (vfac);
  z_cfg_t* t2 = bar (vfac);
  z_cfg_t* t3 = rec1 (vfac);
  z_cfg_t* t4 = rec2 (vfac);
  z_cfg_t* t5 = m (vfac);

  crab::outs() << *t1 << "\n";
  crab::outs() << *t2 << "\n";
  crab::outs() << *t3 << "\n";
  crab::outs() << *t4 << "\n";
  crab::outs() << *t5 << "\n";

  vector<z_cfg_ref_t> cfgs;
  cfgs.push_back(*t1);
  cfgs.push_back(*t2);
  cfgs.push_back(*t3);
  cfgs.push_back(*t4);
  cfgs.push_back(*t5);

  typedef call_graph<z_cfg_ref_t> callgraph_t;

  boost::scoped_ptr<callgraph_t> cg(new callgraph_t(cfgs));
  
  inter_run<z_dbm_domain_t, z_interval_domain_t>(&*cg,false,2,2,20,stats_enabled);
#ifdef HAVE_APRON  
  inter_run<z_oct_apron_domain_t, z_interval_domain_t>(&*cg,false,2,2,20,stats_enabled);
#endif
  /// nothing wrong with this test but it prints invariants differently
  /// on Linux and mac.
  //inter_run<z_term_domain_t, z_interval_domain_t>(&*cg,false,2,2,20,stats_enabled);
  //inter_run<z_num_domain_t, z_num_domain_t>(&*cg,false,2,2,20,stats_enabled);

  delete t1;
  delete t2;
  delete t3;
  delete t4;
  delete t5;

  return 0;
}
