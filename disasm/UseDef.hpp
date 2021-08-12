#pragma once

#include <map>
#include <vector>
#include <string>

#include "Types.hpp"

class MyDocument;
class MyInstruction;
class BasicBlock;

class MyDef {
public:
  MyDef(std::string name, int version = -1);
  void Dump();

  std::string name_;
  int version_;

  MyInstruction *instr_;
	u32 instr_addr_;
	int arg_num_;
};

class MyUse {
public:
  MyUse(std::string name, int version = -1);
  void Dump();

  std::string name_;
  int version_;

  MyInstruction *instr_;
	u32 instr_addr_;
	int arg_num_;
};

typedef std::vector<MyDef> ArrayOfDef;
typedef std::vector<MyUse> ArrayOfUse;
typedef std::map<std::string, ArrayOfDef> MapOfDef;
typedef std::map<std::string, ArrayOfUse> MapOfUse;

class UseDefManager {
public:
  UseDefManager(BasicBlock *bb);

  MyDef &AddDef(std::string name, MyInstruction *instr, int arg_num);
  MyUse &AddUse(std::string name, MyInstruction *instr, int arg_num);

  void Reset();
  void Dump();

  MapOfDef defs_;
  MapOfUse uses_;

  BasicBlock *bb_;
};

class UseDefAnalyzer {
public:
  UseDefAnalyzer(MyDocument *doc);
  void AnalyzeInstruction(MyInstruction *instr);

  void DoAssign(UseDefManager &mgr, MyInstruction* instr);
  void DoStore(UseDefManager &mgr, MyInstruction* instr);
  void DoAllUse(UseDefManager &mgr, MyInstruction *instr);
  void DoDefRa(UseDefManager &mgr, MyInstruction *instr);
  void DoSysCall(UseDefManager &mgr, MyInstruction *instr);

private:
  MyDocument *doc_;
};
