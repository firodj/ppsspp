#pragma once

#include <string>
#include <vector>
#include <map>

#include "Types.hpp"

class MyFunction {
public:
  MyFunction();
  MyFunction(u32 addr);

  u32 addr_, last_addr_;
  Addresses bb_addrs_;
  std::string name_;

  void AddBB(u32 addr);
};

typedef std::unique_ptr<MyFunction> FunctionPtr;
typedef std::map<u32, FunctionPtr> MapAddressToFunction;
typedef std::map<std::string, u32> MapNameToAddress;

class FunctionManager {
public:
  FunctionManager();
  ~FunctionManager();
  // MyFunction* FetchFunction(u32 addr); // deprecated

  MyFunction* GetFunction(u32 addr);
  MyFunction* CreateFunction(u32 addr);

  bool IsExists(u32 addr);
  u32 AddressByName(std::string name);
  void RegisterNameToAddress(std::string name, u32 addr);
  MapAddressToFunction::iterator FNBegin();
  MapAddressToFunction::iterator FNEnd();

  //FunctionManagerInternal *internal() const { return internal_; }

  MapAddressToFunction& functions() { return functions_; }
  MapNameToAddress& mapNameToAddress() { return mapNameToAddress_; }

private:
  //FunctionManagerInternal *internal_;
  MapAddressToFunction functions_;
  MapNameToAddress mapNameToAddress_;
};
