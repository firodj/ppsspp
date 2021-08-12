#ifndef __GO_BRIDGE_H__
#define __GO_BRIDGE_H__

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void* BridgeSymbolMap;
typedef const char* (*GetFuncNameFunc)(int moduleIndex, int func);

typedef struct {
  uint32_t opcodeAddress;
  uint32_t encodedOpcode;

  // shared between branches and conditional moves
  uint8_t isConditional;
  uint8_t conditionMet;

  // branches
  uint32_t branchTarget;
  uint8_t isBranch;
  uint8_t isLinkedBranch;
  uint8_t isLikelyBranch;
  uint8_t isBranchToRegister;
  int branchRegisterNum;
  uint8_t hasDelaySlot;

  // data access
  uint8_t isDataAccess;
  int dataSize;
  uint32_t dataAddress;

  uint8_t hasRelevantAddress;
  uint32_t relevantAddress;

  // dissasembly
  char dizz[128];
  char log[128];
} BridgeMipsOpcodeInfo;

void GlobalSetMemoryBase(void *base, uint32_t startAddress);
BridgeSymbolMap NewSymbolMap();
void DeleteSymbolMap(BridgeSymbolMap sym);
uint32_t SymbolMap_GetFunctionSize(BridgeSymbolMap sym, uint32_t startAddress);
uint32_t SymbolMap_GetFunctionStart(BridgeSymbolMap sym, uint32_t address);
int SymbolMap_SetFunctionSize(BridgeSymbolMap sym, uint32_t startAddress, uint32_t newSize);
int SymbolMap_RemoveFunction(BridgeSymbolMap sym, uint32_t startAddress, int removeName);
const char* SymbolMap_GetLabelName(BridgeSymbolMap sym, uint32_t address);
void SymbolMap_AddFunction(BridgeSymbolMap sym, const char* name, uint32_t address, uint32_t size, int moduleIndex);
void SymbolMap_AddModule(BridgeSymbolMap sym, const char *name, uint32_t address, uint32_t size);
void GlobalSetSymbolMap(BridgeSymbolMap sym);
void GlobalSetGetFuncNameFunc(GetFuncNameFunc func);
int MemoryIsValidAddress(uint32_t address);
BridgeMipsOpcodeInfo MIPSAnalystGetOpcodeInfo(uint32_t address);
const char* MIPSDebugInterface_GetRegName(int cat, int index);

#ifdef __cplusplus
}
#endif

#endif