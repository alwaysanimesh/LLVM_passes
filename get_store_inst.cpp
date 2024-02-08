//===- Hello.cpp - Example code from "Writing an LLVM Pass" ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements two versions of the LLVM "Hello World" pass described
// in docs/WritingAnLLVMPass.html
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include <vector>
using namespace llvm;

#define DEBUG_TYPE "demo"

STATISTIC(HelloCounter, "Counts number of functions greeted");

namespace {
  // Hello - The first implementation, without getAnalysisUsage.
  struct Demo : public FunctionPass {
    static char ID; // Pass identification, replacement for typeid
    Demo() : FunctionPass(ID) {}

    bool runOnFunction(Function &F) override {
     // int number_of_ins = 0;
 
      ++HelloCounter;
      errs() << "Demo: ";
      errs().write_escaped(F.getName()) << '\n';
      for(BasicBlock &BB : F)
      {
        for(Instruction &I : BB)
        {
          errs()<<" opcode of the instruction is : "<< I.getOpcodeName()<<"\n";  //This prints Opcode of all the instructions
          if(I.getOpcode() == Instruction::Store)                    // This compares if the Opcode is "Store" then it prints the stmt
          {
            errs()<<"Ye kay hai"<<"\n";
          }
           
        }
        //errs()<<"Number of Instructions are : "<< number_of_ins<< '\n';
      
      }
      
      return false;
    }
  };
}

char Demo::ID = 0;
static RegisterPass<Demo> X("demo", "This is a Demo pass");
