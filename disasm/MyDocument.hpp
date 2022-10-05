#pragma once

#include <string>
#include <vector>
#include <functional>

#include "Types.hpp"
#include "MyInstruction.hpp"
#include "MyFunction.hpp"
#include "BasicBlock.hpp"
#include "UseDef.hpp"
#include "BBTraceParser.hpp"

#include "Core/Debugger/SymbolMap.h"
#include "go_bridge.h"

void SetFuncGetFuncName(GetFuncNameFunc func, void* userdata);

//--- MyDocument ---

typedef std::vector<const char *> FuncArgTypes;

class MemoryDump {
public:
  MemoryDump();
  ~MemoryDump();
  void Allocate(size_t length);

	u8* data_;
  size_t length_;
};

// --- MyHLEFunction ---
class MyHLEFunction {
public:
	std::string name;
  u32 nid;
  FuncArgTypes retmask;
  FuncArgTypes argmask;
  u32 flags;
};

// --- MyHLEModule ---

struct MyHLEModule {
public:
	std::string name;
	std::vector<MyHLEFunction> funcs;
};

typedef std::vector<MyHLEModule> MyHLEModules;

struct DisasmParam {
  u32 bb_addr;
  Instructions lines;
  u32 br_addr;
  u32 last_addr;
};

// --- MyDocument ---
class MyDocument {
public:
  MyDocument();
  ~MyDocument();
  int Init(std::string path, bool load_analyzed = false);
  void Uninit();

  int DisasmRangeTo(std::function<void(DisasmParam)> cb, u32 start_addr, u32 last_addr);
  void ProcessAnalyzedFunc(MyFunction *func);

	MyInstruction *Disasm(u32 addr);
  int DumpPseudo(u32 addr);
  void BBProcessUseDef(BasicBlock *bb);
  MyFunction *SplitFunctionAt(u32 split_addr, MyFunction **OUT_prev_func);
  MyFunction *CreateNewFunction(u32 addr, u32 last_addr);
  void SaveAnalyzed(std::string path);

  u32 GetFunctionStart(u32 address);
  std::string GetLabelString(u32 address);

  std::string filename_;
  BBTraceParser bbtrace_parser_;

  BasicBlockManager bbManager_;
  InstructionManager instrManager_;
  FunctionManager funcManager_;

  size_t MemorySize();
  u8*  MemoryPtr();
  u32   MemoryStart();
  MyHLEModules &hleModules();

  u32 entry_addr() { return entry_addr_; }
  std::string &entry_name() { return entry_name_; }

  int PseuDoAssign(MyInstruction *instr);
  int PseuDoJump(MyInstruction *instr);
  int PseuDoLoadUpper(MyInstruction *instr);
  int PseuDoLoad(MyInstruction *instr);
  int PseuDoStore(MyInstruction *instr);
  int PseuDoNothing(MyInstruction *instr);
  int PseudoSyscall(MyInstruction *instr);

  static const char* GetFuncName(int moduleIndex, int func);
  static MyDocument* currentDoc;

  MyHLEFunction *GetFunc(std::string fullname);

  SymbolMap& symbol_map() { return symbol_map_; }
  MemoryDump& buf() { return buf_; }
  UseDefAnalyzer& useDefAnalyzer() { return useDefAnalyzer_; }
private:
  SymbolMap symbol_map_;
  MyHLEModules moduleDB_;
  MemoryDump buf_;
  u32 memory_start_;
  UseDefAnalyzer useDefAnalyzer_;

  u32 entry_addr_;
  std::string entry_name_;
};

