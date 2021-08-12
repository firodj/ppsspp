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

//--- MyDocument ---

typedef std::vector<const char *> FuncArgTypes;

class MyDocumentInternal;

class MyHLEFunction {
public:
	std::string name;
  u32 nid;
  FuncArgTypes retmask;
  FuncArgTypes argmask;
  u32 flags;
};

struct MyHLEModule {
public:
	std::string name;
	std::vector<MyHLEFunction> funcs;
};

typedef std::vector<MyHLEModule> HLEModules;

struct DisasmParam {
  u32 bb_addr;
  Instructions lines;
  u32 br_addr;
  u32 last_addr;
};

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

  MyDocumentInternal* internal() { return internal_; }
  size_t MemorySize();
  u8*  MemoryPtr();
  u32   MemoryStart();
  HLEModules &hleModules();

  u32 entry_addr() { return entry_addr_; }
  std::string &entry_name() { return entry_name_; }

  int PseuDoAssign(MyInstruction *instr);
  int PseuDoJump(MyInstruction *instr);
  int PseuDoLoadUpper(MyInstruction *instr);
  int PseuDoLoad(MyInstruction *instr);
  int PseuDoStore(MyInstruction *instr);
  int PseuDoNothing(MyInstruction *instr);
  int PseudoSyscall(MyInstruction *instr);

private:
  MyDocumentInternal *internal_;

  u32 entry_addr_;
  std::string entry_name_;
};

