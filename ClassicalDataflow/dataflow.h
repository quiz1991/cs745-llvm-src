// 15-745 S14 Assignment 2: dataflow.h
// Group: bovik, bovik2
///////////////////////////////////////////////////////////////////////////////

#ifndef __CLASSICAL_DATAFLOW_DATAFLOW_H__
#define __CLASSICAL_DATAFLOW_DATAFLOW_H__

#include <iostream>
#include <queue>

#include "llvm/IR/Instructions.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/ValueMap.h"
#include "llvm/Support/CFG.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Pass.h"

#include "util.h"


namespace llvm {


enum Meet {
  INTERSECTION,
  UNION
};

enum Direction {
  FORWARDS,
  BACKWARDS
};

enum Top {
  ALL,
  NONE
};

class DataFlowPass : public FunctionPass {
 public:
  DataFlowPass(char id, Top top, Meet meet, Direction direction);
  void ExampleFunctionPrinter(raw_ostream& O, const Function& F);
  void transferFunction(const Assignments& generate, const Assignments& kill,
    Assignments& input, Assignments& output);
  virtual Assignments generate(const BasicBlock& block) = 0;
  virtual Assignments kill(const BasicBlock& block) = 0;
  // pass API
  virtual bool runOnFunction(Function& F);
  virtual void getAnalysisUsage(AnalysisUsage& AU) const;

 protected:
  const Top _top;
  const Meet _meet;
  const Direction _direction;

 private:
  void PrintInstructionOps(raw_ostream& O, const Instruction* I);
};


}

#endif
