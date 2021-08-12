#pragma once

#include <map>
#include <string>
#include <vector>

#include "Types.hpp"

#include "MyArgument.hpp"

typedef struct {
	void* cpu;
	u32 opcodeAddress;
	Opcode encodedOpcode;

	// shared between branches and conditional moves
	bool isConditional;
	bool conditionMet;

	// branches
	u32 branchTarget;
	bool isBranch;
	bool isLinkedBranch;
	bool isLikelyBranch;
	bool isBranchToRegister;
	int branchRegisterNum;
	bool hasDelaySlot;

	// data access
	bool isDataAccess;
	int dataSize;
	u32 dataAddress;

	bool hasRelevantAddress;
	u32 relevantAddress;
} MipsOpcodeInfo;

class MyInstruction {
public:
	u32 addr_;
	std::string dizz_;

	MipsOpcodeInfo info_;

	std::string mnemonic_;
	std::vector<MyArgument> arguments_;

	std::string encodingLog;

	MyInstruction(): addr_(0) {}
	MyInstruction(u32 addr): addr_(addr) {}

	void ParseDisasm(const char* disasm);
	const std::string &AsString(bool encinfo = false);

	std::string as_string_;
};

typedef std::unique_ptr<MyInstruction> InstructionPtr;
typedef std::map<u32, InstructionPtr> MapAddressToInstruction;
typedef std::vector<MyInstruction*> Instructions;

class InstructionManagerInternal;

class InstructionManager {
public:
	InstructionManager();
	~InstructionManager();

	MyInstruction *FetchInstruction(u32 addr); // deprecated
	bool InstrIsExists(u32 addr);

	MyInstruction* GetInstruction(u32 addr);
  MyInstruction* CreateInstruction(u32 addr);

	InstructionManagerInternal *internal() { return internal_; }
private:
	InstructionManagerInternal *internal_;
};

class PendingTarget {
public:
	PendingTarget(u32 target, MyInstruction *instr, int arg_num): instr_(instr),
		target_(target), arg_num_(arg_num)
	{
		instr_addr_ = instr_->addr_;
	}

	u32 target_;
	MyInstruction *instr_;
	u32 instr_addr_;
	int arg_num_;
};

typedef std::vector<PendingTarget> PendingTargets;
