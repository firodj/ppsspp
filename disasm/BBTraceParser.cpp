#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <queue>

#include "Types.hpp"
#include "MyDocument.hpp"

#include "BBTraceParser.hpp"
#include "Core/BBTrace.h"

//--- BBTThreadState ---
const std::string&
BBTThreadState::name() {
	return name_;
}

void
BBTThreadState::name(std::string str) {
	name_ = str;
	flame_graph.name_ = str;
}

//--- BBTraceParser ---

BBTraceParser::BBTraceParser(MyDocument *doc): doc_(doc) {
	currentID_ = 0;
	nts_ = 0;
	fts_ = 0;
}

void BBTraceParser::Parse(std::function<void(BBTParseParam)> func, int length) {
	std::ifstream bin(filename_, std::ios::binary);
	std::vector<u32> record;

	bool ok = true;
	//refts nts = 1;
	nts_ = 1;
	fts_ = 1;

	if (length < 1) length = 1;

	u16 cur_ID = 0;

	while (!bin.eof() && ok) {
		u16 kind, last_kind;
		u32 size;

		bin.read((char*)&kind, sizeof(kind));
		if (bin.eof()) break;

		if (kind != KIND_ID) {
			std::cout << "ERROR: unmatched kind 'ID', found:" << std::hex << kind << std::endl;
			break;
		}
		bin.read((char*)&cur_ID, sizeof(cur_ID));
		bin.read((char*)&kind, sizeof(kind));
		if (kind != KIND_SZ) {
			std::cout << "ERROR: unmatched kind 'SZ'" << std::endl;
			break;
		}
		bin.read((char*)&size, sizeof(size));

		std::cout << "ID:" << std::dec << cur_ID << " size:" << std::dec << size << std::endl;
		SetCurrentThread(cur_ID);
		threads_[cur_ID].flame_graph.AddMarker(nts_, threads_[cur_ID].name());
		threads_[cur_ID].flame_graph.fts_ = fts_;

		record.resize(size);
		bin.read((char*)&record[0], size * sizeof(u32));

		for (int i=0; i<size; i++) {
			last_kind = kind;
			u32 last_pc = 0;
			u32 pc = record[i];
			// if (pc == 0x8a0d88c) shown_ = true;

      if ((pc & 0xFFFF0000) == 0) {
        kind = pc & 0xFFFF;

        if (kind == KIND_START) {
          pc = record[++i];

          std::cout << "START:" << std::hex << pc << std::endl;
					SetCurrentThreadPC(cur_ID, pc);
				} else if (kind == KIND_NAME) {
					char name[32 + 1] = {0};
					for (int j = 0; j < 32; j += 4)
						*(u32*)&name[j] = record[++i];
					std::cout << name << std::endl;
					if (last_kind == KIND_START) {
						threads_[cur_ID].name(name);
					}
					if (threads_[cur_ID].name() == "idle0") threads_[cur_ID].executing = false;
					if (threads_[cur_ID].name() == "idle1") threads_[cur_ID].executing = false;
					if (threads_[cur_ID].name() == "SceIoAsync") threads_[cur_ID].executing = false;
				} else {
          std::cout << "ERROR\tUnknown kind: 0x" << std::hex << pc << std::endl;
				}
				continue;
      }

			last_pc = record[++i];

			if (threads_[cur_ID].executing) {
				BBTParseParam param;
				param.ID = cur_ID;
				param.kind = 0;
				param.pc = pc;
				param.last_pc = last_pc;
				param.nts = nts_;

				if (func)
					func(param);
			}

			nts_++;
			length -= 1; if (length == 0) { ok = false; break; }
		}

		threads_[cur_ID].flame_graph.AddMarker(nts_, threads_[cur_ID].name());
		fts_ = threads_[cur_ID].flame_graph.fts_;
	}

	bin.close();
}

void BBTraceParser::SetCurrentThread(u16 ID) {
	if (currentID_ == 0 || currentID_ != ID) {
		currentID_ = ID;
		if (threads_.find(currentID_) == threads_.end()) {
			threads_[currentID_].ID = currentID_;
			threads_[currentID_].reg_sp = 0;
			threads_[currentID_].pc = 0;
			threads_[currentID_].executing = true;
		}
	}
}

u32 BBTraceParser::SetCurrentThreadPC(u16 ID, u32 addr) {
	SetCurrentThread(ID);

	u32 last_pc = threads_[currentID_].pc;
	threads_[currentID_].pc = addr;
	return last_pc;
}

void OnEachBB(BBTraceParser &self, DisasmParam &disparam) {
	BasicBlock *newBB = self.doc()->bbManager_.Get(disparam.bb_addr);
	if (!newBB) {
		newBB = self.doc()->bbManager_.Create(disparam.bb_addr);
		if (newBB) {
			newBB->last_addr_ = disparam.last_addr;
			newBB->branch_instr_ = disparam.br_addr;
		} else {
			std::cout << "ERROR\tUnable to create BB at: " << disparam.bb_addr << std::endl;
			return;
		}
	} else if (newBB->addr_ != disparam.bb_addr) {
		std::cout << "ERROR\nFix me to split bb during OnEachBB at: " << disparam.bb_addr << std::endl;
		return;
	}
}

void OnEnterFunc(BBTraceParser &self, BasicBlock *theBB, u32 ra) {
	auto &stack = self.threads_[self.currentID_].stack;
	auto &flame_graph = self.threads_[self.currentID_].flame_graph;
	auto &hierarchy = self.threads_[self.currentID_].hierarchy;

	MyFunction *theFunc = self.doc()->funcManager_.GetFunction(theBB->addr_);
	FNTreeNodeID hierarchyParentID = 0;

	if (stack.size() > 0) {
		if (ra == 0) {
			std::cout << "ERROR\ninvalid ra when entering func" << std::endl;
		}
		stack.back().ra = ra;

		int level = stack.size();
		flame_graph.EndBlock(level, self.nts());

		hierarchyParentID = stack.back().hierarchyID;
	}

	BBTStackItem stack_item = {
		.addr = theBB->addr_,
		.ra = 0,
		.func = theFunc,
		.hierarchyID = hierarchy.AddNode(theBB->addr_, hierarchyParentID),
	};
	stack.push_back(stack_item);

	int level = stack.size();
	flame_graph.AddBlock(level, self.nts(), theBB->addr_);

	FNTreeNode *hierarchyNode = hierarchy.At( stack.back().hierarchyID );

	if (self.shown_) {
		std::cout << "enter func bb: " << std::hex << theBB->addr_;
	}

	if (!theFunc) {
		u32 fn_start = self.doc()->GetFunctionStart(theBB->addr_);
		if (fn_start) {
			theFunc = self.doc()->SplitFunctionAt(theBB->addr_,  nullptr);
			if (!theFunc) {
				std::cout << "ERROR\tSplit func" << std::endl;
			} else {
				if (self.shown_)
					std::cout << " name: " << theFunc->name_;
			}
		} else {
			theFunc = self.doc()->CreateNewFunction(theBB->addr_, theBB->last_addr_);
			if (!theFunc) {
				std::cout << "ERROR\tunable to create func in symbol: " << theBB->addr_ << std::endl;
			}
		}
	}

	if (theFunc) {
		hierarchyNode->func_ = theFunc;
		stack.back().func = theFunc;
		stack.back().func->AddBB(theBB->addr_);
		if (self.shown_) {
			u32 ofs = theBB->addr_ - theFunc->addr_;
			std::cout << " name: " << theFunc->name_;
			if (ofs != 0) std::cout << "+" << std::hex << ofs;
		}
	}

	if (self.shown_) {
		std::cout << std::endl;
	}
}

void
OnLeaveFunc(BBTraceParser &self, BasicBlock *theBB) {
	auto &stack = self.threads_[self.currentID_].stack;
	auto &flame_graph = self.threads_[self.currentID_].flame_graph;

	int level = stack.size();
	flame_graph.EndBlock(level, self.nts());

	stack.pop_back();

	if (stack.size() > 0) {
		u32 expected_ra = stack.back().ra;

		if (expected_ra != theBB->addr_) {
			std::cout << "WARNING\tUnexpected RA: " << std::hex << theBB->addr_ << " expected: " << expected_ra << std::endl;
			std::cout << "\t------------------ Possibly CB -----------------------" << std::endl;

			OnEnterFunc(self, theBB, expected_ra);

			//stack.back().func->AddBB(theBB->addr_);
		} else {
			u32 past_bb = stack.back().addr;
			stack.back().addr = theBB->addr_;
			stack.back().func->AddBB(theBB->addr_);

			// std::cout << "call-return " << std::hex << past_bb << " -> " << theBB->addr_ << std::endl;

			self.doc()->bbManager_.CreateReference(past_bb, theBB->addr_).adjacent(true);
			int level = stack.size();
			flame_graph.EndBlock(level, self.nts());
		}
	} else {
		MyFunction *myFunc = self.doc()->funcManager_.GetFunction(theBB->addr_);
		std::cout << "INFO\nend of stack, goto: " << theBB->addr_;
		if (myFunc) {
			std::cout << " name: " << myFunc->name_;
		}
		std::cout << std::endl;
	}
}

void
OnMergingPastToLast(BBTraceParser &self, u32 last_pc)
{
	auto &stack = self.threads_[self.currentID_].stack;

	u32 past_addr = stack.back().addr;

	for (int n=0; n>= 0; n++) {
		BasicBlock *pastBB = self.doc()->bbManager_.Get(past_addr);

		if (!pastBB) {
			std::cout << "ERROR\tOnMergingPastToLast\tpast BB notexist: " << std::hex << past_addr << " towards: " << last_pc << std::endl;
			throw std::runtime_error("no past BB");
			return;
		}

		if (n > 0) {
			stack.back().func->AddBB(pastBB->addr_);
			stack.back().addr = pastBB->addr_;
		} else {
			// ASSERT (stack.back().addr == pastBB->addr_)
			if (stack.back().addr != pastBB->addr_) {
				throw std::runtime_error("assertion failed");
			}
		}

		u32 next_addr = pastBB->last_addr_ + 4;

		if (pastBB->branch_instr_) {
			MyInstruction *pastBrInstr = self.doc()->instrManager_.GetInstruction(pastBB->branch_instr_);

			if (!pastBrInstr) {
				std::cout << "ERROR\tOnMergingPastToLast\tNo branch instr for past BB at: " << pastBB->branch_instr_ << std::endl;
				return;
			}

			// likely instr will skip delayed-shot instruction when not jumping, so
			// it jumping on branch isntr.
			if (pastBrInstr->info_.isLikelyBranch) {
				if (pastBB->branch_instr_ == last_pc) break;
			}

			if (pastBB->last_addr_ == last_pc) break;

			if (pastBrInstr->info_.isConditional) {
				next_addr = pastBB->last_addr_ + 4;
				if (pastBrInstr->info_.isBranchToRegister) {
					std::cout << "WARNING\tunimplemented conditional register branch for merging" << std::endl;
				}
			} else if (pastBrInstr->info_.isBranchToRegister) {
				break;
			}
		}

		self.doc()->bbManager_.CreateReference(pastBB->addr_, next_addr).adjacent(true);

		if (self.shown_)
			std::cout << "  BB: " << std::hex << pastBB->addr_ << " to " << next_addr << std::endl;

		past_addr = next_addr;
	}
}

void
OnContinueNext(BBTraceParser &self, BasicBlock *theBB)
{
	auto &stack = self.threads_[self.currentID_].stack;

	stack.back().addr = theBB->addr_;
	stack.back().func->AddBB(theBB->addr_);
}

BasicBlock*
EnsureBB(BBTraceParser &self, u32 addr) {
	BasicBlock *theBB = self.doc()->bbManager_.Get(addr);

	if (!theBB) {
		self.doc()->DisasmRangeTo([&self](DisasmParam disparam) {
			OnEachBB(self, disparam);
		}, addr, 0);
		theBB = self.doc()->bbManager_.Get(addr);

		if (!theBB) {
			std::cout << "ERROR\tUnable to get BB after creating at: " << addr << std::endl;
			throw std::runtime_error("assertion failed");
		}
	} else
	if (theBB->addr_ != addr) {
		BasicBlock *prevBB = nullptr;
		BasicBlock *splitBB = self.doc()->bbManager_.SplitAt(addr, &prevBB);
		// ASSERT (prevBB == theBB)
		if (prevBB != theBB) {
			throw std::runtime_error("assertion failed");
		}

		theBB = splitBB;
		if (self.shown_)
			std::cout << "split bb at: " << splitBB->addr_ << std::endl;
	}

	return theBB;
}

//static
void BBTraceParser::ParsingBB(BBTraceParser &self, BBTParseParam &par) {
	u32 last_pc = self.SetCurrentThreadPC(par.ID, par.pc);
	//self.nts(par.nts);

	if (self.shown_)
		std::cout << "#" << std::dec << par.nts << " {" << std::hex << par.pc << "," << par.last_pc << "}" << std::endl;

	BasicBlock *lastBB = nullptr;
	MyInstruction *brInstr = nullptr;
	BasicBlock *theBB = EnsureBB(self, par.pc);

	if (par.last_pc == 0) {
		// Usually start thread doesn't have last_pc
		OnEnterFunc(self, theBB, 0);
		return;
	}

	OnMergingPastToLast(self, par.last_pc);

	lastBB = self.doc_->bbManager_.Get(par.last_pc);
	if (!lastBB) {
		std::cout << "ERROR\tParsingBB\tUnable to get last BB: " << std::hex << par.last_pc << " at: " << theBB->addr_ << std::endl;
		throw std::runtime_error("no last bb");
		return;
	}

	brInstr = self.doc_->instrManager_.GetInstruction(lastBB->branch_instr_);
	if (!brInstr) {
		std::cout << "ERROR\tParsingBB\tUnable to get last Instruction at: " << lastBB->branch_instr_ << std::endl;
		return;
	}

	if (self.shown_) {
		int level = self.threads_[self.currentID_].stack.size();
		std::cout << level << "   ";
		std::cout << "From BB: " << par.last_pc << " at " << brInstr->addr_ << " doing " << brInstr->AsString() << " to " << theBB->addr_ << std::endl;
	}

	auto &bbref = self.doc_->bbManager_.CreateReference(lastBB->addr_, theBB->addr_);

	// jump to function
	if (brInstr->mnemonic_ == "jal" || brInstr->mnemonic_ == "jalr") {
		u32 ra = brInstr->addr_ + (brInstr->info_.hasDelaySlot ? 8 : 4);
		OnEnterFunc(self, theBB, ra);
	} else
	// return from function
	if (brInstr->mnemonic_ == "jr" && brInstr->arguments_[0].reg_ == "ra") {
		OnLeaveFunc(self, theBB);
	} else {
		OnContinueNext(self, theBB);
	}
}

void
BBTraceParser::EndParsing() {
	for (auto &thread_it: threads_) {
		thread_it.second.flame_graph.StopAll(thread_it.second.stack.size(), nts_);
	}
}

void
BBTraceParser::RenderFlameGraph() {
	int max_w = 0;
	const int init_y = 200;
	const int padding_y = 40;

	int max_h = init_y - padding_y;
	for (auto &thread_it: threads_) {
		auto &thread = thread_it.second;
		int h = thread.flame_graph.CalcDimHeight();
		int w = thread.flame_graph.CalcDimWidth();
		if (max_w < w) max_w = w;
		max_h += h + padding_y;
	}
	const int rect_size = 16;
	svg::Dimensions dimensions(max_w, max_h);
  svg::Document doc(svg::Layout(dimensions, svg::Layout::TopLeft));

	int base_y = init_y;

	for (auto &thread_it: threads_) {
		auto &thread = thread_it.second;
		thread.flame_graph.RenderSVG(doc, base_y, [this](u32 addr) {
			MyFunction *func = doc_->funcManager_.GetFunction(addr);
			std::string text;
			if (func) {
				text = func->name_;
			} else {
				std::stringstream ss;
				ss << std::hex << addr;
				text = ss.str();
			}
			return text;
		});

		base_y += padding_y;
	}
	doc.save("my_svg.svg");
}

void BBTraceParser::DumpAllHierarchy() {
	for (auto &thread_it: threads_) {
		BBTThreadState &thread = thread_it.second;
		std::cout << "Thread #" << thread.ID << " " << thread.name() << std::endl;
		thread.hierarchy.DumpNode();
	}
}

//-- FNHierarchy --

FNHierarchy::FNHierarchy() {
	nodes_.emplace_back(0);
}

FNTreeNode* FNHierarchy::At(FNTreeNodeID id) {
	if (id >= 1 && id < nodes_.size()) {
		return &nodes_[id];
	}
	return nullptr;
}

FNTreeNodeID FNHierarchy::AddNode(u32 func_addr, FNTreeNodeID parentID)
{
	FNChildren *items = parentID ? &nodes_[parentID].children_ : &first_;

	for (auto it = items->begin(); it != items->end(); it++) {
		FNTreeNode *node = &nodes_[*it];
		if (node->func_addr_ == func_addr) {
			return node->ID_;
		}
	}

	FNTreeNodeID theID = nodes_.size();
	nodes_.emplace_back(func_addr);
	FNTreeNode *node = &nodes_.back();
	node->ID_ = theID;
	node->parentID_ = parentID;
	items->push_back(node->ID_);

	return node->ID_;
}

void FNHierarchy::DumpNode(int level, FNTreeNodeID parentID) {
	FNChildren *items = parentID ? &nodes_[parentID].children_ : &first_;

	for (auto it = items->begin(); it != items->end(); it++) {
		FNTreeNode *node = &nodes_[*it];

		for (int n = level; n > 0; n--) {
			std::cout << "  ";
		}
		std::cout << "+ ";

		if (node->func_) {
			std::cout << node->func_->name_;
		} else {
				std::cout << "0x" << std::hex << node->func_addr_;
		}

		std::cout << std::endl;
		DumpNode(level + 1, node->ID_);
	}
}
