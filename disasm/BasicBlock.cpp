#include <map>
#include <iostream>

#include "BasicBlock.hpp"
#include "UseDef.hpp"
#include "MyInstruction.hpp"

// -- BasicBlock --

BasicBlock::BasicBlock(): addr_(0), last_addr_(0), branch_instr_(0),
  defUseManager_(this) {
}

BasicBlock::BasicBlock(u32 addr): addr_(addr), last_addr_(0),
  branch_instr_(0), defUseManager_(this) {
}

void
BasicBlock::AddFuncRef(u32 addr) {
  for (auto check_addr: func_refs_) {
    if (check_addr == addr) return;
  }
  func_refs_.push_back(addr);
}

BBReference::BBReference(): flags_(0)
{};

//--- BasicBlockManager ---

BasicBlockManager::BasicBlockManager(): instrManager_(nullptr) {

}

BasicBlockManager::~BasicBlockManager() {

}

bool
BasicBlockManager::BBIsExists(u32 addr) {
  return basicBlocks_.find(addr) != basicBlocks_.end();
}

MapAddressToBasicBlock::iterator
BasicBlockManager::BBBegin() {
  return basicBlocks_.begin();
}

MapAddressToBasicBlock::iterator
BasicBlockManager::BBEnd() {
  return basicBlocks_.end();
}

BasicBlock* BasicBlockManager::Get(u32 addr)
{
  if (!addr) return nullptr;

  BasicBlock *bb = nullptr;
  auto it = basicBlocks_.lower_bound(addr);
  if (it != basicBlocks_.end()) {

    bb = it->second.get();

    if (addr != bb->addr_) {
      if (it != basicBlocks_.begin()) {
        bb = std::prev(it)->second.get();
      } else {
        bb = nullptr;
      }
    }
  } else {
    auto rit = basicBlocks_.rbegin();
    if (rit != basicBlocks_.rend()) {
      bb = rit->second.get();
    }
  }

  if (bb) {
    if (addr <= bb->last_addr_) return bb;
  }
  return nullptr;
}

BasicBlock* BasicBlockManager::Create(u32 addr)
{
  BasicBlock *bb = Get(addr);
  if (bb) return nullptr;

  bb = new BasicBlock(addr);
  // c++14: std::make_unique<BasicBlock>();
  basicBlocks_[addr] = BasicBlockPtr(bb);

  return bb;
}

BasicBlock* BasicBlockManager::After(u32 addr) {
  if (!addr) return nullptr;

  BasicBlock *bb = nullptr;
  auto it = basicBlocks_.lower_bound(addr);
  if (it != basicBlocks_.end()) {
    bb = it->second.get();

    if (addr == bb->addr_) {
      it = std::next(it);
      if (it != basicBlocks_.end()) {
        bb = it->second.get();
      } else {
        bb = nullptr;
      }
    }
  }

  return bb;
}

BasicBlock*
BasicBlockManager::FetchBasicBlock(u32 addr) {
  if (!addr) return nullptr;

  BasicBlock *bb = nullptr;
  auto it = basicBlocks_.find(addr);
  if (it == basicBlocks_.end()) {
    // c++14: std::make_unique<BasicBlock>();
    basicBlocks_[addr] = BasicBlockPtr(new BasicBlock(addr));
    bb = basicBlocks_[addr].get();
  } else {
    // std::cout << "Found BB at " << addr << std::endl;
    bb = it->second.get();
  }

  return bb;
}

int
BasicBlockManager::BBCount() {
  return basicBlocks_.size();
}

BasicBlock *BasicBlockManager::SplitAt(u32 split_addr, BasicBlock **OUT_prev_bb) {
  BasicBlock *prev_bb = Get(split_addr);
  if (!prev_bb) return nullptr;

  if (OUT_prev_bb) *OUT_prev_bb = prev_bb;

  if (prev_bb->addr_ == split_addr) return nullptr;

  u32 last_addr = prev_bb->last_addr_;
  if (prev_bb->last_addr_ >= split_addr)
    prev_bb->last_addr_ = split_addr - 4;

  BasicBlock *split_bb = Create(split_addr);
  if (!split_bb) {
    prev_bb->last_addr_ = last_addr;
    std::cout << "ERROR\tUnable to create splitted bb at: " << split_addr << std::endl;
    return nullptr;
  }

  if (prev_bb->branch_instr_ >= split_bb->addr_) {
    split_bb->branch_instr_ = prev_bb->branch_instr_;
    prev_bb->branch_instr_ = 0;
  }

  split_bb->last_addr_ = last_addr;
  CreateReference(prev_bb->addr_, split_bb->addr_).adjacent(true);

  return split_bb;
}

BBReference &
BasicBlockManager::CreateReference(u32 from_addr, u32 to_addr) {
  auto k = std::make_pair(from_addr, to_addr);
  bool creates = references_.find(k) == references_.end();

  BBReference &bbref = references_[k];

  if (creates) {
    bbref.from_addr_ = from_addr;
    bbref.to_addr_ = to_addr;

    refs_from_[to_addr].push_back(from_addr);
    refs_to_[from_addr].push_back(to_addr);
  }

  return bbref;
}

Addresses *
BasicBlockManager::RefsFrom(u32 addr) {
  return refs_from_.find(addr) != refs_from_.end() ? &refs_from_[addr] : nullptr;
}

Addresses *
BasicBlockManager::RefsTo(u32 addr) {
  return refs_to_.find(addr) != refs_to_.end() ? &refs_to_[addr] : nullptr;
}