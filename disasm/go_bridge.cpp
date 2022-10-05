#include "go_bridge.h"

#include "Core/MemMap.h"
#include "Core/Debugger/SymbolMap.h"

#include <stdio.h>
#include <iostream>
#include <cstring>
#include <string>

void GlobalSetMemoryBase(void *base) {
  Memory::base = (u8*)base;
}

BridgeSymbolMap NewSymbolMap() {
  auto _sym = new SymbolMap();
  return reinterpret_cast<BridgeSymbolMap>(_sym);
}

void DeleteSymbolMap(BridgeSymbolMap sym) {
  auto _sym = reinterpret_cast<SymbolMap*>(sym);
  delete _sym;
}

uint32_t SymbolMap_GetFunctionSize(BridgeSymbolMap sym, uint32_t startAddress) {
  auto _sym = reinterpret_cast<SymbolMap*>(sym);
  return _sym->GetFunctionSize(startAddress);
}

uint32_t SymbolMap_GetFunctionStart(BridgeSymbolMap sym, uint32_t address) {
  auto _sym = reinterpret_cast<SymbolMap*>(sym);
  return _sym->GetFunctionStart(address);
}

const char* SymbolMap_GetLabelName(BridgeSymbolMap sym, uint32_t address) {
  auto _sym = reinterpret_cast<SymbolMap*>(sym);
  return _sym->GetLabelName(address);
}

void SymbolMap_AddFunction(BridgeSymbolMap sym, const char* name, uint32_t address, uint32_t size, int moduleIndex) {
  auto _sym = reinterpret_cast<SymbolMap*>(sym);
  _sym->AddFunction(name, address, size, moduleIndex);
}

void SymbolMap_AddModule(BridgeSymbolMap sym, const char *name, uint32_t address, uint32_t size) {
  auto _sym = reinterpret_cast<SymbolMap*>(sym);
  _sym->AddModule(name, address, size);
}

void GlobalSetSymbolMap(BridgeSymbolMap sym) {
  if (g_symbolMap && sym) {
		std::cout << "WARNING:\toverride g_symbolMap" << std::endl;
	}
  auto _sym = reinterpret_cast<SymbolMap*>(sym);
  g_symbolMap = _sym;
}
