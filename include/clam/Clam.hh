#pragma once 

/* 
 * Infer invariants using Crab.
 */

#include "clam/ClamAnalysisParams.hh"
#include "clam/crab/crab_cfg.hh"
#include "crab/checkers/base_property.hpp"
#include "llvm/Pass.h"
#include "llvm/ADT/DenseMap.h"

#include <memory>

// forward declarations

namespace clam {
  struct GenericAbsDomWrapper;
  class IntraClam_Impl;
  class InterClam_Impl;
  class CrabBuilderManager;
}

namespace clam {

  using edges_set = std::set<std::pair<const llvm::BasicBlock*, const llvm::BasicBlock*>>;
  
  /**
   * Intra-procedural analysis of a function
   * 
   * Basic usage:
   *    // Create a crab cfg builder manager
   *    CrabBuilderParams params;
   *    auto tli = &getAnalysis<TargetLibraryInfoWrapperPass>().getTLI();
   *    std::unique_ptr<HeapAbstraction> mem(new DummyHeapAbstraction()); 
   *    CrabBuilderManager man(params, tli, std::move(mem));
   *    // Create an intra-procedural analysis 
   *    IntraClam ic(fun, man);
   * 
   *    AnalysisParams params;
   *    ic.analyze(params);
   *    for (auto &b: fun) {
   *      if (auto dom_ptr = ic.get_pre(&b)) {
   *         crab::outs << *dom_ptr << "\n";
   *      }
   *    }
   **/ 
  class IntraClam {
  public:
    
    using wrapper_dom_ptr = std::shared_ptr<GenericAbsDomWrapper>;;
    using abs_dom_map_t = llvm::DenseMap<const llvm::BasicBlock*, wrapper_dom_ptr>;
    using lin_csts_map_t = llvm::DenseMap<const llvm::BasicBlock*, lin_cst_sys_t>;    
    using checks_db_t = crab::checker::checks_db;
    // for backward compatibility with SeaHorn
    using invariant_map_t = abs_dom_map_t;
    using assumption_map_t = lin_csts_map_t;
    
  private:

    std::unique_ptr<IntraClam_Impl> m_impl;
    CrabBuilderManager &m_builder_man;
    const llvm::Function &m_fun;
    abs_dom_map_t m_pre_map;
    abs_dom_map_t m_post_map;
    edges_set m_infeasible_edges;    
    checks_db_t m_checks_db;
    
  public:

    /**
     * Constructor that builds a crab CFG
     **/
    IntraClam(const llvm::Function &fun, CrabBuilderManager &man);
    
    ~IntraClam();    

    /** 
     * Clear all the internal state
     **/
    void clear();

    /* return the manager used to build all CFGs */
    CrabBuilderManager& get_cfg_builder_man();    

    /**
     * Call crab analysis on the CFG under assumptions.
     **/    
    void analyze(AnalysisParams &params, const abs_dom_map_t &assumptions = abs_dom_map_t());

    /**
     * Call crab analysis on the CFG under assumptions starting from entry
     **/    
    void analyze(AnalysisParams &params, const llvm::BasicBlock *entry,
		 const abs_dom_map_t &assumptions);
    
    /**
     * Call crab analysis on the CFG under assumptions.
     **/    
    void analyze(AnalysisParams &params, const lin_csts_map_t &assumptions);

    /**
     * Call crab analysis on the CFG under assumptions starting from entry
     **/    
    void analyze(AnalysisParams &params, const llvm::BasicBlock *entry,
		 const lin_csts_map_t &assumptions);
    
    /**
     * Compute strongest post-condition of an acyclic path.
     * Return false iff the path implies false.
     *
     * post contains the post-conditions at each block.
     * If it returns false then:
     *   - core is a minimal subset of statements that implies false
     **/
    template<typename Statement>
    bool path_analyze(const AnalysisParams& params,
		      const std::vector<const llvm::BasicBlock*>& path,
		      /* use gradually more expensive domains until unsat is proven*/
		      bool layered_solving,
		      std::vector<Statement>& core, abs_dom_map_t& post) const;
    
    template<typename Statement>
    bool path_analyze(const AnalysisParams& params,
		      const std::vector<const llvm::BasicBlock*>& path,
		      /* use gradually more expensive domains until unsat is proven*/
		      bool layered_solving,
		      std::vector<Statement>& core) const;

    /**
     * Return invariants that hold at the entry of b
     **/
    wrapper_dom_ptr get_pre(const llvm::BasicBlock *b, bool keep_shadows=false) const;

    /**
     * Return invariants that hold at the exit of b
     **/
    wrapper_dom_ptr get_post(const llvm::BasicBlock *b, bool keep_shadows=false) const;

    /**
     * Return true if there might be a feasible edge between b1 and b2
     **/
    bool has_feasible_edge(const llvm::BasicBlock *b1, const llvm::BasicBlock* b2) const;
    
    /**
     * Return a database with all checks.
     **/
    const checks_db_t& get_checks_db() const;
  };

  /**
   * Inter-procedural analysis of a module
   **/ 
  class InterClam {
    
  public:

    using wrapper_dom_ptr = typename IntraClam::wrapper_dom_ptr;
    using abs_dom_map_t = typename IntraClam::abs_dom_map_t;
    using lin_csts_map_t = typename IntraClam::lin_csts_map_t;    
    using checks_db_t = typename IntraClam::checks_db_t;
    // for backward compatibility with SeaHorn
    using invariant_map_t = abs_dom_map_t;
    using assumption_map_t = lin_csts_map_t;    
    
  private:

    std::unique_ptr<InterClam_Impl> m_impl;
    CrabBuilderManager &m_builder_man;
    abs_dom_map_t m_pre_map;
    abs_dom_map_t m_post_map;
    edges_set m_infeasible_edges;    
    checks_db_t m_checks_db;
    
  public:

    /**
     * Constructor that builds a crab call graph.
     **/
    InterClam(const llvm::Module &module, CrabBuilderManager &man);

    ~InterClam();    

    /* return the manager used to build all CFGs */
    CrabBuilderManager& get_cfg_builder_man();    
    
    /** 
     * Clear all the internal state
     **/
    void clear();
    
    /**
     * Call crab analysis on the call graph under assumptions.
     **/    
    void analyze(AnalysisParams &params, const abs_dom_map_t &assumptions);

    /**
     * Call crab analysis on the call graph under assumptions.
     **/    
    void analyze(AnalysisParams &params, const lin_csts_map_t &assumptions);
    
    /**
     * Return invariants that hold at the entry of b
     **/
    wrapper_dom_ptr get_pre(const llvm::BasicBlock *b, bool keep_shadows=false) const;

    /**
     * Return invariants that hold at the exit of b
     **/
    wrapper_dom_ptr get_post(const llvm::BasicBlock *b, bool keep_shadows=false) const;

    /**
     * Return true if there might be a feasible edge between b1 and b2
     **/
    bool has_feasible_edge(const llvm::BasicBlock *b1, const llvm::BasicBlock* b2) const;
    
    /**
     * Return a database with all checks.
     **/
    const checks_db_t& get_checks_db() const;
  };
  
  /**
   * LLVM Module pass that computes invariants using Crab.
   **/
  class ClamPass : public llvm::ModulePass {

    using wrapper_dom_ptr = typename IntraClam::wrapper_dom_ptr;
    using abs_dom_map_t = typename IntraClam::abs_dom_map_t;
    using checks_db_t = typename IntraClam::checks_db_t;

    abs_dom_map_t m_pre_map;
    abs_dom_map_t m_post_map;
    edges_set m_infeasible_edges;
    std::unique_ptr<CrabBuilderManager> m_cfg_builder_man;
    checks_db_t m_checks_db; 
    AnalysisParams m_params;
    
   public:

    static char ID;        

    ClamPass();

    /* begin ModulePass API */    
    virtual void releaseMemory();
    
    virtual bool runOnModule(llvm::Module& M);

    virtual bool runOnFunction(llvm::Function &F);

    virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const ;

    virtual llvm::StringRef getPassName() const {
      return "Clam: Crab for Llvm Abstraction Manager";
    }
    /* end ModulePass API */

    /* return the manager used to build all CFGs */
    CrabBuilderManager& get_cfg_builder_man();    

    /* return the analysis options */
    const AnalysisParams& get_analysis_params() const {
      return m_params;
    }

    /* return true if there is Crab CFG for F */
    bool has_cfg(llvm::Function &F);

    /* return the Crab CFG associated to F */
    cfg_ref_t get_cfg(llvm::Function &F);
    
    /**
     * return invariants that hold at the entry of BB
     **/
    wrapper_dom_ptr get_pre(const llvm::BasicBlock *BB, bool KeepShadows=false) const;

    /**
     * return invariants that hold at the exit of BB
     **/
    wrapper_dom_ptr get_post(const llvm::BasicBlock *BB, bool KeepShadows=false) const;

    /**
     * Return true if there might be a feasible edge between b1 and b2
     **/
    bool has_feasible_edge(const llvm::BasicBlock *b1, const llvm::BasicBlock* b2) const;
    
    /**
     * To query and view the analysis results 
     **/

    /* return total number of checks if assertion checker enabled,
       otherwise 0 */
    unsigned get_total_checks() const;
    /* return total number of safe checks if assertion checker
       enabled, otherwise 0 */
    unsigned get_total_safe_checks() const;
    /* return total number of definite error checks if assertion
       checker enabled, otherwise 0 */
    unsigned get_total_error_checks() const;
    /* return total number of possibly error checks if assertion
       checker enabled, otherwise 0 */
    unsigned get_total_warning_checks() const;
    
    void print_checks(llvm::raw_ostream &o) const;
  };

} // end namespace 

