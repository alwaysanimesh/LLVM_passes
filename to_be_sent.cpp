

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
  map<StringRef,map<StringRef, variable_values>> global_map; // This is a map per BB
  
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

      ////////////////////////////////////////////////////////
        map<StringRef, variable_values> variable_state; // This Map contains the name of the Variable and the Value contained in it
        StringRef current_BB_name = front_BB->getName(); // This holds the name of the current BB
        queue<BasicBlock *> pred_of_current_block;
      /*Here, we get the set of all pred basic block */
      for(BasicBlock *pred : predecessors(front_BB))
      {
        errs()<< "Pred of "<<front_BB->getName()<<" is "<<pred->getName()<<"\n";
        pred_of_current_block.push(pred);      
      }
         if(current_BB_name != "entry" and pred_of_current_block.size() == 1)
         {
          /* If the current BB has only one BB, then we first assign the values of the variables in it then proceed */
            for (auto iti = global_map[pred_of_current_block.front()->getName()].begin(); iti !=global_map[pred_of_current_block.front()->getName()].end(); ++iti) {
        errs()<<"Key : "<< iti->first << " Value : "<< iti->second.constant_value<<"\n";}
          variable_state = global_map[pred_of_current_block.front()->getName()];

         //errs()<<"This is we need "<<variable_state["%0"].TOP<<"\n";
          
          errs()<<"This was executed "<<"\n";
         }
         else if(current_BB_name != "entry" and pred_of_current_block.size() == 2)
         {
          errs()<<"Need to handle this case "<<"\n";
            for (auto it = global_map[pred_of_current_block.front()->getName()].begin(); it != global_map[pred_of_current_block.front()->getName()].end(); ++it) 
            {
            errs() << it->first << " = "<<it->second.constant_value<<"\n"; // Accessing the key with it->first
            for(auto iit = global_map[pred_of_current_block.back()->getName()].begin(); iit != global_map[pred_of_current_block.back()->getName()].end(); ++iit)
            {
              if((iit->first == it->first)) // if the key is same 
              {
                errs()<<"Keys were same "<<iit->first<<"\n";
                if((iit->second.constant_value == it->second.constant_value) and (iit->second.TOP == false) and (iit->second.BOTTOM == false) and (it->second.TOP == false) and (it->second.BOTTOM == false))  // c1 meet c1 = c1
                {
                  variable_state[iit->first].constant_value = iit->second.constant_value;
                  variable_state[iit->first].BOTTOM = false;
                  variable_state[iit->first].TOP = false;

                }
                else if((iit->second.constant_value != it->second.constant_value) and (iit->second.TOP == false) and (iit->second.BOTTOM == false) and (it->second.TOP == false) and (it->second.BOTTOM == false))  // c1 meet c2 = BOTTOM
                {
                  errs() << "Again this was hit : "<<"\n";
                  variable_state[iit->first].BOTTOM = true;
                  variable_state[iit->first].TOP = false;

                }
                else if(iit->second.BOTTOM == true or it->second.BOTTOM == true)
                {
                  variable_state[iit->first].BOTTOM = true;
                  variable_state[iit->first].TOP = false;

                }
                else if(iit->second.TOP == true)
                {
                  variable_state[iit->first] = it->second;
                }
                else if(it->second.TOP == true)
                {
                  variable_state[it->first] = iit->second;
                }
                else{
                  errs()<<"This case is not handled"<<"\n";
                }
                break;

              }
              else // if the keys are not same
              {
                variable_state[iit->first] = iit->second;

              }

            }
            }
            errs()<<"*******************"<<"\n";
            for (auto it = global_map[pred_of_current_block.back()->getName()].begin(); it != global_map[pred_of_current_block.back()->getName()].end(); ++it) 
            {
            errs() << it->first << " = "<<it->second.constant_value<<"\n"; // Accessing the key with it->first
            for(auto iit = global_map[pred_of_current_block.front()->getName()].begin(); iit != global_map[pred_of_current_block.front()->getName()].end(); ++iit)
            {
              if((iit->first == it->first)) // if the key is same 
              {

                if((iit->second.constant_value == it->second.constant_value) and (iit->second.TOP == false) and (iit->second.BOTTOM == false) and (it->second.TOP == false) and (it->second.BOTTOM == false))  // c1 meet c1 = c1
                {
                  variable_state[iit->first].constant_value = iit->second.constant_value;
                  variable_state[iit->first].TOP = false;
                  variable_state[iit->first].BOTTOM = false;

                }
                else if((iit->second.constant_value != it->second.constant_value) and (iit->second.TOP == false) and (iit->second.BOTTOM == false) and (it->second.TOP == false) and (it->second.BOTTOM == false))  // c1 meet c2 = BOTTOM
                {
                  variable_state[iit->first].BOTTOM = true;
                  variable_state[iit->first].TOP = false;

                }
                else if(iit->second.BOTTOM == true or it->second.BOTTOM == true)
                {
                  variable_state[iit->first].BOTTOM = true;
                  variable_state[iit->first].TOP = false;
                }
                else if(iit->second.TOP == true)
                {
                  variable_state[iit->first] = it->second;
                }
                else if(it->second.TOP == true)
                {
                  variable_state[it->first] = iit->second;
                }
                else{
                  errs()<<"This case is not handled"<<"\n";
                }
                break;

              }
              else // if the keys are not same
              {
                variable_state[iit->first] = iit->second;

              }

            }
            }
         }
         errs()<<"Map of the Basic Block: "<<current_BB_name<<"\n";
         for (auto iti = variable_state.begin(); iti != variable_state.end(); ++iti) {
        errs()<<"Key : "<< iti->first << " Value : "<< iti->second.constant_value<<"\n";
    }
    errs()<<"Just to get the value of x : BOTTOM: "<<variable_state["x"].BOTTOM<<" TOP : "<<variable_state["x"].TOP<<" Constant : "<< variable_state["x"].constant_value<<"\n";

      ////////////////////////////////////////////
      
     

      /* We execute all the instructions in the current Basic Block sequentially */
      for (Instruction &I : *front_BB) {
        errs()<<"Opcode of the instruction is: "<<I.getOpcodeName()<<"\n";
      
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
            variable_state[Var->getName()] = variable_state[StoredValue->getName()];
            

            //  /* Below code takes care of the different comnination that we can get by using TOP, BOTTOM and CONSTANT */
            //  if (variable_state[Var->getName()].TOP == true)
            //  {
            //   variable_state[Var->getName()] = variable_state[StoredValue->getName()]; // x meet top = x
            //  }
            //  else if(variable_state[Var->getName()].BOTTOM == true)  // No changes will be done her
            //  {
            //  }
            //  else if(variable_state[Var->getName()].constant_value == variable_state[StoredValue->getName()].constant_value)  // C1 meet C1 = C1, so no changes
            //  {
            //  }
            //  else if(variable_state[Var->getName()].constant_value != variable_state[StoredValue->getName()].constant_value)  // C1 meet C2 = BOTTOM
            //  {
            //   variable_state[Var->getName()].BOTTOM = true;
            //   variable_state[Var->getName()].TOP = false;
            //   //variable_state[Var->getName()].CONSTANT = false;

             
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
          // errs() << ">>"<< LoadedValue->getName()<<"\n";
          // errs() << "Val that I want : "<< variable_state[LoadedValue->getName()].BOTTOM<<"\n";
         
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
            variable_state[(Call->getArgOperand(1))->getName()].TOP = false;
            //errs()<<"This is the value of the variable : "<<variable_state[(Call->getArgOperand(1))->getName()].BOTTOM<<"\n";
            
            errs() << I << " --> %" << (Call->getArgOperand(1))->getName() << " = BOTTOM"<< "\n";
          } else if (calledFunction->getName() == "printf") {  // If "printf" is called, then no changes will be done
            errs() << I << "\n";
          }
        }
        /* Processing Return Instructions */ 
        else if (ReturnInst *RetIns = dyn_cast<ReturnInst>(&I)) {
          errs() << I <<"\n";
          global_map[current_BB_name] = variable_state;
        }
        /* Processing Arithmetic Instructions */ 
        else if (auto *binaryOp = dyn_cast<BinaryOperator>(&I))
        {
          if (binaryOp->getOpcode() == Instruction::Add)
          {
            Value *operand1 = binaryOp->getOperand(1); 
            if(ConstantInt *op1 = dyn_cast<ConstantInt>(operand1))
            {
             if(op1->getValue().getBitWidth() != variable_state[(binaryOp->getOperandUse(0))->getNameOrAsOperand()].constant_value.getBitWidth())
             {
              llvm::APInt ope1 = op1->getValue();
              llvm::APInt ope2 = variable_state[(binaryOp->getOperandUse(0))->getNameOrAsOperand()].constant_value;
              errs()<<"they are not the same "<<"\n";
              unsigned int max_width = std::max(ope1.getBitWidth(),ope2.getBitWidth());
              ope1 = ope1.zextOrTrunc(max_width);
              ope2 = ope2.zextOrTrunc(max_width);

              APInt result_new = ope1 + ope2;
            variable_state[I.getName()].constant_value = result_new;
             variable_state[I.getName()].TOP = false;
             variable_state[I.getName()].BOTTOM = false;

             //errs() <<"Just for verifying>> Key : "<<variable_state["add"].constant_value<<"\n";
             errs() <<I <<" --> %"<<I.getName()<<" = "<< variable_state[I.getName()].constant_value<<"\n";
              
             }
            //  APInt result = op1->getValue() + variable_state[(binaryOp->getOperandUse(0))->getNameOrAsOperand()].constant_value;
            
            //  variable_state[I.getName()].constant_value = result;
            //  variable_state[I.getName()].TOP = false;
            //  variable_state[I.getName()].BOTTOM = false;

            //  //errs() <<"Just for verifying>> Key : "<<variable_state["add"].constant_value<<"\n";
            //  errs() <<I <<" --> %"<<I.getName()<<" = "<< variable_state[I.getName()].constant_value<<"\n";
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
          global_map[current_BB_name] = variable_state;
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


/*Code is correctly working: 
1. File1.ll
2. File4.ll
3. File2.ll 




*/
