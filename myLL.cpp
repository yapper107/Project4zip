#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Type.h"

using namespace llvm;

namespace {
  struct MemoryOverflowPass : public FunctionPass {
    static char ID;

    MemoryOverflowPass() : FunctionPass(ID) {}

    bool runOnFunction(Function &F) override {
      for (auto &BB : F) {
        for (auto &Inst : BB) {
          if (auto *Call = dyn_cast<CallInst>(&Inst)) {
            Function *Callee = Call->getCalledFunction();
            if (Callee && Callee->getName() == "memset") {
              Value *Dest = Call->getArgOperand(0);
              Value *Size = Call->getArgOperand(1);
              if (auto *GV = dyn_cast<GlobalVariable>(Dest)) {
                Type *GVType = GV->getType()->getPointerElementType();
                uint64_t GVSize = GVType->getPrimitiveSizeInBits() / 8;
                ConstantInt *SizeCI = dyn_cast<ConstantInt>(Size);
                if (SizeCI && SizeCI->getZExtValue() > GVSize) {
                  errs() << "Potential memory overflow detected at line "
                         << Inst.getDebugLoc().getLine() << "\n";
                }
              }
            }
          }
        }
      }
      return false;
    }
  };
}

char MemoryOverflowPass::ID = 0;

static RegisterPass<MemoryOverflowPass> X("memory-overflow", "Memory Overflow Vulnerability Pass");
