#include <map>
#include <iostream>
#include <sstream>

#include "Core/MIPS/MIPSDebugInterface.h"
#include "Common/CommonTypes.h"
// #include "Core/Debugger/DisassemblyManager.h"
#include "Core/Debugger/MemBlockInfo.h"
#include "Core/System.h"
#include "Core/Debugger/SymbolMap.h"
#include "Core/MIPS/MIPS.h"
#include "Core/MIPS/MIPSTables.h"
#include "Core/MIPS/MIPSAnalyst.h"
#include "Core/MemMap.h"

#include "MyInstruction_internal.hpp"

//--- MyInstruction ---

void MyInstruction::ParseDisasm(const char* disasm)
{
	const char *start = disasm;
	char formatted[256] = {0};

	dizz_ = std::string(disasm);

	// copy opcode
	while (*disasm != 0 && *disasm != '\t') disasm++;
	mnemonic_ = std::string(start, (size_t)(disasm-start));

	if (*disasm++ == 0)	return;

	start = disasm;
	const char* jumpAddress = strstr(disasm,"->$");
	const char* jumpRegister = strstr(disasm,"->");
	bool is_codelocation = false;
	while (*disasm != 0)
	{
		// parse symbol
		if (disasm == jumpAddress) {
			is_codelocation = true;
			MyArgument arg;
			arg.Scan(disasm+3, 8);

			const std::string addressSymbol = g_symbolMap->GetLabelString(arg.value_);
			if (!addressSymbol.empty()) {
				arg.label_ = addressSymbol;
			}
			arg.is_codelocation_ = is_codelocation;

			arguments_.push_back(arg);
			is_codelocation = false;

			disasm += 3+8;
			start = disasm;
			continue;
		}

		if (disasm == jumpRegister) {
			is_codelocation = true;
			disasm += 2;
			start = disasm;
		}

		if (*disasm == ',') {
			MyArgument arg;
			arg.Scan(start, (size_t)(disasm-start));
			arg.is_codelocation_ = is_codelocation;

			arguments_.push_back(arg);
			is_codelocation = false;

			disasm += 2;
			start = disasm;
			continue;
		}

		disasm++;
	}

	size_t  sz = (size_t)(disasm-start);
	if (sz) {
		MyArgument arg;
		arg.Scan(start, sz);
		arg.is_codelocation_ = is_codelocation;

		arguments_.push_back(arg);
	}
}

const std::string &MyInstruction::AsString(bool encinfo) {
	if (as_string_.empty()) {
		std::stringstream ss;

		if (encinfo) {
			ss << std::hex << info_.opcodeAddress;
			ss << '\t';
			ss << std::hex << info_.encodedOpcode;
			ss << '\t';
		}

		ss << mnemonic_;
		if (arguments_.size() > 0) {
			for (int a = 0; a < arguments_.size(); a++) {
				auto &arg = arguments_[a];
				ss << '\t' << arg.Str();
				if (a + 1 < arguments_.size())
					ss << ",";

				if (encinfo)
					ss << " (val:" << std::dec << arg.value_ << ", reg:" << arg.reg_ << ")";
			}
		}

		if (encinfo) {
			ss << "\t; dizz=" << dizz_;
			ss << "\t; log=" << encodingLog;
			ss << "\t; rs=" << std::dec << ((info_.encodedOpcode>>21) & 0x1F);
		}

		as_string_ = ss.str();
	}
	return as_string_;
}

// --- InstructionManager ---

InstructionManager::InstructionManager() {
	internal_ = new InstructionManagerInternal();
}

InstructionManager::~InstructionManager() {
	delete internal_;
}

// deprecated
MyInstruction *InstructionManager::FetchInstruction(u32 addr) {
	if (!addr) return nullptr;

  MyInstruction *instr = nullptr;
  auto it = internal_->instructions_.find(addr);
  if (it == internal_->instructions_.end()) {
		instr = new MyInstruction(addr);
    internal_->instructions_[addr] = InstructionPtr(instr);
  } else {
    instr = it->second.get();
  }

  return instr;
}

MyInstruction* InstructionManager::GetInstruction(u32 addr) {
	if (!addr) return nullptr;

  auto it = internal_->instructions_.find(addr);
  if (it != internal_->instructions_.end()) return it->second.get();

	return nullptr;
}

MyInstruction* InstructionManager::CreateInstruction(u32 addr) {
	if (!addr) return nullptr;

  auto it = internal_->instructions_.find(addr);
  if (it == internal_->instructions_.end()) {
		MyInstruction *instr = new MyInstruction(addr);
    internal_->instructions_[addr] = InstructionPtr(instr);

		return instr;
  }

  return nullptr;
}


bool InstructionManager::InstrIsExists(u32 addr) {
  return internal_->instructions_.find(addr) != internal_->instructions_.end();
}
