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
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include <bits/stdc++.h>
#include <map>
#include <vector>
using namespace llvm;
using namespace std;

#define DEBUG_TYPE "demo"

enum lattice_symbols { TOP = -2, BOTTOM };

STATISTIC(HelloCounter, "Counts number of functions greeted");
struct variable_values {
  bool TOP;
  bool BOTTOM;
  APInt constant_value;

  variable_values() : TOP(false), BOTTOM(false) {}
};

namespace {
// Hello - The first implementation, without getAnalysisUsage.
struct Demo : public FunctionPass {
  static char ID; // Pass identification, replacement for typeid
  Demo() : FunctionPass(ID) {}
  map<StringRef, variable_values> variable_state;
  queue<BasicBlock *> BB_to_execute; // This queue stores the address of the BBs
                                     // in the order they will be executed
  // std::map<std::string, int> VariableMap;

  bool runOnFunction(Function &F) override {
    // int number_of_ins = 0;

    ++HelloCounter;
    // errs() << "Demo: ";
    // errs().write_escaped(F.getName()) << '\n';
    // for (BasicBlock &BB : F) {
    // errs() <<"\n";
    // errs() <<"\n";
    // errs() << "*** New Basic Block being executed ***"<<"\n";
    BasicBlock *startBB = &F.getEntryBlock();
    BB_to_execute.push(startBB);

    // errs() << "Successors of BasicBlock: " << startBB->getName() << "\n";
    // for (BasicBlock *Succ : successors(startBB)) {
    //     errs() << "  " << Succ->getName() << "\n";
    //     BB_to_execute.push(Succ);
    //     errs() <<" BB from the queue
    //     "<<BB_to_execute.back()->getName()<<"\n";
    // }

    while (!BB_to_execute.empty()) {
      BasicBlock *front_BB = BB_to_execute.front();
      BB_to_execute.pop();

      for (BasicBlock *Succ : successors(front_BB)) {
        // errs() <<"Successors of this "<< front_BB->getName() <<" is : "<<
        // Succ->getName()<<"\n";
        BB_to_execute.push(Succ);
      }
      int i = 0;

      for (Instruction &I : *front_BB) {
        // errs() << "Opcode of the instruction is : " << I.getOpcodeName()<< "
        // "; errs() << "Variable name : "<< I.getName()<<"\n"; // This gets the
        // variable name
        //errs() << "Opcode of the instruction is : "<<I.getOpcodeName()<<"\n";

        // errs()<<I<<"\n";
        // errs() <<"Instr : "<<i<<" Opcode : "<<I.getOpcodeName()<<"\n";
        i++;

        /* Processing Store Instructions */
        if (StoreInst *Store = dyn_cast<StoreInst>(&I)) {
          Value *StoredValue = Store->getValueOperand(); // gets the Operand value
          auto *Var = dyn_cast<AllocaInst>(Store->getPointerOperand()); // this fetches the address where the
                                           // variable has been stored
          // errs()<<"Variable in which the store will happen:
          // "<<Var->getName()<<"\n"; // Here, we get the name of the variable
          // from the address fetched above

          if (ConstantInt *ConstantValue = dyn_cast<ConstantInt>(StoredValue)) { // Initially, the operand value was in hex,
                                  // but we need to change in integer
            // errs() << "Constant value in store instruction: "<<
            // ConstantValue->getValue() << "\n"; // This gives the constant
            // Integer value

            /* Here, we store the variable and the value assigned to it*/
            variable_state[Var->getName()].constant_value = ConstantValue->getValue();
            variable_state[Var->getName()].TOP = false;
            variable_state[Var->getName()].BOTTOM = false;

            // errs() <<"Key : "<< Var->getName() <<" Value : "<<
            // variable_state[Var->getName()].constant_value<<"\n";
            errs() << I << " --> %" << Var->getName() << " = " << variable_state[Var->getName()].constant_value << "\n";
          }
          else 
          {
            errs() << I << " --> %" << Var->getName() << " = " << variable_state[StoredValue->getName()].constant_value << "\n";

           // errs() <<"Name of the non-constant variable : "<<Var->getName()<<"\n";
           //  errs() <<"Name in which the storage will be done : "<<StoredValue->getName()<<"\n";
            //variable_state[Var->getName()] = variable_state[StoredValue->getName()];
            auto it = variable_state.find(Var->getName());
             if(it !=variable_state.end())
             {
            //  errs()<< "This key exists in the map"<<"\n";
             }

             if (variable_state[Var->getName()].TOP == true)
             {
             // errs()<< "Intially the value was TOP "<<"\n";
              variable_state[Var->getName()] = variable_state[StoredValue->getName()]; // x meet top = x
             }
             else if(variable_state[Var->getName()].BOTTOM == true)
             {
             // errs()<< " Intially the value was BOTTOM, so no changes is needed  "<<"\n"; // x meet bottom = x
             }
             else if(variable_state[Var->getName()].constant_value == variable_state[StoredValue->getName()].constant_value)
             {
             // errs()<<"Same constant value was there"<<"\n";
             }
             else if(variable_state[Var->getName()].constant_value != variable_state[StoredValue->getName()].constant_value)
             {
             // errs()<<"Since the constant value is not same: "<<"\n";
              variable_state[Var->getName()].BOTTOM = true;
              variable_state[Var->getName()].TOP = false;

             }
             else
             {
             // errs()<<"This is not handled yet"<<"\n";
             }

            //errs() << I << " --> %" << Var->getName() << " = "
                     // << variable_state[Var->getName()].constant_value << "\n";

            

          }
        }
        /* Processing Alloca Instructions */
        else if (AllocaInst *Alloca = dyn_cast<AllocaInst>(&I)) {
          // errs()<<"This is the Variable on RHS : "<<Alloca->getName()<<"\n";
          variable_state[Alloca->getName()].TOP = true;

          // errs() <<"Key : "<< Alloca->getName() <<" Value : "<<
          // variable_state[Alloca->getName()].TOP<<"\n";

          errs() << I << " --> %" << Alloca->getName() << " = TOP"
                 << "\n";

        }

        /* Processing Load instructions */

        else if (LoadInst *Load = dyn_cast<LoadInst>(&I)) {
          Value *LoadedValue = Load->getPointerOperand();
          auto *Var = dyn_cast<AllocaInst>(LoadedValue);
          // Value *ResultValue = &I;

          // errs() << "Variable on which load will happen: " <<I.getName()<<
          // "\n"; errs() << "Loaded Value: " << LoadedValue->getName() << "\n";
          Value *destination = &I;
          Value *ptrOperand = Load->getPointerOperand();
          auto *ptrArg = dyn_cast<Argument>(ptrOperand);
          // errs() << "Load operation on variable in function : "
          // <<I.getNameOrAsOperand() << "\n"; errs() << "Loaded Value: " <<
          // LoadedValue->getName() << "\n";

          variable_state[I.getNameOrAsOperand()] =
              variable_state[LoadedValue->getName()];
          if (variable_state[I.getNameOrAsOperand()].TOP) {
            errs() << I << " --> " << I.getNameOrAsOperand() << " = TOP"
                   << "\n";
          } else if (variable_state[I.getNameOrAsOperand()].BOTTOM) {
            errs() << I << " --> " << I.getNameOrAsOperand() << " = BOTTOM"
                   << "\n";
          } else {
            errs() << I << " --> " << I.getNameOrAsOperand() << " = "
                   << variable_state[LoadedValue->getName()].constant_value
                   << "\n";
          }

        }

        /* Processing the Call Instructions */
        else if (CallInst *Call = dyn_cast<CallInst>(&I)) {
          /* If the function is scanf, then we assign value as bottom to the
           * variable passed to it*/
          /* If the function is printf, then we ignore it */
          Function *calledFunction = Call->getCalledFunction();
          // errs()<< "Name of the function is : "<<
          // calledFunction->getName()<<"\n";
          if (calledFunction->getName() == "__isoc99_scanf") {
            // errs()<<"Variable on which scanf was called :
            // "<<(Call->getArgOperand(1))->getName()<<"\n";  // Here,
            // Call->getArgOperand(1) will return the address of the 1st arg of
            // the Scanf function, then we get the name of that address using
            // "getName()" function
            variable_state[(Call->getArgOperand(1))->getName()].TOP = true;
            // errs() << "Key : "<< (Call->getArgOperand(1))->getName() << "
            // Value :
            // "<<variable_state[(Call->getArgOperand(1))->getName()].TOP
            // <<"\n";
            errs() << I << " --> %" << (Call->getArgOperand(1))->getName()
                   << " = TOP"
                   << "\n";
          } else if (calledFunction->getName() == "printf") {
            errs() << I << " --> "
                   << "No information"
                   << "\n";
          }
        } else if (ReturnInst *RetIns = dyn_cast<ReturnInst>(&I)) {
          errs() << I << " --> Return instruction"
                 << "\n";
        } else if (auto *binaryOp = dyn_cast<BinaryOperator>(&I))
        {
          if (binaryOp->getOpcode() == Instruction::Add)
          {
           // errs() <<"Variable were the value of the addition will be stored is : "<<I.getName()<<"\n";
            
              //errs() << "Operand " << i << ": " << (binaryOp->getOperandUse(0))->getNameOrAsOperand()<< "\n";
              //errs() << "Operand " << i << ": " << (binaryOp->getOperandUse(1))->getNameOrAsOperand()<< "\n";
              //errs() << "Operand " << i << ": " << (binaryOp->getOperandUse(1))->get<< "\n";

              //if (ConstantInt *ConstantValue = dyn_cast<ConstantInt>(StoredValue))
              
            if (ConstantInt *op1 = dyn_cast<ConstantInt>(binaryOp->getOperand(1)))
            {
             //errs() <<" value is : "<<  op1->getValue()<<"\n";
             APInt result = op1->getValue() + variable_state[(binaryOp->getOperandUse(0))->getNameOrAsOperand()].constant_value;
             //errs() << "Result that we got in addition is : "<< result<<"\n";

             variable_state[I.getName()].constant_value = result;
             variable_state[I.getName()].TOP = false;
             variable_state[I.getName()].BOTTOM = false;

             //errs() <<"Just for verifying>> Key : "<<variable_state["add"].constant_value<<"\n";
             errs() <<I <<" --> %"<<I.getName()<<" = "<< variable_state[I.getName()].constant_value<<"\n";
            }
          }
          else if(binaryOp->getOpcode() == Instruction::Sub)
          {
             // errs() << "Operand " << i << ": " << (binaryOp->getOperandUse(0))->getNameOrAsOperand()<< "\n";
             // errs() << "Operand " << i << ": " << (binaryOp->getOperandUse(1))->getNameOrAsOperand()<< "\n";
              //errs() << "Operand " << i << ": " << (binaryOp->getOperandUse(1))->get<< "\n";

              //if (ConstantInt *ConstantValue = dyn_cast<ConstantInt>(StoredValue))
              
            if (ConstantInt *op1 = dyn_cast<ConstantInt>(binaryOp->getOperand(1)))
            {
             //errs() <<" value is : "<<  op1->getValue()<<"\n";
             APInt result = variable_state[(binaryOp->getOperandUse(0))->getNameOrAsOperand()].constant_value - op1->getValue();
             //errs() << "Result that we got in subtraction is : "<< result<<"\n";

             variable_state[I.getName()].constant_value = result;
             variable_state[I.getName()].TOP = false;
             variable_state[I.getName()].BOTTOM = false;

             errs() <<I <<" --> %"<<I.getName()<<" = "<< variable_state[I.getName()].constant_value<<"\n";

            }
          }
        }
        else if(CmpInst *cmpinst = dyn_cast<CmpInst>(&I))
        {
        //  errs()<<"Name of the variable is : "<< I.getName()<<"\n";
          variable_state[I.getName()].BOTTOM = true;
          errs()<<I<<" --> %"<<I.getName() <<" = BOTTOM"<<"\n";

          
        }
        else if(BranchInst *BrInst = dyn_cast<BranchInst>(&I))
        {
          if(BrInst->isConditional())
          {
          //errs() <<"name is "<<I.getName()<<"\n";
          //errs()<<"branch will be done on : "<<(BrInst->getOperand(0))->getName()<<" and "<<(BrInst->getOperand(1))->getName()<<"\n";
          if(variable_state[(BrInst->getOperand(0))->getName()].BOTTOM)
          {
            errs() << I << " --> %" << (BrInst->getOperand(0))->getName()<<" = BOTTOM"<<"\n";
          }
          else if(variable_state[(BrInst->getOperand(0))->getName()].TOP)
          {
          errs() << I << " --> %" << (BrInst->getOperand(0))->getName()<<" = TOP"<<"\n";
          }
          else
          {
          errs() << I << " --> %" << (BrInst->getOperand(0))->getName()<<" = "<<variable_state[(BrInst->getOperand(0))->getName()].constant_value<<"\n";

          }   
        }
        else
        {
          errs()<<I<<"\n";
        }
        }
      }
    }

    return false;
  }
};
} // namespace

char Demo::ID = 0;
static RegisterPass<Demo> X("demo", "This is a Demo pass");
