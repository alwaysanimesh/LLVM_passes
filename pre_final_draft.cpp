

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include <bits/stdc++.h>
#include <map>
#include "llvm/Support/CommandLine.h"
#include <vector>
using namespace llvm;
using namespace std;

#define DEBUG_TYPE "demo"

struct variable_values {
  bool TOP;
  bool BOTTOM;
  //bool CONSTANT;
  APInt constant_value;

  variable_values() : TOP(false), BOTTOM(false) {}
};


namespace {
struct Demo : public FunctionPass {
  static char ID; 
  Demo() : FunctionPass(ID) {}
  map<StringRef, variable_values> variable_state; // This Map contains the name of the Variable and the Value contained in it
  queue<BasicBlock *> BB_to_execute;   // This queue is used for maintaining the execution order of the Basic Blocks

  bool runOnFunction(Function &F) override {
    /* We Push the entry Basic Block first */
    BasicBlock *startBB = &F.getEntryBlock();
    BB_to_execute.push(startBB);

    
    while (!BB_to_execute.empty()) {
      BasicBlock *front_BB = BB_to_execute.front();  // We get the address of the Basic Block to run the evaluation on
      BB_to_execute.pop();     // After fetching the address, we remove the Basic Block from the queue
      
      /* We then push all the successor Basic Blocks of the current Basic Block */
      for (BasicBlock *Succ : successors(front_BB)) {
        if(Succ != BB_to_execute.back())
        {
            BB_to_execute.push(Succ);
        }
      }

      /* We execute all the instructions in the current Basic Block sequentially */
      for (Instruction &I : *front_BB) {

        /* Processing Store Instructions */
        if (StoreInst *Store = dyn_cast<StoreInst>(&I)) {
          Value *StoredValue = Store->getValueOperand(); // gets the Operand value
          auto *Var = dyn_cast<AllocaInst>(Store->getPointerOperand());
          
          /* If the Store operation contains a Constant value then this code "if" will be executed */
          if (ConstantInt *ConstantValue = dyn_cast<ConstantInt>(StoredValue)) { 
                                  
            /* Here, we store the variable and the value assigned to it*/
            variable_state[Var->getName()].constant_value = ConstantValue->getValue();
            variable_state[Var->getName()].TOP = false;
            variable_state[Var->getName()].BOTTOM = false;
            //variable_state[Var->getName()].CONSTANT = true;


            errs() << I << " --> %" << Var->getName() << " = " << variable_state[Var->getName()].constant_value << "\n";
          }
          else 
          {
            errs() << I << " --> %" << Var->getName() << " = " << variable_state[StoredValue->getName()].constant_value << "\n";
            

             /* Below code takes care of the different comnination that we can get by using TOP, BOTTOM and CONSTANT */
             if (variable_state[Var->getName()].TOP == true)
             {
              variable_state[Var->getName()] = variable_state[StoredValue->getName()]; // x meet top = x
             }
             else if(variable_state[Var->getName()].BOTTOM == true)  // No changes will be done her
             {
             }
             else if(variable_state[Var->getName()].constant_value == variable_state[StoredValue->getName()].constant_value)  // C1 meet C1 = C1, so no changes
             {
             }
             else if(variable_state[Var->getName()].constant_value != variable_state[StoredValue->getName()].constant_value)  // C1 meet C2 = BOTTOM
             {
              variable_state[Var->getName()].BOTTOM = true;
              variable_state[Var->getName()].TOP = false;
              //variable_state[Var->getName()].CONSTANT = false;

             }
          }
        }

        /* Processing Alloca Instructions */
        else if (AllocaInst *Alloca = dyn_cast<AllocaInst>(&I)) {
          variable_state[Alloca->getName()].TOP = true;    // For all alloca variables, we intialize it to TOP
          errs() << I << " --> %" << Alloca->getName() << " = TOP"<< "\n";
        }

        /* Processing Load instructions */
        else if (LoadInst *Load = dyn_cast<LoadInst>(&I)) {
          Value *LoadedValue = Load->getPointerOperand();
          auto *Var = dyn_cast<AllocaInst>(LoadedValue);
          Value *destination = &I;
          Value *ptrOperand = Load->getPointerOperand();
         
          variable_state[I.getNameOrAsOperand()] = variable_state[LoadedValue->getName()];  // Here, we copy the value of the source variable to destination variable
          if (variable_state[I.getNameOrAsOperand()].TOP) {
            errs() << I << " --> " << I.getNameOrAsOperand() << " = TOP, "<<LoadedValue->getName()<<" = TOP"<<"\n";
          } else if (variable_state[I.getNameOrAsOperand()].BOTTOM) {
            errs() << I << " --> " << I.getNameOrAsOperand() << " = BOTTOM, " <<LoadedValue->getName()<<" = BOTTOM"<< "\n";
          } else {
            errs() << I << " --> " << I.getNameOrAsOperand() << " = "<< variable_state[LoadedValue->getName()].constant_value<<", "<<LoadedValue->getName()<<" = "<< variable_state[LoadedValue->getName()].constant_value<< "\n";
          }

        }

        /* Processing the Call Instructions */
        else if (CallInst *Call = dyn_cast<CallInst>(&I)) {
          Function *calledFunction = Call->getCalledFunction();
          if (calledFunction->getName() == "__isoc99_scanf") {  // If "scanf" is called, then we assign BOTTOM to the variable
            variable_state[(Call->getArgOperand(1))->getName()].BOTTOM = true;
            
            errs() << I << " --> %" << (Call->getArgOperand(1))->getName() << " = BOTTOM"<< "\n";
          } else if (calledFunction->getName() == "printf") {  // If "printf" is called, then no changes will be done
            errs() << I << "\n";
          }
        }
        /* Processing Return Instructions */ 
        else if (ReturnInst *RetIns = dyn_cast<ReturnInst>(&I)) {
          errs() << I <<"\n";
        }
        /* Processing Arithmetic Instructions */ 
        else if (auto *binaryOp = dyn_cast<BinaryOperator>(&I))
        {
          if (binaryOp->getOpcode() == Instruction::Add)
          {
        
            

                 
            Value *operand1 = binaryOp->getOperand(1); 
            if(ConstantInt *op1 = dyn_cast<ConstantInt>(operand1))
            {
             APInt result = op1->getValue() + variable_state[(binaryOp->getOperandUse(0))->getNameOrAsOperand()].constant_value;
            
             variable_state[I.getName()].constant_value = result;
             variable_state[I.getName()].TOP = false;
             variable_state[I.getName()].BOTTOM = false;

             //errs() <<"Just for verifying>> Key : "<<variable_state["add"].constant_value<<"\n";
             errs() <<I <<" --> %"<<I.getName()<<" = "<< variable_state[I.getName()].constant_value<<"\n";
            }
            else
            {
              llvm::APInt operand1 = variable_state[(binaryOp->getOperandUse(0))
                                                        ->getNameOrAsOperand()]
                                         .constant_value;
              llvm::APInt operand2 = variable_state[(binaryOp->getOperandUse(1))
                                                        ->getNameOrAsOperand()]
                                         .constant_value;

              // Ensure operands have the same bit width
              if (operand1.getBitWidth() == operand2.getBitWidth()) {
                // If the bit widths are different, handle it here
                // For example, you could extend or truncate one of the operands
                // to match the other Let's assume we extend the shorter operand
                // to match the longer one

                // Get the maximum bit width of the two operands
                errs()<<"This is being executed "<<"\n";
                unsigned int maxBitWidth =
                    std::max(operand1.getBitWidth(), operand2.getBitWidth());

                // Extend both operands to have the maximum bit width
                operand1 = operand1.zextOrTrunc(maxBitWidth);
                operand2 = operand2.zextOrTrunc(maxBitWidth);

                APInt result_final = operand1 + operand2;
                errs()<< I <<" --> %"<<I.getName()<<" = "<<result_final<<"\n";
                variable_state[I.getName()].constant_value = result_final;
                variable_state[I.getName()].TOP = false;
                variable_state[I.getName()].BOTTOM  = false;
              }

            // //ConstantInt *constant1 = dyn_cast<ConstantInt>(operand1)
            //   errs() << "Op1 : "<< (binaryOp->getOperandUse(0))->getNameOrAsOperand()<<" OP2 : "<< (binaryOp->getOperandUse(1))->getNameOrAsOperand()<<"\n";
            //   errs()<< " Value of Operand 1  : "<<variable_state[(binaryOp->getOperandUse(0))->getNameOrAsOperand()].constant_value<<"\n";
            //   errs() <<"Executing this"<<"\n";
            //   errs()<< " Value of Operand 2 : "<<variable_state[(binaryOp->getOperandUse(1))->getNameOrAsOperand()].constant_value<<"\n";
            //   if((variable_state[(binaryOp->getOperandUse(0))->getNameOrAsOperand()].constant_value).getBitWidth()!= (variable_state[(binaryOp->getOperandUse(1))->getNameOrAsOperand()].constant_value).getBitWidth())
            //   {
            //     errs()<<"This is True"<<"\n";

            // //     unsigned int maxWidth = std::max((variable_state[(binaryOp->getOperandUse(0))->getNameOrAsOperand()].constant_value).getBitWidth(), variable_state[(binaryOp->getOperandUse(0))->getNameOrAsOperand()].constant_value).getBitWidth());
            // // constant1 = ConstantInt::get(constant1->getContext(), constant1->getValue().sextOrTrunc(maxWidth));
            // // constant2 = ConstantInt::get(constant2->getContext(), constant2->getValue().sextOrTrunc(maxWidth));
            //   }
            //   APInt result2 = variable_state[(binaryOp->getOperandUse(0))->getNameOrAsOperand()].constant_value + variable_state[(binaryOp->getOperandUse(1))->getNameOrAsOperand()].constant_value;
              //errs() << "Value that we want : "<< result2<<"\n";

            }
          }
                else if(binaryOp->getOpcode() == Instruction::Sub) // Handling Sub Instruction
          {
      
            if (ConstantInt *op1 = dyn_cast<ConstantInt>(binaryOp->getOperand(1)))
            {
             APInt result = variable_state[(binaryOp->getOperandUse(0))->getNameOrAsOperand()].constant_value - op1->getValue();

             variable_state[I.getName()].constant_value = result;
             variable_state[I.getName()].TOP = false;
             variable_state[I.getName()].BOTTOM = false;

             errs() <<I <<" --> %"<<I.getName()<<" = "<< variable_state[I.getName()].constant_value<<"\n";

            }
          }
        }
        /* Processing Comparison Instruction */
        else if(CmpInst *cmpinst = dyn_cast<CmpInst>(&I))
        {
          variable_state[I.getName()].BOTTOM = true;
          errs()<<I<<" --> %"<<I.getName() <<" = BOTTOM"<<"\n";
          
        }
        /* Processing Branch Instruction */
        else if(BranchInst *BrInst = dyn_cast<BranchInst>(&I))
        {
          if(BrInst->isConditional())
          {
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
} 

char Demo::ID = 0;
static RegisterPass<Demo> X("demo", "This is a Demo pass");
