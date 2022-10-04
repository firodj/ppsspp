#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <fstream>
#include <array>
#include <map>
#include <string>
#include <stdexcept>
#include <functional>
#include <ostream>
#include <queue>

#include "MyDocument.hpp"

#include "Common/CommonTypes.h"
#include "Core/MIPS/MIPSDebugInterface.h"
// #include "Core/Debugger/DisassemblyManager.h"
#include "Core/Debugger/MemBlockInfo.h"
#include "Core/System.h"
#include "Core/Debugger/SymbolMap.h"
#include "Core/MIPS/MIPS.h"
#include "Core/MIPS/MIPSTables.h"
#include "Core/MIPS/MIPSAnalyst.h"
#include "Core/MemMap.h"

#include "Yaml_internal.hpp"

extern MyDocument* g_currentDocument;

//--- MemoryDump ---//

MemoryDump::MemoryDump(): data_(nullptr), length_(0)
{}

MemoryDump::~MemoryDump() {
	if (data_) delete[] data_;
}

void
MemoryDump::Allocate(size_t length) {
	length_ = length;
	data_ = new u8[length];
}

//--- MyDocumentInternal ---//

const char*
MyDocument::GetFuncName(int moduleIndex, int func) {
	static char temp[256] = { 0 };
	if (moduleDB_.size() > moduleIndex) {
		auto &mod = moduleDB_[moduleIndex];
		if (mod.funcs.size() > func) {
			auto &fun = mod.funcs[func];
			sprintf(temp, "%s::%s", mod.name.c_str(), fun.name.c_str());
			return temp;
		}
	}

	sprintf(temp, "HLE(%x,%x)", moduleIndex, func);
	return temp;
}

MyHLEFunction *
MyDocument::GetFunc(std::string fullname) {
	auto marker = fullname.find("::");
	if (marker == std::string::npos) return nullptr;

	std::string modulename = fullname.substr(0, marker);

	std::string funcname = fullname.substr(marker + 2);

	for (auto &module: moduleDB_) {
		if (module.name == modulename) {
			for (auto &func: module.funcs) {
				if (func.name == funcname) {
					return &func;
				}
			}
		}
	}
	return nullptr;
}

//--- MyDocument ---//

size_t MyDocument::MemorySize() { return buf_.length_; }

u8* MyDocument::MemoryPtr() { return buf_.data_; }

u32 MyDocument::MemoryStart() { return memory_start_;  }

MyHLEModules &MyDocument::hleModules() { return moduleDB_; }

MyDocument::MyDocument(): bbtrace_parser_(this), memory_start_(0),
  useDefAnalyzer_(this) {
  bbManager_.instrManager(&instrManager_);
}

MyDocument::~MyDocument() {
	Uninit();
}

u32 MyDocument::GetFunctionStart(u32 address) {
	u32 addr = symbol_map_.GetFunctionStart(address);
	return addr == SymbolMap::INVALID_ADDRESS ? 0 : addr;
}

std::string MyDocument::GetLabelString(u32 address) {
	return symbol_map_.GetLabelString(address);
}

MyFunction *MyDocument::CreateNewFunction(u32 addr, u32 last_addr) {
	char funcname[256];

	MyFunction *func = funcManager_.CreateFunction(addr);
  if (!func) return nullptr;

  func->last_addr_ = last_addr;
	std::snprintf(funcname, 256, "z_un_%08x", func->addr_);
	func->name_ = funcname;

	u32 fn_size = func->last_addr_ - func->addr_ + 4;
	symbol_map_.AddFunction(funcname, func->addr_, fn_size);
	funcManager_.RegisterNameToAddress(func->name_, func->addr_);

	return func;
}

MyFunction *MyDocument::SplitFunctionAt(u32 split_addr, MyFunction **OUT_prev_func) {
	u32 fn_start = GetFunctionStart(split_addr);
	MyFunction *funcStart = funcManager_.GetFunction(split_addr);

	if (!fn_start) {
		if (!funcStart) {
			std::cout << "TODO\tunimplemented create new function" << std::endl;
			return nullptr;
		} else {
			return nullptr;
		}
	}

	funcStart = funcManager_.GetFunction(fn_start);
	if (OUT_prev_func) {
		*OUT_prev_func = funcStart;
	}

  u32 last_addr = funcStart->last_addr_;
  if (funcStart->last_addr_ >= split_addr)
    funcStart->last_addr_ = split_addr - 4;

	MyFunction *split_func = CreateNewFunction(split_addr, last_addr);

  if (!split_func) {
    funcStart->last_addr_ = last_addr;
    std::cout << "ERROR\tUnable to create splitted func at: " << split_addr << std::endl;
    return nullptr;
  }

	u32 funcStart_size = funcStart->last_addr_ - funcStart->addr_ + 4;
	symbol_map_.SetFunctionSize(funcStart->addr_, funcStart_size);

  return split_func;
}

void ParseArgTypes(const char *mask, int n, FuncArgTypes &types) {
	for (int i=0; i<n; i++, mask++) {
		const char *t;
		switch (*mask) {
			case 'x': t = "u32"; break;
			case 'i': t = "s32"; break;
			case 'f': t = "float"; break;
			case 'X': t = "u64"; break;
			case 'I': t = "s64"; break;
			case 'F': t = "double"; break;
			case 's': t = "const char*"; break;
			case 'p': t = "u32*";  break;
			case '?': t = "void*"; break;
			default: return;
		}
		types.push_back(t);
	}
}

void MyDocument::Uninit() {
	if (g_currentDocument == this) g_currentDocument = nullptr;
	if (g_symbolMap == &symbol_map_) g_symbolMap = nullptr;
}

void LoadFunctionsFromYaml(MyDocument &self, Yaml::Node & node_functions) {
	for(auto it = node_functions.Begin(); it != node_functions.End(); it++) {
		Yaml::Node & node_func = (*it).second;

		std::string fn_name = node_func["name"].As<std::string>();
		u32 fn_address = node_func["address"].As<u32>();
		u32 fn_last_address = 0, fn_size = 0;

		if (! node_func["last_address"].IsNone()) {
			fn_last_address = node_func["last_address"].As<u32>();
			fn_size = fn_last_address - fn_address + 4;
		}
		if (! node_func["size"].IsNone()) {
			fn_size = node_func["size"].As<u32>();
			fn_last_address = fn_address + fn_size - 4;
		}

		self.symbol_map().AddFunction(fn_name.c_str(), fn_address, fn_size);

		MyFunction *func = self.funcManager_.CreateFunction(fn_address);
		if (!func) {
			std::cout << "ERROR\nMyDocument::LoadFunctionsFromYaml\tUnable to add function: " << fn_name << std::endl;
			continue;
		} else {
			func->last_addr_ = fn_last_address;
			func->name_ = fn_name;
		}
		self.funcManager_.RegisterNameToAddress(fn_name, fn_address);

		if (! node_func["bb_addresses"].IsNone()) {
			Yaml::Node & bb_addresses = node_func["bb_addresses"];
			for(auto bb_it = bb_addresses.Begin(); bb_it != bb_addresses.End(); bb_it++) {
				u32 addr = (*bb_it).second.As<u32>();
				auto *bb = self.bbManager_.FetchBasicBlock(addr);
				if (!bb) {
					std::cout << "Error\tMissing bb:" << addr << " for func:" << func->name_ << std::endl;
					continue;
				}
				func->bb_addrs_.push_back(addr);

				bb->AddFuncRef(func->addr_);
			}
		}
	}
}

int MyDocument::Init(std::string path, bool load_analyzed)
{
	std::string path_yaml = path;
	path_yaml += "/Sora.yaml";
	std::string path_data = path;
	path_data += "/SoraMemory.bin";
	std::string bb_data = path;
	bb_data += "/SoraBBTrace.rec";
	std::string path_anal = path;
	path_anal += "/SoraAnalyzed.yaml";

	bbtrace_parser_.filename_ = bb_data;

	if (g_symbolMap) {
		std::cout << "WARNING:\tMyDocument::Init\tOverride g_symbolMap" << std::endl;
	}
	g_symbolMap = &symbol_map_;
	if (g_currentDocument) {
		std::cout << "WARNING:\tMyDocument::Init\tOverride g_currentDocument" << std::endl;
	}
	g_currentDocument = this;

	Yaml::Node root;
	try
	{
		Yaml::Parse(root, path_yaml.c_str());
	}
	catch (const Yaml::Exception e)
	{
		std::cout << "ERROR:\nMyDocument::Init#path\nexception " << e.Type() << ": " << e.what() << std::endl;
		std::cout << path_yaml.c_str() << std::endl;
		return EXIT_FAILURE;
	}

	Yaml::Node root_anal;
	if (load_analyzed) {
		try
		{
			Yaml::Parse(root_anal, path_anal.c_str());
		}
		catch (const Yaml::Exception e)
		{
			std::cout << "WARNING:\nMyDocument::Init#analyzed\nexception " << e.Type() << ": " << e.what() << std::endl;
			std::cout << path_anal.c_str() << std::endl;
			load_analyzed = false;
		}
	}

	memory_start_ = root["memory"]["start"].As<u32>();
	u32 memory_size = root["memory"]["start"].As<u32>();
	entry_addr_ = root["module"]["nm"]["entry_addr"].As<u32>();

	buf_.Allocate(memory_size);
	std::ifstream bin(path_data, std::ios::binary);
	bin.read((char*)buf_.data_, memory_size);
	bin.close();
	Memory::base = buf_.data_ - memory_start_;

	Yaml::Node & node_modules = root["modules"];
	for(auto it = node_modules.Begin(); it != node_modules.End(); it++) {
		Yaml::Node & node_modu = (*it).second;

		std::string fn_name = node_modu["name"].As<std::string>();
		u32 fn_address = node_modu["address"].As<u32>();
		u32 fn_size = node_modu["size"].As<u32>();

		symbol_map_.AddModule(fn_name.c_str(), fn_address, fn_size);
	}

	if (load_analyzed) {
		Yaml::Node & basic_blocks = root_anal["basic_blocks"];
		if (basic_blocks.Size()) {
			for(auto it = basic_blocks.Begin(); it != basic_blocks.End(); it++) {
				Yaml::Node & bblock = (*it).second;
				u32 bb_addr = bblock["address"].As<u32>();

				auto bb = bbManager_.Create(bb_addr);
				if (!bb) {
					std::cout << "ERROR:\nLoading BB:" << bb_addr << std::endl;
					continue;
				}

				bb->last_addr_ = bblock["last_address"].As<u32>();
				bb->branch_instr_ = bblock["branch_adress"].As<u32>();
			}
		}

		Yaml::Node & basic_block_refs = root_anal["basic_block_refs"];
		if (basic_block_refs.Size()) {
			for(auto it = basic_block_refs.Begin(); it != basic_block_refs.End(); it++) {
				Yaml::Node & bblockref = (*it).second;
				u32 from_addr = bblockref["from"].As<u32>();
				u32 to_addr = bblockref["to"].As<u32>();
				u32 flags = bblockref["flags"].As<u32>();

				auto &bbref = bbManager_.CreateReference(from_addr, to_addr);
				bbref.flags_ = flags;
			}
		}
	}

	Yaml::Node *node_functions = &root["functions"];
	if (load_analyzed)
		node_functions = &root_anal["functions"];
	LoadFunctionsFromYaml(*this, *node_functions);

	Yaml::Node & node_hle_modules = root["hle_modules"];
	for(auto it = node_hle_modules.Begin(); it != node_hle_modules.End(); it++) {
		Yaml::Node & node_hle_module = (*it).second;

		std::string fn_name = node_hle_module["name"].As<std::string>();
		MyHLEModule hlemod = {
			.name = fn_name
		};

		Yaml::Node & fn_funcs = node_hle_module["funcs"];
		if (fn_funcs.IsSequence()) {
			for(auto it2 = fn_funcs.Begin(); it2 != fn_funcs.End(); it2++) {
				Yaml::Node & node_hle_func = (*it2).second;

				MyHLEFunction hlefun = {
					.name = node_hle_func["name"].As<std::string>(),
					.nid = node_hle_func["nid"].As<u32>(),
					.flags = node_hle_func["flags"].As<u32>(0),
				};

				std::string retmask = node_hle_func["retmask"].As<std::string>();
				ParseArgTypes(retmask.c_str(), retmask.size(), hlefun.retmask);

				std::string argmask = node_hle_func["argmask"].As<std::string>("");
				ParseArgTypes(argmask.c_str(), argmask.size(), hlefun.argmask);

				hlemod.funcs.push_back(hlefun);
			}
		}

		moduleDB_.push_back(hlemod);
	}

	entry_name_ = g_symbolMap->GetLabelString(entry_addr_);

  return EXIT_SUCCESS;
}

// maybe deprecated? will move use def after pseudo level
void
MyDocument::BBProcessUseDef(BasicBlock *bb) {
	bb->defUseManager_.Reset();
	for (u32 addr=bb->addr_; addr <=bb->last_addr_; addr += 4) {
		if (!instrManager_.InstrIsExists(addr)) break;

		MyInstruction* instr = instrManager_.FetchInstruction(addr);
		useDefAnalyzer_.AnalyzeInstruction(instr);
	}
}

int MyDocument::DisasmRangeTo(std::function<void(DisasmParam)> cb, u32 start_addr, u32 last_addr) {
	MyInstruction *prevInstr = nullptr;
	Instructions lines;
	u32 bb_addr = 0, br_addr = 0;
	int bb_count = 0;

	for (u32 addr = start_addr; last_addr == 0 || addr <= last_addr; addr += 4) {
		if (bb_addr == 0) bb_addr = addr;

		MyInstruction *instr = Disasm(addr);
		if (!instr) {
			std::cout << "ERROR\tUnable to Disasm instr at: " << addr << std::endl;
		}
		lines.push_back(instr);

		if (instr->info_.isBranch) {
			br_addr = addr;
			if (!instr->info_.hasDelaySlot) {
				std::cout << "WARNING\tDisasmRange\tUnhandled no delay shot jump" << std::endl;

				DisasmParam param = {
					.bb_addr = bb_addr,
					.lines = lines,
					.br_addr = br_addr,
					.last_addr = addr,
				};

				cb(param);
				bb_addr = 0;
				br_addr = 0;
				lines.clear();
				bb_count++;

				if (last_addr == 0 && instr->info_.isConditional) break;
			}
		}
		if (prevInstr && prevInstr->info_.hasDelaySlot) {
			DisasmParam param = {
				.bb_addr = bb_addr,
				.lines = lines,
				.br_addr = br_addr,
				.last_addr = addr,
			};

			cb(param);

			bb_addr = 0;
			br_addr = 0;
			lines.clear();
			bb_count++;

			if (last_addr == 0 && !prevInstr->info_.isConditional) break;
		}
		prevInstr = instr;
	}

	if (!lines.empty()) {
		DisasmParam param = {
			.bb_addr = bb_addr,
			.lines = lines,
			.br_addr = br_addr,
			.last_addr = last_addr,
		};
		cb(param);

		bb_count++;
	}

	return bb_count;
}

MyInstruction*
MyDocument::Disasm(u32 addr)  {
	char dizz[256];

	if (!Memory::IsValidAddress(addr)) return nullptr;
	if (instrManager_.InstrIsExists(addr)) return instrManager_.GetInstruction(addr);
	MyInstruction* instr = instrManager_.CreateInstruction(addr);
	if (!instr) return nullptr;

	MIPSAnalyst::MipsOpcodeInfo mips_info =
		MIPSAnalyst::GetOpcodeInfo(currentDebugMIPS, addr);
	std::memcpy(&instr->info_, &mips_info, sizeof(instr->info_));

	// Implemetation from: currentDebugMIPS->disasm(instr->addr, 4)
	bool tabsToSpaces = false;
	MIPSDisAsm(mips_info.encodedOpcode, mips_info.opcodeAddress, dizz,
		tabsToSpaces, &instr->encodingLog);

	instr->ParseDisasm(dizz);

	return instr;
}

void
MyDocument::SaveAnalyzed(std::string path) {
	std::string path_anal = path;
	path_anal += "/SoraAnalyzed.yaml";

	std::ostream *outputSelector = &std::cout;

	std::fstream fs;
	fs.open(path_anal, std::fstream::out);
	if (fs.good()) {
		outputSelector = &fs;
		std::cout << "INFO\tWriting to: " << path_anal << std::endl;
	} else {
		std::cout << "ERROR\tWriting to: " << path_anal << std::endl;
	}

	std::ostream &write = *outputSelector;

	write << "basic_blocks:" << std::endl;

	for (auto it = bbManager_.BBBegin(); it != bbManager_.BBEnd(); it++) {
		BasicBlock *bb = it->second.get();
		write << "  - address: 0x" << std::hex << bb->addr_ << '\n';
		write << "    last_address: 0x" << std::hex << bb->last_addr_ << '\n';
		write << "    branch_adress: 0x" << std::hex << bb->branch_instr_ << std::endl;
	}

	write << "basic_block_refs:" << std::endl;
	for (auto it = bbManager_.references_.begin(); it != bbManager_.references_.end(); it++) {
		write << "  - from: 0x" << std::hex << it->first.first << '\n';
		write << "    to: 0x" << std::hex << it->first.second << '\n';
		write << "    flags: 0x" << std::hex << (unsigned)it->second.flags_ << std::endl;
	}

	write << "functions:" << std::endl;

  for (auto it = funcManager_.FNBegin(); it != funcManager_.FNEnd(); it++) {
    MyFunction *func = it->second.get();

		write << "  - name: " << func->name_  << '\n';
		write << "    address: 0x" << std::hex << func->addr_  << '\n';
		write << "    last_address: 0x" << std::hex << func->last_addr_  << '\n';

		if (func->bb_addrs_.size() > 0) {
			write << "    bb_addresses:";

			int j;
			for (j=0; j<func->bb_addrs_.size(); j++) {
				write << '\n';
				write << "      - 0x" << std::hex << func->bb_addrs_[j];
			}

			write << std::endl;
		}
  }

	fs.close();
}

// This function will check visited `func->bb_addrs_` start from
// bb func->addr_.
void
MyDocument::ProcessAnalyzedFunc(MyFunction *func)
{
	struct BasicBlockVisit {
		BasicBlockVisit(): bb(nullptr), visited(0) {};
		BasicBlock *bb;

		int visited;
	};

	std::map<u32, BasicBlockVisit> blocks;
	Addresses emptyAddrs;
	int j;
	for (auto addr: func->bb_addrs_) {
		blocks[addr].bb = bbManager_.Get(addr);
		blocks[addr].visited = 0;
	}

	std::queue<u32> bb_queues;
	bb_queues.push(func->addr_);

	auto print_disasm = [&](DisasmParam param) {
		std::cout << std::hex << param.bb_addr << ":" << std::endl;
		for (auto it = param.lines.begin(); it != param.lines.end(); it++) {
			MyInstruction* instr = *it;
#if 0
			std::cout << '\t' << (*it)->AsString() << std::endl;
#else
			int ret = DumpPseudo(instr->addr_);

			if (ret == +1) it++; // hasDelayShot
			if (ret == -1) {
				// not implemented mnemonic,
			}
#endif
		}
		std::cout << std::hex << param.last_addr << "." << std::endl;
	};

	while (bb_queues.size()) {
		u32 cur_addr = bb_queues.front();
		bb_queues.pop();

		if (blocks.find(cur_addr) == blocks.end()) {
			if (!funcManager_.IsExists(cur_addr))
				std::cout << "don't know: " << std::hex << cur_addr << std::endl;
			continue;
		}

		BasicBlockVisit *cur_visit = &blocks[cur_addr];

		if (cur_visit->visited) continue;
		cur_visit->visited++;

		std::cout << "---" << std::endl;
		// std::cout << "BB: " <<std::hex << cur_visit->bb->addr_ << std::endl;
		DisasmRangeTo(print_disasm, cur_visit->bb->addr_, cur_visit->bb->last_addr_);

		if (cur_visit->bb->branch_instr_) {
			MyInstruction *BrInstr = instrManager_.GetInstruction(cur_visit->bb->branch_instr_);
			if (!BrInstr) {
				std::cout << "WARNING\tProcessAnalyzedFunc\t"
					<< "missing branch instr for BB 0x" << std::hex << cur_addr
					<< " at 0x" << cur_visit->bb->branch_instr_ << std::endl;
				//FIXME: bb_queues.push(cur_visit->bb->last_addr_ + 4);
			}
			/**
				else if (BrInstr->info_.isConditional || BrInstr->info_.isLinkedBranch) {
				std::cout << "conditional: " << BrInstr->mnemonic_ << std::endl;
				bb_queues.push(cur_visit->bb->last_addr_ + 4);
			*/
			else if (BrInstr->info_.isBranchToRegister && BrInstr->arguments_[0].reg_ == "ra") {
				std::cout << "INFO\tProcessAnalyzedFunc\treturn from BB 0x" << std::hex << cur_addr << std::endl;
			}
		}

		Addresses *nexts = bbManager_.RefsTo(cur_addr);

		if (!nexts) {
			std::cout << "no to-refs from BB: " << cur_addr << std::endl;
		} else {
			for (auto next_addr: *nexts) {
				bb_queues.push(next_addr);
			}
		}
	}

	for (auto &bb_visit: blocks) {
		if (bb_visit.second.visited == 0) {
			std::cout << "WARNING: Unvisited bb: " << std::hex << bb_visit.first
				<< " for func: " << func->name_ << std::endl;
		}
	}
}