#include <iostream>

#include "llvm/Support/Error.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Support/TargetSelect.h"

#include "llvm/ExecutionEngine/Orc/LLJIT.h"

#include "EXTZONES.h"

using namespace llvm;
using namespace llvm::orc;

static struct LLVMStuff {
  LLVMStuff() :
    TheJIT(nullptr),
    TheContext(nullptr),
    TheModule(nullptr),
    TheFPM(nullptr),
    Builder(nullptr),
    NamedValues()
  {}

  void reset() {
    // Create a new context
    TheContext = std::make_unique<LLVMContext>();
    // Open a new module
    TheModule = std::make_unique<Module>("my cool jit", *TheContext);
    TheModule->setDataLayout(TheJIT->getDataLayout());

    Builder = std::make_unique<IRBuilder<>>(*TheContext);

    // Create a new pass manager attached to it.
    TheFPM = std::make_unique<legacy::FunctionPassManager>(TheModule.get());

    // Do simple "peephole" optimizations and bit-twiddling optzns.
    TheFPM->add(createInstructionCombiningPass());
    // Reassociate expressions.
    TheFPM->add(createReassociatePass());
    // Eliminate Common Subexpressions
    TheFPM->add(createGVNPass());
    // Simplify the control flow graph (deleting unreachable blocks, etc)
    TheFPM->add(createCFGSimplificationPass());

    TheFPM->doInitialization();
  }

  void init() {
    TheJIT = std::move(LLJITBuilder().create().get());
    // reset handles TheContext, TheModule, Builder, and TheFPM
    reset();
  }

  // returns true on error
  bool crystallize() {
    auto Err = TheJIT->addIRModule(ThreadSafeModule(std::move(TheModule), std::move(TheContext)));
    cantFail(std::move(Err), "yep crystallize definitely cannot fail");
    reset();
    return static_cast<bool>(Err);
  }

  std::unique_ptr<LLJIT> TheJIT;
  std::unique_ptr<LLVMContext> TheContext;
  std::unique_ptr<Module> TheModule;
  std::unique_ptr<legacy::FunctionPassManager> TheFPM;
  std::unique_ptr<IRBuilder<>> Builder;
  std::map<std::string, Value *> NamedValues;
} G;

int main() {
  // read in all our IR:
  {
    SMDiagnostic diag;
    G.TheModule = std::move(parseIRFile("ir/one.ll", diag, *G.TheContext));
    G.TheModule->setDataLayout(G.TheJIT->getDataLayout());
    assert(!G.crystallize());
  }
  {
    SMDiagnostic diag;
    G.TheModule = std::move(parseIRFile("ir/two.ll", diag, *G.TheContext));
    G.TheModule->setDataLayout(G.TheJIT->getDataLayout());
    assert(!G.crystallize());
  }
  {
    SMDiagnostic diag;
    G.TheModule = std::move(parseIRFile("ir/three.ll", diag, *G.TheContext));
    G.TheModule->setDataLayout(G.TheJIT->getDataLayout());
    assert(!G.crystallize());
  }

  // add llvm_zone_malloc to the main symbol table
  {
    const DataLayout& DL = G.TheJIT->getDataLayout();
    MangleAndInterner Mangle(G.TheJIT->getExecutionSession(), DL);

    auto syms =
        absoluteSymbols({{Mangle("llvm_zone_malloc"),
                          JITEvaluatedSymbol(pointerToJITTargetAddress(&extemp::EXTZONES::llvm_zone_malloc), {})}});
    cantFail(G.TheJIT->getMainJITDylib().define(syms), "defining llvm_zone_malloc");
  }

  {
    SMDiagnostic diag;
    G.TheModule = std::move(parseIRFile("ir/four.ll", diag, *G.TheContext));
    G.TheModule->setDataLayout(G.TheJIT->getDataLayout());
    assert(!G.crystallize());
  }

  {
    SMDiagnostic diag;
    G.TheModule = std::move(parseIRFile("ir/five.ll", diag, *G.TheContext));
    G.TheModule->setDataLayout(G.TheJIT->getDataLayout());
    assert(!G.crystallize());
  }

  // try run add1_adhoc_W2k2NCxpNjRd_maker(void* something_idk)
  {
    auto sym = cantFail(G.TheJIT->lookup("add1_adhoc_W2k2NCxpNjRd_maker"), "lookup add1_adhoc_X_maker");
    void (*f)(llvm_zone_t*) = (void (*)(llvm_zone_t*))sym.getAddress();
    f(static_cast<llvm_zone_t*>(nullptr));
  }

  // fprintf(stderr, "ready> ");
  // getNextToken();

  // MainLoop();

  G.TheJIT->getExecutionSession().dump(errs());
  
  std::cout << "Hello, world!" << std::endl;
  return 0;
}
