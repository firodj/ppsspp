#pragma once

#include "MyDocument.hpp"

#include "Core/Debugger/SymbolMap.h"

class MemoryDump {
public:
  MemoryDump();
  ~MemoryDump();
  void Allocate(size_t length);

	u8* data_;
  size_t length_;
};

class MyDocumentInternal {
public:
  MyDocumentInternal(MyDocument *doc): memory_start_(0), doc_(doc),
   useDefAnalyzer_(doc) {
  }
  SymbolMap symbol_map_;
  HLEModules moduleDB_;
  MemoryDump buf_;
  u32 memory_start_;
  UseDefAnalyzer useDefAnalyzer_;

  const char* GetFuncName(int moduleIndex, int func);
  MyHLEFunction *GetFunc(std::string fullname);
private:
  MyDocument *doc_;
};

extern MyDocument* g_currentDocument;
