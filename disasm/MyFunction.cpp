#include "MyFunction.hpp"

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
}

FunctionManager::~FunctionManager()
{
}

MyFunction*
FunctionManager::GetFunction(u32 addr) {
  if (!addr) return nullptr;

  auto it = functions().find(addr);
  if (it == functions().end()) return nullptr;

  return it->second.get();
}

MyFunction*
FunctionManager::CreateFunction(u32 addr) {
  if (!addr) return nullptr;

  auto it = functions().find(addr);
  if (it != functions().end()) return nullptr;

  MyFunction *func = new MyFunction(addr);
  functions()[addr] = FunctionPtr(func);

  return func;
}

bool
FunctionManager::IsExists(u32 addr) {
  return functions().find(addr) != functions().end();
}

u32
FunctionManager::AddressByName(std::string name)
{
  if (mapNameToAddress().find(name) == mapNameToAddress().end()) {
    return 0;
  }
  return mapNameToAddress()[name];
}

void
FunctionManager::RegisterNameToAddress(std::string name, u32 addr)
{
  mapNameToAddress().insert(std::make_pair(name, addr));
}

MapAddressToFunction::iterator FunctionManager::FNBegin()
{
  return functions().begin();
}

MapAddressToFunction::iterator FunctionManager::FNEnd()
{
  return functions().end();
}