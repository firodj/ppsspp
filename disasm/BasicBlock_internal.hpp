#pragma once

#include "BasicBlock.hpp"
#include "UseDef.hpp"

class BasicBlockManagerInternal {
public:
  BasicBlockManagerInternal(): instrManager_(nullptr) {};
  void instrManager(InstructionManager *instrManager) { instrManager_ = instrManager; }

  MapAddressToBasicBlock basicBlocks_;
  InstructionManager *instrManager_;
};
