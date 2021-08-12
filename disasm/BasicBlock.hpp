#pragma once

#include <vector>
#include <map>

#include "Types.hpp"
#include "UseDef.hpp"

class BasicBlockInternal;
class BasicBlockManagerInternal;
class InstructionManager;

class BBReference {
public:
  BBReference();

  u32 from_addr_;
  u32 to_addr_;

  union {
    struct {
      bool is_dynamic:1; // immediate or by reg/mem/pointer
      bool is_adjacent:1; // next/prev bb
      bool is_linked:1; // call/linked or not
      bool is_visited:1; // by bbtrace
    };
    char flags_;
  };

   BBReference &adjacent(bool val) { is_adjacent = val; return *this; }
   BBReference &linked(bool val) { is_linked = val; return *this; }
};

class BasicBlock {
public:
  BasicBlock();
  BasicBlock(u32 addr);

  u32 addr_;
  u32 last_addr_; // inbound
  u32 branch_instr_; // usually last_addr_ - 4

  void AddFuncRef(u32 addr);

  UseDefManager defUseManager_;

  Addresses func_refs_; // func that using this bb, usually 1 only.
};

typedef std::unique_ptr<BasicBlock> BasicBlockPtr;
typedef std::map<u32, BasicBlockPtr> MapAddressToBasicBlock;
typedef std::pair<u32, u32> AddrPair;

class BasicBlockManager {
public:
  BasicBlockManager();
  ~BasicBlockManager();

  BasicBlock* FetchBasicBlock(u32 addr); //deprecated
  BasicBlock* Get(u32 addr);
  BasicBlock* Create(u32 addr);
  BasicBlock* After(u32 addr);

  bool BBIsExists(u32 addr);  //deprecated
  int BBCount();

  MapAddressToBasicBlock::iterator BBBegin();
  MapAddressToBasicBlock::iterator BBEnd();
  BasicBlock *BBSplitAt(u32 bb_addr, u32 split_addr); // deprecated
  BasicBlock *SplitAt(u32 split_addr, BasicBlock **OUT_prev_bb);
  std::map<AddrPair, BBReference> references_;
  std::map<u32, Addresses> refs_from_, refs_to_;

  BBReference &CreateReference(u32 from_addr, u32 to_addr);
  Addresses *RefsFrom(u32 addr);
  Addresses *RefsTo(u32 addr);

  BasicBlockManagerInternal *internal() { return internal_; }
private:
  BasicBlockManagerInternal *internal_;
};
