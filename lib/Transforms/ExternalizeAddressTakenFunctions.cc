/* Externalize uses of functions whose address have been taken */
#include "llvm/Pass.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

namespace clam {

  class ExternalizeAddressTakenFunctions : public ModulePass {
   public:
    
    static char ID;
    
    ExternalizeAddressTakenFunctions (): ModulePass (ID) {}
    
    virtual bool runOnModule (Module &M) {
      bool Changed = false;
      for (auto &F: M) {
        
        // skip functions without a body
        if (F.isDeclaration () || F.empty ()) continue;
        
        if (F.hasAddressTaken ()) {
          // create function type
          const FunctionType *FTy = F.getFunctionType ();
          std::vector<llvm::Type*> ParamsTy (FTy->param_begin (), FTy->param_end ());
          Type *RetTy = F.getReturnType ();
          FunctionType *NFTy = FunctionType::get (RetTy, 
                                                  ArrayRef<llvm::Type*> (ParamsTy), 
                                                  FTy->isVarArg ());
          // create new function 
          Function *NF = Function::Create (NFTy, 
                                           GlobalValue::ExternalLinkage, 
                                           F.getName () + ".stub");
          NF->copyAttributesFrom(&F);
          F.getParent ()->getFunctionList ().insert(F.getIterator(), NF);       
          
          // replace each use &foo with &foo_stub() where foo_stub is a
          // copy of foo but marked as external.
	  Value::use_iterator UI = F.use_begin(), E = F.use_end();
          for (; UI != E;) {
	    Use *U = &(*UI);
            ++UI;
            User *FU = U->getUser();
            if (isa<BlockAddress>(FU))
              continue;
            
            if (isa<CallInst>(FU) || isa<InvokeInst>(FU)) {
              ImmutableCallSite CS(dyn_cast<Instruction>(FU)); 
             if (!CS.isCallee (U)) {
               U->set (NF);
               Changed=true;
             }
            } else {
              if (GlobalAlias *a = dyn_cast<GlobalAlias> (FU)) {
                a->setAliasee (NF);
	  	Changed = true;
	      } else if (Constant *c = dyn_cast<Constant> (FU)) {
		if (isa<GlobalVariable>(c) &&
		    (c->getNumOperands () == 1 && c->getOperand(0) == &F && isa<Constant>(NF))) {
		  // -- handleOperandChange cannot handle this case:
		  //    @ldv_0_callback_handler = internal global i32 (i32, i8*)* @alx_intr_legacy
		  //    where we want to replace @alx_intr_legacy with
		  //    @alx_intr_legacy_stub
		  c->setOperand(0, cast<Constant>(NF));
		  Changed = true;
		} else  if (isa<ConstantArray>(c)  || isa<ConstantStruct>(c) ||
			    isa<ConstantVector>(c) || isa<ConstantExpr>(c)) {
		  // -- handleOperandChange can only handle these four
		  //    kind of constants.
		  
		  // In cases like this one:
		  // 
		  // @x = internal constant %struct.dev_pm_ops { i32
		  //      (%struct.device*)* @alx_suspend, i32
		  //      (%struct.device*)* @alx_suspend, ... } 
		  //
		  // even if U is the first occurrence of alx_suspend
		  // it seems that handleOperandChange will also
		  // replace the second occurrence. We need to skip
		  // uses for which the replacement took place already
		  // during the processing of another use.
		  Use *OperandList = c->getOperandList();
		  unsigned OperandToUpdate = U-OperandList;
		  if (c->getOperand(OperandToUpdate) == &F) {
		    c->handleOperandChange (&F, NF);
		    Changed = true;
		  }
		}
	      } else {
	  	U->set (NF);
	  	Changed = true;		
	      }
            }
          }
        }
      }
      return Changed;
    }
    void getAnalysisUsage (AnalysisUsage &AU) const {
      //AU.setPreservesAll ();
    }

    StringRef getPassName () const {
      return "Clam: Externalize uses of address-taken functions";
    }
  };

   char ExternalizeAddressTakenFunctions::ID = 0;

   Pass* createExternalizeAddressTakenFunctionsPass () {
     return new ExternalizeAddressTakenFunctions ();
   }
   
} // end namespace   
      


