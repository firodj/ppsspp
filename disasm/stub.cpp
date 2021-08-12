#include <iostream>

#include "Core/MIPS/MIPSDebugInterface.h"
// #include "Core/Debugger/DisassemblyManager.h"
#include "Core/Debugger/MemBlockInfo.h"
#include "Core/System.h"
//#include "Core/Debugger/SymbolMap.h"
#include "Core/MIPS/MIPS.h"
#include "Core/MIPS/MIPSTables.h"
#include "Core/MIPS/MIPSAnalyst.h"
//#include "Core/MemMap.h"
//#include "Yaml.hpp"

#include "MyDocument_internal.hpp"

MyDocument* g_currentDocument = nullptr;

bool PSP_IsInited() {
  return true;
}

Path GetSysDirectory(PSPDirectories directoryType) {
  return Path();
}

void NotifyMemInfo(MemBlockFlags flags, uint32_t start, uint32_t size, const char* str, size_t strLength) {}

const char* GetFuncName(int moduleIndex, int func) {
  if (!g_currentDocument) {
    std::cout << "ERRROR\tGetFuncName\tcurrentDocument is not set" << std::endl;
    return nullptr;
  }
	return g_currentDocument->internal()->GetFuncName(moduleIndex, func);
}
