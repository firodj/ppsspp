#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdio>
#include <string>
#include <exception>

#include "argh.h"

#include "FlameGraph.hpp"

#include "Core/MIPS/MIPSAnalyst.h"

//--- Assert ---
class Assert : public std::exception {
public:
  Assert(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    std::vsnprintf(what_msg_, 1024, fmt, args);

    va_end(args);
  }
  const char * what () const throw () {
    return what_msg_;
  }

  char what_msg_[1024];

  static void Equals(u32 expected, u32 actual, const char *msg, const char *filesrc, int lineno);
  static void NotEquals(u32 expected, u32 actual, const char *msg, const char *filesrc, int lineno);
  static void Equals(std::string expected, std::string actual, const char *msg, const char *filesrc, int lineno);
  static void Equals(void* expected, void* actual, const char *msg, const char *filesrc, int lineno);
  static void IsNull(void* actual, const char *msg, const char *filesrc, int lineno);
  static void IsNotNull(void* actual, const char *msg, const char *filesrc, int lineno);
  static void IsTrue(bool actual, const char *msg, const char *filesrc, int lineno);
  static void IsFalse(bool actual, const char *msg, const char *filesrc, int lineno);
};

void Assert::Equals(std::string expected, std::string actual, const char *msg, const char *filesrc, int lineno)
{
  if (expected != actual)
    throw Assert("unexpected %s = %s, expect %s (%s:%d)", msg, actual.c_str(), expected.c_str(), filesrc, lineno);
}

void Assert::NotEquals(u32 expected, u32 actual, const char *msg, const char *filesrc, int lineno)
{
  if (expected == actual)
    throw Assert("unexpected %s = %d, expect %d (%s:%d)", msg, actual, expected, filesrc, lineno);
}

void Assert::Equals(u32 expected, u32 actual, const char *msg, const char *filesrc, int lineno)
{
  if (expected != actual)
    throw Assert("unexpected %s = %d (0x%08x), expect %d (0x%08x) (%s:%d)", msg, actual, actual, expected, expected,filesrc, lineno);
}

void Assert::Equals(void* expected, void* actual, const char *msg, const char *filesrc, int lineno)
{
  if (expected != actual)
    throw Assert("unexpected %s = 0x%x, expect 0x%x (%s:%d)", msg, (unsigned long)actual, (unsigned long)expected, filesrc, lineno);
}

void Assert::IsNull(void* actual, const char *msg, const char *filesrc, int lineno)
{
  if (nullptr != actual)
    throw Assert("unexpected %s = 0x%x, expect null (%s:%d)", msg, (unsigned long)actual, filesrc, lineno);
}

void Assert::IsNotNull(void* actual, const char *msg, const char *filesrc, int lineno)
{
  if (nullptr == actual)
    throw Assert("unexpected %s = 0x%x, expect non-null (%s:%d)", msg, (unsigned long)actual, filesrc, lineno);
}

void Assert::IsTrue(bool actual, const char *msg, const char *filesrc, int lineno) {
  if (true != actual)
    throw Assert("unexpected %s is false, expect true (%s:%d)", msg, filesrc, lineno);
}

void Assert::IsFalse(bool actual, const char *msg, const char *filesrc, int lineno) {
  if (false != actual)
    throw Assert("unexpected %s is true, expect false (%s:%d)", msg, filesrc, lineno);
}

//---

std::string GetSoraPath() {
  const char* env_p;
#ifdef WIN32
	env_p = std::getenv("USERPROFILE");
#else
	env_p = std::getenv("HOME");
#endif
	std::string path = env_p;
  path += "/Sora";
  return path;
}

void InitDoc(MyDocument &doc, bool load_analyzed = false) {
  int res = doc.Init(GetSoraPath(), load_analyzed);

  Assert::Equals(EXIT_SUCCESS, res, "doc.Init result", __FILE__, __LINE__);
}

void TestDoc() {
  MyDocument g_doc;
  InitDoc(g_doc);

  //Assert::Equals((void*)&g_doc, (void*)g_currentDocument, "g_currentDocument", __FILE__, __LINE__);
  Assert::IsNotNull(g_symbolMap, "g_symbolMap", __FILE__, __LINE__);

  std::cout << "TestDoc" << std::endl;

  Assert::Equals("start", g_doc.entry_name(), "doc.entry_name", __FILE__, __LINE__);
  Assert::Equals(0x8804114, g_doc.entry_addr(), "doc.entry_addr", __FILE__, __LINE__);

  MipsOpcodeInfo info;
  MyInstruction *instr = g_doc.Disasm(0x8804114);

  Assert::Equals(sizeof(MIPSAnalyst::MipsOpcodeInfo), sizeof(MipsOpcodeInfo), "MipsOpcodeInfo size",
    __FILE__, __LINE__);
  Assert::Equals(0x27bdffe0, instr->info_.encodedOpcode, "doc.Init result", __FILE__, __LINE__);

  instr = g_doc.Disasm(0x8A38A8C);
  Assert::Equals(0x0014030c, instr->info_.encodedOpcode, "encoded at 0x8A38A8C contains SysCall", __FILE__, __LINE__);
  Assert::Equals("syscall\tSysMemUserForUser::sceKernelSetCompilerVersion", instr->dizz_, "Disasm at 0x8A38A8C contains SysCall", __FILE__, __LINE__);

  g_doc.Uninit();
  //Assert::IsNull(g_currentDocument, "g_currentDocument", __FILE__, __LINE__);
  Assert::IsNull(g_symbolMap, "g_symbolMap", __FILE__, __LINE__);

  std::cout << "\tPASS" << std::endl;
}

void TestInstr() {
  MyDocument g_doc;
  InitDoc(g_doc);

  std::cout << "TestInstr" << std::endl;

  MyInstruction *instr = g_doc.instrManager_.GetInstruction(0x8004000);
  Assert::IsNull((void*)instr, "GetInstruction on non-existent", __FILE__, __LINE__);

  instr = g_doc.instrManager_.CreateInstruction(0x8004000);
  Assert::IsNotNull((void*)instr, "CreateInstruction on non-existent", __FILE__, __LINE__);

  instr = g_doc.instrManager_.CreateInstruction(0x8004000);
  Assert::IsNull((void*)instr, "CreateInstruction on existent", __FILE__, __LINE__);

  instr = g_doc.instrManager_.GetInstruction(0x8004000);
  Assert::IsNotNull((void*)instr, "GetInstruction on existent", __FILE__, __LINE__);

  std::cout << "\tPASS" << std::endl;
}

void TestBasicBlock() {
  MyDocument g_doc;
  InitDoc(g_doc);

  std::cout << "TestBasicBlock" << std::endl;

  u32 addr = 0x8004000;
  u32 addr2 = addr + 0x2000;
  BasicBlock *bb = g_doc.bbManager_.Get(addr);
  Assert::IsNull(bb, "BasicBlockManager::Get at addr", __FILE__,  __LINE__);

  bb = g_doc.bbManager_.Create(addr);
  Assert::IsNotNull(bb, "BasicBlockManager::Create at addr", __FILE__,  __LINE__);
  bb->last_addr_ = bb->addr_ + 0x1000 - 4;

  bb = g_doc.bbManager_.Create(addr);
  Assert::IsNull(bb, "BasicBlockManager::Create at addr", __FILE__,  __LINE__);

  bb = g_doc.bbManager_.Get(addr);
  Assert::IsNotNull(bb, "BasicBlockManager::Get at addr", __FILE__,  __LINE__);

  bb = g_doc.bbManager_.Create(addr2);
  Assert::IsNotNull(bb, "BasicBlockManager::Create at addr2", __FILE__,  __LINE__);
  bb->last_addr_ = bb->addr_ + 0x1000 - 4;

  bb = g_doc.bbManager_.Get(addr - 0x10);
  Assert::IsNull(bb, "BasicBlockManager::Get at addr-0x10", __FILE__,  __LINE__);

  bb = g_doc.bbManager_.Get(addr + 0x10);
  Assert::IsNotNull(bb, "BasicBlockManager::Get at addr+0x10", __FILE__,  __LINE__);
  Assert::Equals(addr, bb->addr_, "BasicBlockManager::Get at addr+0x10 is BB at addr", __FILE__,  __LINE__);
  bb = g_doc.bbManager_.Create(addr + 0x10);
  Assert::IsNull(bb, "BasicBlockManager::Create at addr+0x10", __FILE__,  __LINE__);

  bb = g_doc.bbManager_.Get(addr2 - 0x10);
  Assert::IsNull(bb, "BasicBlockManager::Get at addr2-0x10", __FILE__,  __LINE__);

  bb = g_doc.bbManager_.Get(addr2 + 0x10);
  Assert::IsNotNull(bb, "BasicBlockManager::Get at addr2+0x10", __FILE__,  __LINE__);
  Assert::Equals(addr2, bb->addr_, "BasicBlockManager::Get at addr2+0x10 is BB at addr2", __FILE__,  __LINE__);

  bb = g_doc.bbManager_.Get(addr2 + 0x1010);
  Assert::IsNull(bb, "BasicBlockManager::Get at addr2+0x10", __FILE__,  __LINE__);

  bb = g_doc.bbManager_.After(addr);
  Assert::IsNotNull(bb, "BasicBlockManager::After at addr", __FILE__,  __LINE__);
  Assert::Equals(addr2, bb->addr_, "BasicBlockManager::After at addr", __FILE__,  __LINE__);

  BasicBlock *bb2 = g_doc.bbManager_.After(addr+0x10);
  Assert::Equals(bb, bb2, "BasicBlockManager::After at addr+0x10", __FILE__,  __LINE__);

  bb = g_doc.bbManager_.After(addr-0x10);
  Assert::IsNotNull(bb, "BasicBlockManager::After at addr-0x10", __FILE__,  __LINE__);
  Assert::Equals(addr, bb->addr_, "BasicBlockManager::After at addr-0x10", __FILE__,  __LINE__);

  bb = g_doc.bbManager_.After(addr2);
  Assert::IsNull(bb, "BasicBlockManager::After at addr2", __FILE__,  __LINE__);

  bb = g_doc.bbManager_.Create(0x88041A4);
  bb->last_addr_ = 0x88041B4;
  bb->branch_instr_ = 0x88041B0;
  u32 split_addr = 0x88041A8;

  BasicBlock *split_bb = nullptr, *prev_bb = nullptr;
  split_bb = g_doc.bbManager_.SplitAt(split_addr, &prev_bb);
  Assert::IsNotNull(bb, "split_bb SplitAt", __FILE__,  __LINE__);
  Assert::IsNotNull(bb, "prev_bb SplitAt", __FILE__,  __LINE__);
  Assert::Equals(0x88041A4, prev_bb->addr_, "prev_bb last_addr SplitAt", __FILE__, __LINE__);
  Assert::Equals(0x88041A8 - 4, prev_bb->last_addr_, "prev_bb last_addr SplitAt", __FILE__, __LINE__);
  Assert::Equals(0, prev_bb->branch_instr_, "prev_bb branch_instr SplitAt", __FILE__, __LINE__);
  Assert::Equals(0x88041A8, split_bb->addr_, "split_bb addr SplitAt", __FILE__, __LINE__);
  Assert::Equals(0x88041B4, split_bb->last_addr_, "split_bb last_addr SplitAt", __FILE__, __LINE__);
  Assert::Equals(0x88041B0, split_bb->branch_instr_, "split_bb last_addr SplitAt", __FILE__, __LINE__);

  std::cout << "\tPASS" << std::endl;
}

std::string LoadFixture(std::string name) {
  std::stringstream ss;
  std::fstream fs(name, std::fstream::in);
  if (fs.good())
    ss << fs.rdbuf();
  else
    throw Assert("unable to LoadFixture: %s", name.c_str());
  return ss.str();
}

void TestDisasm() {
  MyDocument g_doc;
  InitDoc(g_doc);

  std::cout << "TestDisasm" << std::endl;

  std::string fn_name = "start";
  u32 fn_address = g_doc.funcManager_.AddressByName(fn_name);
  Assert::NotEquals(0, fn_address, "AddressByName start", __FILE__, __LINE__);

  MyFunction *func = g_doc.funcManager_.GetFunction(fn_address);
  Assert::IsNotNull(func, "GetFunction start", __FILE__, __LINE__);

	u32 fn_size = (func->last_addr_ - func->addr_ + 4); //  be: = g_symbolMap->GetFunctionSize(fn_address)
  Assert::Equals(276, fn_size, "Funtion size of start", __FILE__, __LINE__);

  std::stringstream *current_ss = nullptr;

  auto print_disasm = [&](DisasmParam param) {
    if (!current_ss) return;

    std::stringstream &ss = *current_ss;
    ss << std::hex << param.bb_addr << ":" << std::endl;
    for (auto &s: param.lines) {
      ss << '\t' << s->dizz_ << std::endl;
    }
    ss << "---" << std::endl;
  };

	int bb_count = g_doc.DisasmRangeTo(print_disasm, func->addr_, func->last_addr_);
  Assert::Equals(13, bb_count, "bb_count DisasmRangeTo", __FILE__, __LINE__);

  std::stringstream ss_one;
  current_ss = &ss_one;

  bb_count = g_doc.DisasmRangeTo(print_disasm, func->addr_, 0);
  Assert::Equals(1, bb_count, "bb_count DisasmRangeTo", __FILE__, __LINE__);

  auto one_bb = LoadFixture("tests/one_bb.txt");
  Assert::IsTrue(one_bb == ss_one.str(), "Context one_bb", __FILE__, __LINE__);

  std::cout << "\tPASS" << std::endl;
}

void TestFunc() {
  MyDocument g_doc;
  InitDoc(g_doc);

  std::cout << "TestFunc" << std::endl;

  u32 func_prev_addr = 0x8a2e768;
  u32 func1_addr = 0x8a319a8;
  u32 func2_addr = 0x8a2f8fc;

  {
    MyFunction *theFunc = g_doc.funcManager_.GetFunction(func1_addr);
    Assert::IsNull(theFunc, "func1_addr should null", __FILE__, __LINE__);

    u32 prev_fn_start = g_doc.GetFunctionStart(func1_addr);

    Assert::NotEquals(func1_addr, prev_fn_start, "find func1_addr start before", __FILE__, __LINE__);
    Assert::Equals(func_prev_addr, prev_fn_start, "prev_fn start at expected func_prev addr", __FILE__, __LINE__);

    MyFunction *prev_func = nullptr;
    MyFunction *split_func = g_doc.SplitFunctionAt(func1_addr, &prev_func);

    Assert::IsNotNull(split_func, "split func not null", __FILE__, __LINE__);
    Assert::IsNotNull(prev_func, "prev func not null", __FILE__, __LINE__);

    Assert::Equals(prev_fn_start, prev_func->addr_, "prev func addr", __FILE__, __LINE__);
    Assert::Equals(func1_addr - 4, prev_func->last_addr_, "prev func last addr", __FILE__, __LINE__);

    Assert::Equals(func1_addr, split_func->addr_, "split func addr", __FILE__, __LINE__);
    Assert::Equals("z_un_08a319a8", split_func->name_, "split func name", __FILE__, __LINE__);

    u32 fn_start = g_doc.GetFunctionStart(func1_addr);
    Assert::Equals(func1_addr, fn_start, "func1_addr start after in SymbolMap", __FILE__, __LINE__);
    u32 fn_size = g_doc.internal()->symbol_map_.GetFunctionSize(fn_start);
    u32 expected_fn_size = split_func->last_addr_ -  split_func->addr_ + 4;
    Assert::Equals(expected_fn_size, fn_size, "func1_addr size after in SymbolMap", __FILE__, __LINE__);

    u32 prev_fn_size = g_doc.internal()->symbol_map_.GetFunctionSize(prev_fn_start);
    expected_fn_size = prev_func->last_addr_ -  prev_func->addr_ + 4;
    Assert::Equals(expected_fn_size, prev_fn_size, "prev_fn size after in SymbolMap", __FILE__, __LINE__);
  }

  {
    MyFunction *theFunc = g_doc.funcManager_.GetFunction(func2_addr);
    Assert::IsNull(theFunc, "func2_addr should be null", __FILE__, __LINE__);

    u32 prev_fn_start = g_doc.GetFunctionStart(func2_addr);
    Assert::NotEquals(func2_addr, prev_fn_start, "find func1_addr start before", __FILE__, __LINE__);
    Assert::Equals(func_prev_addr, prev_fn_start, "prev_fn start at expected func_prev addr", __FILE__, __LINE__);

    MyFunction *prev_func = nullptr;
    MyFunction *split_func = g_doc.SplitFunctionAt(func2_addr, &prev_func);
    Assert::IsNotNull(split_func, "split func at func2_addr should not null", __FILE__, __LINE__);
    Assert::IsNotNull(prev_func, "prev func at func2_addr should not null", __FILE__, __LINE__);

    Assert::Equals("z_un_08a2f8fc", split_func->name_, "split func name", __FILE__, __LINE__);
  }

  std::cout << "\tPASS" << std::endl;
}

void TestFlameGraph() {
  FlameGraph flame_graph;

  Assert::Equals(STACKGRAPH_BASE, flame_graph.stack_graphs_.size(), "stack_graphs size at initial", __FILE__, __LINE__);
  Assert::Equals(0, flame_graph.GetMaxLevel(), "flame_graph GetMaxLevel", __FILE__, __LINE__);

  Assert::IsNull(flame_graph.GetStackGraph(0), "GetStackGraph lower boud", __FILE__, __LINE__);

  auto stack_graph1 = flame_graph.GetStackGraph(1);
  Assert::IsNotNull(stack_graph1, "GetStackGraph(1)", __FILE__, __LINE__);
  Assert::Equals(1, flame_graph.GetMaxLevel(), "flame_graph GetMaxLevel with 1 level", __FILE__, __LINE__);

  auto stack_graph3 = flame_graph.GetStackGraph(3);
  Assert::IsNotNull(stack_graph3, "GetStackGraph(3)", __FILE__, __LINE__);
  Assert::Equals(3, flame_graph.GetMaxLevel(), "flame_graph GetMaxLevel with 3 level", __FILE__, __LINE__);

  flame_graph.AddMarker(1, "start");
  flame_graph.AddMarker(11, "stop");
  Assert::Equals(2, flame_graph.stack_graphs_[0].GetSize(), "stack_graphs_[0] or marker items", __FILE__, __LINE__);

  auto block = stack_graph1->Add(1, 0x4000, "un_4000");
  block->stop_ = 11;

  auto stack_graph2 = flame_graph.GetStackGraph(2);
  block = stack_graph2->Add(2, 0x6000, "un_6000");
  block->End(8);

  block = stack_graph3->Add(3, 0x8000, "un_8000");
  block->End(4);

  block = stack_graph3->Add(5, 0x8800, "un_8800");
  block->End(6);

  Assert::Equals(1, stack_graph1->GetSize(), "stack_graphs level 1 items", __FILE__, __LINE__);
  Assert::Equals(1, stack_graph2->GetSize(), "stack_graphs level 2 items", __FILE__, __LINE__);
  Assert::Equals(2, stack_graph3->GetSize(), "stack_graphs level 3 items", __FILE__, __LINE__);
}

void TestBBTrace(int n) {
  {
    MyDocument g_doc;
    InitDoc(g_doc);

    std::cout << "TestBBTrace" << std::endl;

    g_doc.bbtrace_parser_.shown_ = false;
    g_doc.bbtrace_parser_.Parse([&g_doc](BBTParseParam par) {
      BBTraceParser::ParsingBB(g_doc.bbtrace_parser_, par);
    }, n);
    g_doc.bbtrace_parser_.EndParsing();

    g_doc.bbtrace_parser_.RenderFlameGraph();
    g_doc.SaveAnalyzed(GetSoraPath());

    std::cout << "Store BBs:" << std::dec << g_doc.bbManager_.BBCount() << std::endl;
    std::cout << "Parse: " << std::dec << g_doc.bbtrace_parser_.nts() << std::endl;
  }

  {
    MyDocument g_doc2;
    InitDoc(g_doc2, true);
    std::cout << "Loaded BBs:" << std::dec << g_doc2.bbManager_.BBCount() << std::endl;

    std::cout << "Funcs has BBs: ";
    for (auto fn_it = g_doc2.funcManager_.FNBegin(); fn_it != g_doc2.funcManager_.FNEnd(); fn_it++) {
      MyFunction *func = fn_it->second.get();
      if (func->bb_addrs_.size() > 0)
        std::cout << ", " << func->name_;
    }
    std::cout << std::endl;
  }

  std::cout << "\tPASS" << std::endl;
}

int main(int argc, char *argv[]) {
  argh::parser cmdl;
  cmdl.parse(argc, argv, argh::parser::PREFER_PARAM_FOR_UNREG_OPTION);
  int n = 20;

	if (cmdl(1)) {
		cmdl(1) >> n;
	}

  try {
    TestDoc();
    TestInstr();
    TestBasicBlock();
    TestDisasm();
    TestFunc();
    TestFlameGraph();
    TestBBTrace(n);

    std::cout << "DONE" << std::endl;
    return EXIT_SUCCESS;
  }
  catch (std::exception& e) {
    std::cout << "FAILED\texception: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
