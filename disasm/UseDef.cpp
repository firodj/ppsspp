#include <iostream>

#include "Core/MIPS/MIPSDebugInterface.h"

#include "UseDef.hpp"
#include "BasicBlock.hpp"
#include "MyInstruction.hpp"
#include "MyDocument.hpp"

struct UseDefTableRow {
  const char *mnemonic;
  void (UseDefAnalyzer::*funcUseDef)(UseDefManager &mgr, MyInstruction *instr);
};

static const std::map<std::string, UseDefTableRow> mapTables = {
  { "addiu", { "addiu", &UseDefAnalyzer::DoAssign } },
  { "lui", { "lui", &UseDefAnalyzer::DoAssign } },
  { "li", { "li", &UseDefAnalyzer::DoAssign } },
  { "lw", { "lw", &UseDefAnalyzer::DoAssign } },
  { "move", { "move", &UseDefAnalyzer::DoAssign } },
  { "ori", { "ori", &UseDefAnalyzer::DoAssign } },
  { "or", { "or", &UseDefAnalyzer::DoAssign } },
  { "xor", { "xor", &UseDefAnalyzer::DoAssign } },
  { "sll", { "sll", &UseDefAnalyzer::DoAssign } },

  { "sw", { "sw", &UseDefAnalyzer::DoStore } },

  { "beq", { "beq", &UseDefAnalyzer::DoAllUse } },
  { "nop", { "nop", &UseDefAnalyzer::DoAllUse } },
  { "jr", { "jr", &UseDefAnalyzer::DoAllUse } },
  { "j", { "j", &UseDefAnalyzer::DoAllUse } },

  { "jal", { "jal", &UseDefAnalyzer::DoDefRa } },

  { "syscall", { "syscall", &UseDefAnalyzer::DoSysCall }}
};

//--- MyDef ---

MyDef::MyDef(std::string name, int version):
  name_(name), version_(version), instr_(nullptr)
{
}

void MyDef::Dump() {
  std::cout << "\t-- def: ";
  std::cout << name_ << " (" << std::dec << version_ << ") ";
  std::cout << "@" << std::hex << instr_addr_ << ": " << instr_->dizz_ << " arg:" << std::dec << arg_num_;
  std::cout << std::endl;
}

//--- MyUse ---

MyUse::MyUse(std::string name, int version):
  name_(name), version_(version), instr_(nullptr)
{
}

void MyUse::Dump() {
  std::cout << "\t-- use: ";
  std::cout << name_ << " (" << std::dec << version_ << ") ";
  std::cout << "@" << std::hex << instr_addr_ << ": " << instr_->dizz_ << " arg:" << std::dec << arg_num_;
  std::cout << std::endl;
}

// --- UseDefManager ---

UseDefManager::UseDefManager(BasicBlock *bb): bb_(bb)
{
}

void UseDefManager::Reset()
{
  defs_.clear();
  uses_.clear();
}

MyDef&
UseDefManager::AddDef(std::string name, MyInstruction *instr, int arg_num)
{
  if (name.empty()) {
    std::cout << "ERROR\tAddDef\tname empty" << std::endl;
  }

  int ver = defs_[name].size();
  defs_[name].emplace_back(name, ver);

  MyDef &myDef = defs_[name].back();
  myDef.instr_ = instr;
  myDef.instr_addr_ = instr->addr_;
  myDef.arg_num_ = arg_num;

  return myDef;
}

MyUse&
UseDefManager::AddUse(std::string name, MyInstruction *instr, int arg_num)
{
  if (name.empty()) {
    std::cout << "ERROR\tAddUse\tname empty" << std::endl;
  }

  int def_ver = -1;
  if (defs_.find(name) != defs_.end())
    def_ver = defs_[name].size() -1;

  for (MyUse &myUse: uses_[name]) {
    if (myUse.instr_addr_ == instr->addr_ && myUse.arg_num_ == arg_num) return myUse;
  }
  uses_[name].emplace_back(name, def_ver);

  MyUse &myUse = uses_[name].back();
  myUse.instr_ = instr;
  myUse.instr_addr_ = instr->addr_;
  myUse.arg_num_ = arg_num;

  return myUse;
}

void
UseDefManager::Dump() {
  std::cout << "  --> BB use:";
  for (auto &a_uses: uses_) {
    for (MyUse &myUse: a_uses.second) {
      if (myUse.version_ == -1) {
        std::cout << " " << myUse.name_;
        break;
      }
    }
  }
  std::cout << std::endl;

  std::cout << "  --> BB def:";
  for (auto &a_defs: defs_) {
    // MyDef &myDef = a_defs.second.back();
    std::cout << " " << a_defs.first;
  }
  std::cout << std::endl;
}

//--- UseDefAnalyzer ---

UseDefAnalyzer::UseDefAnalyzer(MyDocument *doc): doc_(doc) {
}

void UseDefAnalyzer::DoDefRa(UseDefManager &mgr, MyInstruction *instr)
{
  auto &myDef = mgr.AddDef("ra", instr, -1);
  //myDef.Dump();
}

void
UseDefAnalyzer::DoStore(UseDefManager &mgr, MyInstruction* instr) {
  int arg_i;

  if (instr->arguments_.size() != 2) {
    std::cout << "ERROR\tDoStore\tArgument size mismatch: " << instr->dizz_ << std::endl;
  }

  arg_i = 0;
  if (!instr->arguments_[arg_i].IsZero() && instr->arguments_[arg_i].type_ == ArgReg) {
    auto &myUse = mgr.AddUse(instr->arguments_[arg_i].reg_, instr, arg_i);
    //myUse.Dump();
  } else if (instr->arguments_[arg_i].type_ == ArgMem) {
    auto &myUse = mgr.AddUse(instr->arguments_[arg_i].reg_, instr, arg_i);
    //myUse.Dump();
  }

  arg_i = 1;
  if (instr->arguments_[arg_i].type_ == ArgMem) {
    auto &myUse = mgr.AddUse(instr->arguments_[arg_i].reg_, instr, arg_i);
    //myUse.Dump();
  }
}

void UseDefAnalyzer::DoAssign(UseDefManager &mgr, MyInstruction *instr) {
  int arg_i;
  for (arg_i=instr->arguments_.size() -1; arg_i>0; arg_i--) {
    if (instr->arguments_[arg_i].IsZero()) continue;

    if (instr->arguments_[arg_i].type_ == ArgReg) {
      auto &myUse = mgr.AddUse(instr->arguments_[arg_i].reg_, instr, arg_i);
      //myUse.Dump();
    } else if (instr->arguments_[arg_i].type_ == ArgMem) {
      auto &myUse = mgr.AddUse(instr->arguments_[arg_i].reg_, instr, arg_i);
      //myUse.Dump();
    }
  }

  if (instr->arguments_[arg_i].type_ == ArgReg) {
    auto &myDef = mgr.AddDef(instr->arguments_[arg_i].reg_, instr, arg_i);
    //myDef.Dump();
  }
}

void UseDefAnalyzer::DoAllUse(UseDefManager &mgr, MyInstruction *instr) {
  int arg_i;
  for (arg_i=instr->arguments_.size() -1; arg_i>= 0; arg_i--) {
    if (instr->arguments_[arg_i].IsZero()) continue;

    if (instr->arguments_[arg_i].type_ == ArgReg) {
      auto &myUse = mgr.AddUse(instr->arguments_[arg_i].reg_, instr, arg_i);
      //myUse.Dump();
    } else if (instr->arguments_[arg_i].type_ == ArgMem) {
      auto &myUse = mgr.AddUse(instr->arguments_[arg_i].reg_, instr, arg_i);
      //myUse.Dump();
    }
  }
}

void UseDefAnalyzer::DoSysCall(UseDefManager &mgr, MyInstruction *instr)
{
  MyHLEFunction *hlefun = doc_->GetFunc(instr->arguments_[0].Str());
  for (int ret_i = 0; ret_i < hlefun->retmask.size() && ret_i < 2; ret_i++) {
    mgr.AddDef(currentDebugMIPS->GetRegName(0, 2 + ret_i), instr, -1);
  }
  for (int arg_i = 0; arg_i < hlefun->argmask.size(); arg_i++) {
    mgr.AddUse(currentDebugMIPS->GetRegName(0, 4 + arg_i), instr, -1);
  }
}

void UseDefAnalyzer::AnalyzeInstruction(MyInstruction *instr) {
  auto it = mapTables.find(instr->mnemonic_);
  if (it == mapTables.end()) {
    std::cout << "ERROR\tUseDefManager::AnalyzeInstruction\tmissing mnemonic: " << instr->dizz_ << std::endl;
    return;
  }

  BasicBlock *bb = doc_->bbManager_.FetchBasicBlock(instr->addr_);

  (this->*it->second.funcUseDef)(bb->defUseManager_, instr);
}
