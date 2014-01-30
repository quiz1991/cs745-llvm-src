#include "LocalOpts.h"

// useful headers
using namespace llvm;
using std::cout;
using std::endl;

namespace local {

void constantFold(BasicBlock& block) {
  cout << "  block:" << endl;
  for (BasicBlock::iterator it = block.begin(); it != block.end(); ++it) {
    Instruction* instr = &(*it);
    cout << "    " << instr->getOpcodeName();
    if (BinaryOperator* binOp = dyn_cast<BinaryOperator>(instr)) {
      // ConstantInt* a = dyn_cast<ConstantInt>(binOp->getOperand(0));
      // ConstantInt* b = dyn_cast<ConstantInt>(binOp->getOperand(1));
      cout << " <binary> ";
    } else if (UnaryInstruction* unaryOp = dyn_cast<UnaryInstruction>(instr)) {
      cout << " <unrary> ";
    }
    cout << endl;
  }
}

void LocalOpts::strengthReduction(BasicBlock& block) {
  LLVMContext& context = block.getContext();

  for (BasicBlock::iterator it = block.begin(); it != block.end(); ++it) {
    Instruction* instr = &(*it);
    if (BinaryOperator* binOp = dyn_cast<BinaryOperator>(instr)) {
      BinaryOperator::BinaryOps opcode = binOp->getOpcode();
      Value* left = binOp->getOperand(0);
      Value* right = binOp->getOperand(1);

      // determine the operand types
      Instruction* leftInstr = dyn_cast<Instruction>(left);
      Instruction* rightInstr = dyn_cast<Instruction>(right);
      ConstantInt* leftValue = dyn_cast<ConstantInt>(left);
      ConstantInt* rightValue = dyn_cast<ConstantInt>(right);

      // multiply instruction
      if (opcode == Instruction::Mul) {
        if (leftInstr && rightValue) {
          uint64_t value = rightValue->getValue().getZExtValue();
          uint64_t log2Value = log2(value);
          if (value == (1 << log2Value)) {
            ConstantInt* amount = ConstantInt::get(Type::getInt32Ty(context), log2Value);
            Instruction* shift = BinaryOperator::Create(Instruction::Shl, leftInstr, amount);
            ReplaceInstWithInst(block.getInstList(), it, shift);
          }
        } else if (leftValue && rightInstr) {
          uint64_t value = leftValue->getValue().getZExtValue();
          uint64_t log2Value = log2(value);
          if (value == (1 << log2Value)) {
            ConstantInt* amount = ConstantInt::get(Type::getInt32Ty(context), log2Value);
            Instruction* shift = BinaryOperator::Create(Instruction::Shl, rightInstr, amount);
            ReplaceInstWithInst(block.getInstList(), it, shift);
          }
        }

      // divide instruction
      } else if (opcode == Instruction::SDiv) {
        if (leftInstr && rightValue) {
          uint64_t value = rightValue->getValue().getZExtValue();
          uint64_t log2Value = log2(value);
          if (value == (1 << log2Value)) {
            ConstantInt* amount = ConstantInt::get(Type::getInt32Ty(context), log2Value);
            Instruction* shift = BinaryOperator::Create(Instruction::LShr, leftInstr, amount);
            ReplaceInstWithInst(block.getInstList(), it, shift);
          }
        }
      }

    }
  }
}

void LocalOpts::algebraicIdentities(BasicBlock& block) {
  LLVMContext& context = block.getContext();

  // iterate through instructions
  for (BasicBlock::iterator it = block.begin(); it != block.end(); ++it) {
    Instruction* instr = &(*it);
    if (BinaryOperator* binOp = dyn_cast<BinaryOperator>(instr)) {
      BinaryOperator::BinaryOps opcode = binOp->getOpcode();
      Value* left = binOp->getOperand(0);
      Value* right = binOp->getOperand(1);

      // determine the operand types
      Instruction* leftInstr = dyn_cast<Instruction>(left);
      Instruction* rightInstr = dyn_cast<Instruction>(right);
      ConstantInt* leftValue = dyn_cast<ConstantInt>(left);
      ConstantInt* rightValue = dyn_cast<ConstantInt>(right);

      // both sources are instructions
      if (leftInstr && rightInstr) {
        if (leftInstr->isSameOperationAs(rightInstr)) {
          if (opcode == Instruction::Sub) {
            ConstantInt* value = ConstantInt::get(Type::getInt32Ty(context), 0);
            ReplaceInstWithValue(block.getInstList(), it, value);
          } else if (opcode == Instruction::SDiv) {
            // TODO: does not catch divide by zero
            ConstantInt* value = ConstantInt::get(Type::getInt32Ty(context), 1);
            ReplaceInstWithValue(block.getInstList(), it, value);
          }
        }
        // clean up references if possible
        if (leftInstr->use_empty())
          leftInstr->eraseFromParent();
        if (rightInstr->use_empty())
          rightInstr->eraseFromParent();
      }

      // left source is a constant
      else if (leftInstr && rightValue) {
        if ((opcode == Instruction::Mul || opcode == Instruction::SDiv) &&
             rightValue->isOne()) {
          ReplaceInstWithValue(block.getInstList(), it, leftInstr);
        } else if ((opcode == Instruction::Add || opcode == Instruction::Sub) &&
                    rightValue->isZero()) {
          ReplaceInstWithValue(block.getInstList(), it, leftInstr);
        }
      }

      // right source is a constant
      else if (leftValue && rightInstr) {
        if (opcode == Instruction::Mul && leftValue->isOne()) {
          ReplaceInstWithValue(block.getInstList(), it, rightInstr);
        } else if (opcode == Instruction::Add && leftValue->isZero()) {
          ReplaceInstWithValue(block.getInstList(), it, rightInstr);
        }
      }
    }
  }
}

bool LocalOpts::runOnModule(Module& module) {
  std::cout << "module: " << module.getModuleIdentifier().c_str() << std::endl;
  for (Module::iterator it = module.begin(); it != module.end(); ++it) {
    eachFunction(*it);
  }
  return false;
}

void LocalOpts::eachFunction(Function& function) {
  cout << "function: " << function.getName().data() << endl;
  for (Function::iterator it = function.begin(); it != function.end(); ++it) {
    // algebraicIdentities(*it);
    strengthReduction(*it);
  }
}

// Changed so we can actually modify the code tree.
void LocalOpts::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesCFG();
}

uint64_t LocalOpts::log2(uint64_t x) {
  int i = 0;
  while (x >>= 1) {
    i++;
  }
  return i;
}

// LLVM uses the address of this static member to identify the pass, so the
// initialization value is unimportant.
char LocalOpts::ID = 0;
RegisterPass<LocalOpts> X("local-opts", "15745: Local Optimizations");

}
