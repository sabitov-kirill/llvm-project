#include "MCTargetDesc/SystemSMCTargetDesc.h"
#include "SystemS.h"
#include "SystemSISelLowering.h"
#include "SystemSTargetMachine.h"

#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/CodeGen/TargetLowering.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "SystemS-isel"

namespace {

class SystemSDAGToDAGISel : public SelectionDAGISel {
public:
  [[maybe_unused]]
  static char ID;

  SystemSDAGToDAGISel() = delete;
  explicit SystemSDAGToDAGISel(SystemSTargetMachine &TM,
                               CodeGenOptLevel OptLevel)
      : SelectionDAGISel(TM, OptLevel) {}

  bool runOnMachineFunction(MachineFunction &MF) override {
    return SelectionDAGISel::runOnMachineFunction(MF);
  }

  void Select(SDNode *N) override;

#include "SystemSGenDAGISel.inc"
};

class SystemSDAGToDAGISelLegacy : public SelectionDAGISelLegacy {
public:
  static char ID;

  SystemSDAGToDAGISelLegacy(SystemSTargetMachine &TM, CodeGenOptLevel OptLevel)
      : SelectionDAGISelLegacy(
            ID, std::make_unique<SystemSDAGToDAGISel>(TM, OptLevel)) {}

  StringRef getPassName() const override {
    return "SystemS DAG->DAG Pattern Instruction Selection";
  }
};
} // end anonymous namespace

char SystemSDAGToDAGISelLegacy::ID = 0;

FunctionPass *llvm::createSystemSISelDag(SystemSTargetMachine &TM,
                                         CodeGenOptLevel OptLevel) {
  return new SystemSDAGToDAGISelLegacy(TM, OptLevel);
}

void SystemSDAGToDAGISel::Select(SDNode *Node) {
  if (Node->isMachineOpcode()) {
    Node->setNodeId(-1);
    return;
  }
  SDLoc DL(Node);
  SelectCode(Node);
}