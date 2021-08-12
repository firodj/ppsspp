#pragma once

#include <memory>

#include "MyFunction.hpp"

class FunctionManagerInternal {
public:
  MapAddressToFunction functions_;
  std::map<std::string, u32> mapNameToAddress_;
};
