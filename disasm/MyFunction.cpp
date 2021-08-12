#include "MyFunction_internal.hpp"

#include <iostream>

//--- MyFunction ---

MyFunction::MyFunction(): addr_(0), last_addr_(0) {
};

MyFunction::MyFunction(u32 addr): addr_(addr), last_addr_(0) {
};

 void MyFunction::AddBB(u32 addr) {
  for (auto check_addr: bb_addrs_)
    if (check_addr == addr) return;
  bb_addrs_.push_back(addr);
}

//--- FunctionManager ---

FunctionManager::FunctionManager()
{
  internal_ = new FunctionManagerInternal();
}

FunctionManager::~FunctionManager()
{
  delete internal_;
}

MyFunction*
FunctionManager::GetFunction(u32 addr) {
  if (!addr) return nullptr;

  auto it = internal_->functions_.find(addr);
  if (it == internal_->functions_.end()) return nullptr;

  return it->second.get();
}

MyFunction*
FunctionManager::CreateFunction(u32 addr) {
  if (!addr) return nullptr;

  auto it = internal_->functions_.find(addr);
  if (it != internal_->functions_.end()) return nullptr;

  MyFunction *func = new MyFunction(addr);
  internal_->functions_[addr] = FunctionPtr(func);

  return func;
}

bool
FunctionManager::IsExists(u32 addr) {
  return internal_->functions_.find(addr) != internal_->functions_.end();
}

u32
FunctionManager::AddressByName(std::string name)
{
  if (internal_->mapNameToAddress_.find(name) == internal_->mapNameToAddress_.end()) {
    return 0;
  }
  return internal_->mapNameToAddress_[name];
}

void
FunctionManager::RegisterNameToAddress(std::string name, u32 addr)
{
  internal_->mapNameToAddress_.insert(std::make_pair(name, addr));
}

MapAddressToFunction::iterator FunctionManager::FNBegin()
{
  return internal_->functions_.begin();
}

MapAddressToFunction::iterator FunctionManager::FNEnd()
{
  return internal_->functions_.end();
}