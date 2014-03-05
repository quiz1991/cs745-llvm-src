// 15-745 S14 Assignment 2: liveness.h
// Group: akbhanda, zheq
///////////////////////////////////////////////////////////////////////////////

#ifndef __DOMINANCE_PASS_H__
#define __DOMINANCE_PASS_H__

#include "llvm/IR/Function.h"
#include "llvm/Pass.h"

#include "dataflow.h"
#include "util.h"

namespace llvm {


class DominancePass : public DataFlowPass {
 public:
  static char ID;
  DominancePass();
  Assignments top(const BasicBlock& block);
  Assignments generate(const BasicBlock& block);
  Assignments kill(const BasicBlock& block);
  void transferFn(const Assignments& generate, const Assignments& kill,
    const Assignments& input, Assignments& output);
};


}

#endif