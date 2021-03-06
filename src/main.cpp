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
#include "EXTClosureAddressTable.h"
#include "EXTLLVM.h"

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

  void dump() {
    TheJIT->getExecutionSession().dump(errs());
  }

  std::unique_ptr<LLJIT> TheJIT;
  std::unique_ptr<LLVMContext> TheContext;
  std::unique_ptr<Module> TheModule;
  std::unique_ptr<legacy::FunctionPassManager> TheFPM;
  std::unique_ptr<IRBuilder<>> Builder;
  std::map<std::string, Value *> NamedValues;
} G;

int main() {
  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();
  InitializeNativeTargetAsmParser();
  G.init();
  // read in all our IR:
  {
    SMDiagnostic diag;
    G.TheModule = std::move(parseIRFile("ir/one.ll", diag, *G.TheContext));
    G.TheModule->setDataLayout(G.TheJIT->getDataLayout());
    assert(!G.crystallize());
    G.dump();
  }
  {
    SMDiagnostic diag;
    G.TheModule = std::move(parseIRFile("ir/two.ll", diag, *G.TheContext));
    G.TheModule->setDataLayout(G.TheJIT->getDataLayout());
    assert(!G.crystallize());
    G.dump();
  }
  {
    SMDiagnostic diag;
    G.TheModule = std::move(parseIRFile("ir/three.ll", diag, *G.TheContext));
    G.TheModule->setDataLayout(G.TheJIT->getDataLayout());
    assert(!G.crystallize());
    G.dump();
  }

  // add syms to the main symbol table
  {
    const DataLayout& DL = G.TheJIT->getDataLayout();
    MangleAndInterner Mangle(G.TheJIT->getExecutionSession(), DL);

    auto syms = absoluteSymbols(
        {{Mangle("llvm_zone_malloc"),
          JITEvaluatedSymbol(
              pointerToJITTargetAddress(&extemp::EXTZONES::llvm_zone_malloc),
              {})},
         {Mangle("llvm_zone_mark"),
          JITEvaluatedSymbol(
              pointerToJITTargetAddress(&extemp::EXTZONES::llvm_zone_mark),
              {})},
         {Mangle("new_address_table"),
          JITEvaluatedSymbol(
              pointerToJITTargetAddress(
                  &extemp::ClosureAddressTable::new_address_table),
              {})},
         {Mangle("add_address_table"),
          JITEvaluatedSymbol(
              pointerToJITTargetAddress(
                  &extemp::ClosureAddressTable::add_address_table),
              {})},
         {Mangle("llvm_zone_mark_size"),
          JITEvaluatedSymbol(
              pointerToJITTargetAddress(&extemp::EXTZONES::llvm_zone_mark_size),
              {})},
         {Mangle("llvm_zone_ptr_set_size"),
          JITEvaluatedSymbol(pointerToJITTargetAddress(
                                 &extemp::EXTZONES::llvm_zone_ptr_set_size),
                             {})},
         {Mangle("llvm_destroy_zone_after_delay"),
          JITEvaluatedSymbol(
              pointerToJITTargetAddress(
                  &extemp::EXTLLVM::llvm_destroy_zone_after_delay),
              {})},
         {Mangle("llvm_peek_zone_stack"),
          JITEvaluatedSymbol(pointerToJITTargetAddress(
                                 &extemp::EXTZONES::llvm_peek_zone_stack),
                             {})},
         {Mangle("llvm_pop_zone_stack"),
          JITEvaluatedSymbol(pointerToJITTargetAddress(
                                 &extemp::EXTZONES::llvm_pop_zone_stack),
                             {})},
         {Mangle("llvm_push_zone_stack"),
          JITEvaluatedSymbol(pointerToJITTargetAddress(
                                 &extemp::EXTZONES::llvm_push_zone_stack),
                             {})},
         {Mangle("llvm_zone_destroy"),
          JITEvaluatedSymbol(pointerToJITTargetAddress(
                                 &extemp::EXTZONES::llvm_zone_destroy),
                             {})}
	});

    cantFail(G.TheJIT->getMainJITDylib().define(syms),
             "defining llvm_zone_malloc");

    G.dump();
  }

  {
    SMDiagnostic diag;
    G.TheModule = std::move(parseIRFile("ir/four.ll", diag, *G.TheContext));
    G.TheModule->setDataLayout(G.TheJIT->getDataLayout());
    assert(!G.crystallize());
    G.dump();
  }

  {
    SMDiagnostic diag;
    G.TheModule = std::move(parseIRFile("ir/five.ll", diag, *G.TheContext));
    diag.print("five.ll", errs());
    G.TheModule->setDataLayout(G.TheJIT->getDataLayout());
    assert(!G.crystallize());
    G.dump();
  }

  // try run add1_adhoc_W2k2NCxpNjRd_maker(void* something_idk)
  {
    auto sym = cantFail(G.TheJIT->lookup("add1_adhoc_W2k2NCxpNjRd_maker"), "lookup add1_adhoc_X_maker");
    void (*f)(llvm_zone_t*) = (void (*)(llvm_zone_t*))sym.getAddress();
    llvm_zone_t* z = extemp::EXTZONES::llvm_zone_create(50 * 1024 * 1024);
    f(z);
  }

  {
    SMDiagnostic diag;
    G.TheModule = std::move(parseIRFile("ir/six.ll", diag, *G.TheContext));
    diag.print("six.ll", errs());
    G.TheModule->setDataLayout(G.TheJIT->getDataLayout());
    assert(!G.crystallize());
    G.dump();
  }

  {
    MangleAndInterner Mangle(G.TheJIT->getExecutionSession(), G.TheJIT->getDataLayout());
    auto& main = G.TheJIT->getMainJITDylib();
    auto& ES = G.TheJIT->getExecutionSession();
    cantFail(main.remove({Mangle("add1_adhoc_W2k2NCxpNjRd_maker"), Mangle("add1_adhoc_W2k2NCxpNjRd_setter")}), "removing _maker and _setter");
    G.dump();
  }

  {
    SMDiagnostic diag;
    G.TheModule = std::move(parseIRFile("ir/seven.ll", diag, *G.TheContext));
    diag.print("seven.ll", errs());
    G.TheModule->setDataLayout(G.TheJIT->getDataLayout());
    assert(!G.crystallize());
    G.dump();
  }

  {
    auto sym = cantFail(G.TheJIT->lookup("add1_adhoc_W2k2NCxpNjRd_maker"), "lookup add1_adhoc_X_maker");
    void (*f)(llvm_zone_t*) = (void (*)(llvm_zone_t*))sym.getAddress();
    llvm_zone_t* z = extemp::EXTZONES::llvm_zone_create(50 * 1024 * 1024);
    f(z);
  }

  G.dump();
  
  return 0;
}
