#include "go_bridge.h"

#include "Core/MemMap.h"
#include "Core/MIPS/MIPSDebugInterface.h"
#include "Core/Debugger/SymbolMap.h"
#include "Core/MIPS/MIPS.h"
#include "Core/MIPS/MIPSTables.h"
#include "Core/MIPS/MIPSAnalyst.h"

#include <stdio.h>
#include <iostream>
#include <cstring>
#include <string>

void GlobalSetMemoryBase(void *base, uint32_t startAddress) {
  Memory::base = (u8*)base - startAddress;
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

int SymbolMap_SetFunctionSize(BridgeSymbolMap sym, uint32_t startAddress, uint32_t newSize) {
  auto _sym = reinterpret_cast<SymbolMap*>(sym);
  return _sym->SetFunctionSize(startAddress, newSize);
}

int SymbolMap_RemoveFunction(BridgeSymbolMap sym, uint32_t startAddress, int removeName) {
  auto _sym = reinterpret_cast<SymbolMap*>(sym);
  return _sym->RemoveFunction(startAddress, removeName);
}

void GlobalSetSymbolMap(BridgeSymbolMap sym) {
  if (g_symbolMap && sym) {
		std::cout << "WARNING:\toverride g_symbolMap" << std::endl;
	}
  auto _sym = reinterpret_cast<SymbolMap*>(sym);
  g_symbolMap = _sym;
}

int MemoryIsValidAddress(uint32_t address) {
  return Memory::IsValidAddress(address);
}

BridgeMipsOpcodeInfo MIPSAnalystGetOpcodeInfo(uint32_t address) {
  MIPSAnalyst::MipsOpcodeInfo mips_info =
		MIPSAnalyst::GetOpcodeInfo(currentDebugMIPS, address);

  BridgeMipsOpcodeInfo ret = {
    mips_info.opcodeAddress,
    mips_info.encodedOpcode.encoding,

    mips_info.isConditional,
    mips_info.conditionMet,

    mips_info.branchTarget,
    mips_info.isBranch,
    mips_info.isLinkedBranch,
    mips_info.isLikelyBranch,
    mips_info.isBranchToRegister,
    mips_info.branchRegisterNum,
    mips_info.hasDelaySlot,

    mips_info.isDataAccess,
    mips_info.dataSize,
    mips_info.dataAddress,

    mips_info.hasRelevantAddress,
    mips_info.relevantAddress,
  };

  // Implemetation from: currentDebugMIPS->disasm(instr->addr, 4)
	bool tabsToSpaces = false;
  std::string encodingLog;
  MIPSDisAsm(mips_info.encodedOpcode, mips_info.opcodeAddress, ret.dizz,
	  tabsToSpaces, &encodingLog);

  strncpy(ret.log, encodingLog.c_str(), 127);
  ret.log[127] = '\0';

  return ret;
}

const char * MIPSDebugInterface_GetRegName(int cat, int index) {
  return currentDebugMIPS->GetRegName(cat, index);
}
