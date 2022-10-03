// NOTES:
// cmake -X Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
//
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <fstream>
#include <array>
#include <map>

#include "argh.h"

#include "Types.hpp"
#include "MyInstruction.hpp"
#include "BasicBlock.hpp"
#include "MyDocument.hpp"

#include "go_bridge.h"

void testSyscall(MyDocument &doc) {
	auto inst = doc.Disasm(0x8A38A70);
	std::cout << inst->AsString() << std::endl;
	inst = doc.Disasm(0x8A38A74);
	std::cout << inst->AsString() << std::endl;
}

void testJump(MyDocument &doc) {
	auto inst = doc.Disasm(0x8804140);
	std::cout << inst->AsString() << std::endl;
}

void testBBTrace(MyDocument &doc) {
	auto print_disasm = [&](DisasmParam param) {
    std::cout << std::hex << param.bb_addr << ":" << std::endl;
    for (auto &s: param.lines) {
      std::cout << '\t' << s->dizz_ << std::endl;
    }
    std::cout << "---" << std::endl;
  };

	doc.bbtrace_parser_.Parse([&](BBTParseParam param) {
		if (!doc.bbManager_.BBIsExists(param.pc)) {

		}
		std::cout << std::dec << param.ID << "| " << std::hex << param.pc << std::endl;

		doc.DisasmRangeTo(print_disasm, param.pc, 0);
	});
}

void show_help(argh::parser &cmdl) {
	std::string app_name;
	cmdl(0) >> app_name;
	std::cout << "Syntax:" << std::endl;
	std::cout << "\t" << app_name << "\t<action>" << std::endl << std::endl;
	std::cout << "Action:" << std::endl;
	std::cout << "\thelp" << "\tShow this help." << std::endl;
	std::cout << "\ttesj" << "\tdisasm testJump." << std::endl;
	std::cout << "\ttess" << "\tdisasm testSyscall." << std::endl;
	std::cout << "\tbbtrace, bbtracetes" << "\tbbtrace parse." << std::endl;
	std::cout << std::endl;
	std::cout << "Option:" << std::endl;
	std::cout << "\t--pc <addr>" << "\tAddress (eg. 0x8000000)" << std::endl;
	std::cout << "\t--fn <name>" << "\tFunc name (eg. start)" << std::endl;
	std::cout << "\t--n <count>" << "\tNumber of bbtrace count to process" << std::endl;
}

int main(int argc, char *argv[])
{
	std::string action_name;
	argh::parser cmdl;
  cmdl.parse(argc, argv, argh::parser::PREFER_PARAM_FOR_UNREG_OPTION);

	if (cmdl(1)) {
		cmdl(1) >> action_name;
		if (action_name == "help") {
			show_help(cmdl);
			return EXIT_SUCCESS;
		}
	}

	const char* env_p;
#ifdef WIN32
	env_p = std::getenv("USERPROFILE");
#else
	env_p = std::getenv("HOME");
#endif
	std::string path = env_p;

	MyDocument myDoc;
	myDoc.Init(path + "/Sora", true);

	if (action_name == "tes") {
		MyArgument arg1, arg2;
		arg1.Scan("-0xa0(a0)", 9);
		std::cout << "a1 = " << arg1.Str() << std::endl;

		arg2.Scan("ra", 2);
		std::cout << "a2 = " << arg2.Str() << std::endl;

		int y = xgox(25);
	} else if (action_name == "tes2") {
		testJump(myDoc);
		testSyscall(myDoc);
	} else if (action_name == "hle") {
		std::cout << "HLE Modules:" << myDoc.hleModules().size() << std::endl;
		int idx = 0;
		for (auto &hle: myDoc.hleModules()) {
			std::cout << idx << ":" << hle.name << std::endl;
			idx++;
		}
	} else if (action_name == "funcs") {
		std::cout << "All functions:" << std::endl;
		for (auto it = myDoc.funcManager_.FNBegin(); it != myDoc.funcManager_.FNEnd(); ++it) {
			std::cout << std::hex << it->first << ": " << it->second.get()->name_ << std::endl;
		}
	} else if (action_name == "bbtracetes") {
		testBBTrace(myDoc);
	} else if (action_name == "bbtraceanal") {
		MyDocument myDoc2;
		myDoc2.Init(path + "/Sora", false);

		std::string n_value;
		int n = 20;
		cmdl("n") >> n_value;
		if (!n_value.empty()) n = std::stoul(n_value, nullptr, 0);

		myDoc2.bbtrace_parser_.shown_ = false;
		myDoc2.bbtrace_parser_.Parse([&](BBTParseParam par) {
			BBTraceParser::ParsingBB(myDoc2.bbtrace_parser_, par);
		}, n);
		myDoc2.bbtrace_parser_.EndParsing();
		myDoc2.SaveAnalyzed(path + "/Sora");
		std::cout << "Store BBs:" << std::dec << myDoc2.bbManager_.BBCount() << std::endl;
    std::cout << "Parse: " << std::dec << myDoc2.bbtrace_parser_.nts() << std::endl;

		std::cout << "All Hierarchy:" << std::endl;
		myDoc2.bbtrace_parser_.DumpAllHierarchy();
	} else if (action_name == "bbtrace") {
		std::string fn_name;
		std::cout << "Loaded BBs:" << std::dec << myDoc.bbManager_.BBCount() << std::endl;
		cmdl("fn") >> fn_name;
		if (fn_name.empty()) fn_name = "start";
		u32 fn_start = myDoc.funcManager_.AddressByName(fn_name);
		if (fn_start) {
			MyFunction *fnStart = myDoc.funcManager_.GetFunction(fn_start);
			myDoc.ProcessAnalyzedFunc(fnStart);
		}
	} else if (action_name == "") {
		std::string fn_name;
		std::string pc_value;
		u32 fn_start = 0;
		cmdl("pc") >> pc_value;
		if (!pc_value.empty()) {
			u32 pc = std::stoul(pc_value, nullptr, 0);
			fn_start = myDoc.GetFunctionStart(pc);
			if (fn_start) {
				fn_name = myDoc.GetLabelString(fn_start);
				std::cout << "for pc: 0x" << std::hex << pc << " the func_name: " << fn_name << std::endl;
			} else {
				std::cout << "no func start at: 0x" << std::hex << pc << std::endl;

				myDoc.DisasmRangeTo([&](DisasmParam dispar) {
					std::cout << dispar.bb_addr << ":" << std::endl;
					for (auto instr: dispar.lines) {
						std::cout << '\t' << instr->AsString() << std::endl;
					}
				}, pc, pc + 0x10 - 4);
			}
		} else {
			cmdl("fn") >> fn_name;
			fn_start = myDoc.funcManager_.AddressByName(fn_name);
			std::cout << "for pc: 0x" << std::hex << fn_start << " the func_name: " << fn_name << std::endl;
		}

		if (fn_start) {
			MyFunction *fnStart = myDoc.funcManager_.GetFunction(fn_start);
			std::cout << "BBs: ";
			for (int j=0; j<fnStart->bb_addrs_.size(); j++) {
				std::cout << ", " << std::hex << fnStart->bb_addrs_[j];
			}
			std::cout << std::endl;

			myDoc.DisasmRangeTo([&](DisasmParam dispar) {
				std::cout << dispar.bb_addr << ":" << std::endl;
				for (auto instr: dispar.lines) {
					std::cout << '\t' << instr->AsString() << std::endl;
				}
			}, fnStart->addr_, fnStart->last_addr_);
			return EXIT_SUCCESS;
		}
	}

	return EXIT_SUCCESS;
}

