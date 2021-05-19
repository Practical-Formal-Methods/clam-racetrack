#pragma once

/**  Program transformation to replace indirect calls with direct calls **/

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/InstVisitor.h"

#include <memory>
#include <map> // for multimap

namespace llvm {
class Module;
class Function;
class CallSite;
class PointerType;
class CallGraph;
} // namespace llvm

namespace clam {

namespace devirt_impl {
using AliasSetId = const llvm::PointerType *;

/// returns an id of an alias set to which this function belongs
/// requires that CS is an indirect call through a function pointer
AliasSetId typeAliasId(const llvm::Function &F);

/// returns an id of an alias set of the called value
AliasSetId typeAliasId(llvm::CallSite &CS);
} // end namespace devirt_impl

enum CallSiteResolverKind {
   RESOLVER_TYPES
 , RESOLVER_DSA /* deprecated, for debugging purposes*/
 , RESOLVER_SEA_DSA
};

struct DevirtStats {
  // number of indirect calls
  unsigned m_num_indirect_calls;
  // number of calls resolved
  unsigned m_num_resolved_calls;
  // number of calls unresolved by Dsa
  unsigned m_num_dsa_unresolved;
  // number of calls which were resolved by Dsa but no consistent type
  // signature was found
  unsigned m_num_type_unresolved;

  DevirtStats():
    m_num_indirect_calls(0), m_num_resolved_calls(0),
    m_num_dsa_unresolved(0), m_num_type_unresolved(0) {}

  void dump() const;
};

/*
 * Generic class API for resolving indirect calls
 */
class CallSiteResolver {
protected:
  CallSiteResolverKind m_kind;
  DevirtStats& m_stats;
  
  CallSiteResolver(CallSiteResolverKind kind, DevirtStats &stats)
    : m_kind(kind), m_stats(stats) {}

public:
  using AliasSetId = devirt_impl::AliasSetId;  
  using AliasSet = llvm::SmallVector<const llvm::Function *, 16>;

  virtual ~CallSiteResolver() {}

  CallSiteResolverKind get_kind() const { return m_kind; }

  /* return all possible targets for CS */
  virtual const AliasSet* getTargets(llvm::CallSite &CS) = 0;

  /* for reusing bounce functions */
  virtual llvm::Function* getBounceFunction(llvm::CallSite &CS) = 0;
  virtual void cacheBounceFunction(llvm::CallSite &CS, llvm::Function* bounce) = 0;
};

/*
 * Resolve an indirect call by selecting all functions defined in the
 * same module whose type signature matches with the callsite.
 */
class CallSiteResolverByTypes: public CallSiteResolver {
public:
  using AliasSetId = CallSiteResolver::AliasSetId;  
  using AliasSet = CallSiteResolver::AliasSet;
  
  CallSiteResolverByTypes(llvm::Module &M, DevirtStats &stats);

  virtual ~CallSiteResolverByTypes();

  const AliasSet* getTargets(llvm::CallSite& CS);

  llvm::Function* getBounceFunction(llvm::CallSite& CS);
  
  void cacheBounceFunction(llvm::CallSite& CS, llvm::Function* bounce);
  
private:
  /* invariant: the value in TargetsMap's entries is sorted */
  using TargetsMap = llvm::DenseMap<AliasSetId, AliasSet>;
  using BounceMap = llvm::DenseMap<AliasSetId, llvm::Function *>;
  
  // -- the module
  llvm::Module &m_M;
  // -- map from alias-id to the corresponding targets
  TargetsMap m_targets_map;
  // -- map alias set id to an existing bounce function
  BounceMap m_bounce_map;
  
  void populateTypeAliasSets(void);
};

/*
 * Resolve indirect call by using DSA pointer analysis.
 */
template<typename Dsa>  
class CallSiteResolverByDsa final: public CallSiteResolverByTypes {
  /*
    Assume that Dsa provides these methods:
    - bool isComplete(CallSite&)
    - iterator begin(CallSite&)
    - iterator end(CallSite&) 
    where each element of iterator is of type Function*
  */
  
public:
  using AliasSetId = CallSiteResolverByTypes::AliasSetId;  
  using AliasSet = CallSiteResolverByTypes::AliasSet;
  
  CallSiteResolverByDsa(llvm::Module& M, Dsa& dsa,
			bool incomplete, unsigned max_num_targets, DevirtStats &stats);
    
  ~CallSiteResolverByDsa();
  
  const AliasSet* getTargets(llvm::CallSite &CS);

  llvm::Function* getBounceFunction(llvm::CallSite& CS);
  
  void cacheBounceFunction(llvm::CallSite&CS, llvm::Function* bounceFunction);  
			   
private:
  /* invariant: the value in TargetsMap's entries is sorted */  
  using TargetsMap = llvm::DenseMap<llvm::Instruction*, AliasSet>;
  using BounceMap = std::multimap<AliasSetId, std::pair<const AliasSet*, llvm::Function *>>;
  // -- the module
  llvm::Module& m_M;
  // -- the pointer analysis to resolve function pointers
  Dsa& m_dsa;
  // -- Resolve incomplete nodes (unsound, in general)
  bool m_allow_incomplete;
  // -- Maximum number of targets (used to avoid having too large
  // -- bounce functions). if equal to 0 then unlimited.
  unsigned m_max_num_targets;
  // -- map from callsite to the corresponding alias set
  TargetsMap m_targets_map;  
  // -- map from alias set id + dsa targets to an existing bounce function
  BounceMap m_bounce_map;  
};

//
// Class: DevirtualizeFunctions
//
//  This transform pass will look for indirect function calls and
//  transform them into a switch statement that selects one of
//  several direct function calls to execute. This transformation
//  pass is parametric on the method used to resolve the call.
//
class DevirtualizeFunctions : public llvm::InstVisitor<DevirtualizeFunctions> {

private:
  using AliasSet = CallSiteResolver::AliasSet;
  using AliasSetId = devirt_impl::AliasSetId;

  // Call graph of the program
  //llvm::CallGraph *m_cg;
  // allow creating of indirect calls during devirtualization
  // (required for soundness)
  bool m_allowIndirectCalls;
  // Worklist of call sites to transform
  llvm::SmallVector<llvm::Instruction *, 32> m_worklist;
  // For stats
  DevirtStats m_stats;
  
  /// turn the indirect call-site into a direct one
  void mkDirectCall(llvm::CallSite CS, CallSiteResolver *CSR);

  /// create a bounce function that calls functions directly
  llvm::Function *mkBounceFn(llvm::CallSite &CS, CallSiteResolver *CSR);

public:
  DevirtualizeFunctions(llvm::CallGraph *cg, bool allowIndirectCalls);

  ~DevirtualizeFunctions();

  DevirtStats& getStats() { return m_stats;}

  const DevirtStats& getStats() const { return m_stats; }
  
  // Resolve all indirect calls in the Module using a particular
  // callsite resolver.
  bool resolveCallSites(llvm::Module &M, CallSiteResolver *CSR);
  // -- VISITOR IMPLEMENTATION --

  void visitCallSite(llvm::CallSite &CS);
  void visitCallInst(llvm::CallInst &CI);
  void visitInvokeInst(llvm::InvokeInst &II);
};
} // end namespace clam
  
