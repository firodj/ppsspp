#include <iostream>
#include <memory>

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
#include "MyDocument.hpp"

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
	return g_currentDocument->GetFuncName(moduleIndex, func);
}

// Common/StringUtils
size_t truncate_cpy(char *dest, size_t destSize, const char *src) {
	size_t len = strlen(src);
	if (len >= destSize - 1) {
		memcpy(dest, src, destSize - 1);
		len = destSize - 1;
	} else {
		memcpy(dest, src, len);
	}
	dest[len] = '\0';
	return len;
}

namespace Reporting {
  void ReportMessage(const char *message, ...) {

  }
  bool ShouldLogNTimes(const char *identifier, int count) {
    return true;
  }
}

bool HandleAssert(const char *function, const char *file, int line, const char *expression, const char* format, ...) {
  return false;
}

namespace LogTypes {
  enum LOG_TYPE {};
  enum LOG_LEVELS : int {};
}

void GenericLog(LogTypes::LOG_LEVELS level, LogTypes::LOG_TYPE type,
		const char *file, int line, const char *fmt, ...) {

}