#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

using namespace llvm;

namespace {
struct Hello : public FunctionPass {
  static char ID;
  Hello() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override {
    Module *M = F.getParent();
    // TODO: figure out how to not need to hardcode the function.
    Function *func = M->getFunction("exp");
    LLVMContext & context = M->getContext();
    Type *intType = Type::getInt32Ty(context);
    // Declare C standard library printf 
    std::vector<Type *> printfArgsTypes({Type::getInt8PtrTy(context)});
    FunctionType *printfType = FunctionType::get(intType, printfArgsTypes, true);
    auto print = M->getOrInsertFunction("printf", printfType);
    if (!func) {
      errs() << "Error\n";
    }

    for (BasicBlock &BB : F) {
      for (Instruction &I : BB) {
        if (CallInst *call = dyn_cast<CallInst>(&I)) {
          if (call->getCalledFunction() == func) {
            Value *x = call->getArgOperand(0);
            Value *y = dyn_cast<Value>(call);
            IRBuilder<> builder(call);
            builder.SetInsertPoint(call);
            builder.CreateCall(print, {builder.CreateGlobalStringPtr("%f "), x});
            builder.SetInsertPoint(call->getNextNonDebugInstruction());
            builder.CreateCall(print, {builder.CreateGlobalStringPtr("%f "), y});

          }
        }
      }
    }
    // For debug purposes only.
    for (BasicBlock &BB : F) {
      errs() << "===============================\n";
      for (Instruction &I : BB) {
        errs() << I << "\n";
      }
      errs() << "===============================\n";
    }
    return true;
  }
}; // end of struct Hello
}  // end of anonymous namespace

char Hello::ID = 0;
static RegisterPass<Hello> X("hello", "Hello World Pass",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);