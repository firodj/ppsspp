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

void GlobalSetMemoryBase(void *base);
BridgeSymbolMap NewSymbolMap();
void DeleteSymbolMap(BridgeSymbolMap sym);
uint32_t SymbolMap_GetFunctionSize(BridgeSymbolMap sym, uint32_t startAddress);
uint32_t SymbolMap_GetFunctionStart(BridgeSymbolMap sym, uint32_t address);
const char* SymbolMap_GetLabelName(BridgeSymbolMap sym, uint32_t address);
void SymbolMap_AddFunction(BridgeSymbolMap sym, const char* name, uint32_t address, uint32_t size, int moduleIndex);
void SymbolMap_AddModule(BridgeSymbolMap sym, const char *name, uint32_t address, uint32_t size);
void GlobalSetSymbolMap(BridgeSymbolMap sym);
void GlobalSetGetFuncNameFunc(GetFuncNameFunc func);

#ifdef __cplusplus
}
#endif

#endif