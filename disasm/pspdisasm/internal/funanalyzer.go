package internal

import (
	"fmt"

	"github.com/firodj/ppsspp/disasm/pspdisasm/bridge"
)

type FunctionAnalyzer struct {
	doc *SoraDocument
	idx int
}

func NewFunctionAnalyzer(doc *SoraDocument, idx int) *FunctionAnalyzer {
	return &FunctionAnalyzer{
		doc: doc,
		idx: idx,
	}
}

func (anal *FunctionAnalyzer) Process() {
	fun := anal.doc.GetFunctionByIndex(anal.idx)
	if fun == nil {
		fmt.Printf("ERROR:\tno func for index:%d\n", anal.idx)
		return
	}
	var last_addr uint32 = 0
	if fun.LastAddress != nil {
		last_addr = *fun.LastAddress
	}
	anal.ProcessBB(fun.Address, last_addr)
}

type BBAnalState struct {
	BBAddr uint32
	BranchAddr uint32
	Lines []bridge.MipsOpcode
  Count int
}

func (bbas *BBAnalState) Init() {
	bbas.Reset()
	bbas.Count = 0
}

func (bbas *BBAnalState) Reset() {
	bbas.BBAddr = 0
	bbas.BranchAddr = 0
	bbas.Lines = nil
}

func (bbas *BBAnalState) SetBB(addr uint32) {
	if bbas.BBAddr == 0 {
		bbas.BBAddr = addr
	}
}

func (bbas *BBAnalState) SetBranch(addr uint32) {
	if bbas.BranchAddr != 0 {
		fmt.Printf("WARNING:\tSetBranch already set\n")
	}
	bbas.BranchAddr = addr
}

func (bbas *BBAnalState) Append(instr *bridge.MipsOpcode) {
	bbas.Lines = append(bbas.Lines, *instr)
}

func (bbas *BBAnalState) Yield(last_addr uint32) {
	if len(bbas.Lines) == 0 {
		return
	}

	fmt.Printf("bb:0x%08x br:0x%08x last_addr:0x%08x\n", bbas.BBAddr, bbas.BranchAddr, last_addr)
	for _, line := range bbas.Lines {
		fmt.Printf("\t0x%08x %s\n", line.Address, line.Dizz)
	}

	bbas.Reset()
	bbas.Count += 1
}

func (anal *FunctionAnalyzer) ProcessBB(start_addr uint32, last_addr uint32) int {
	var bbas BBAnalState
	bbas.Init()
	var prevInstr *bridge.MipsOpcode = nil

	for addr := start_addr; last_addr == 0 || addr <= last_addr; addr += 4 {
		bbas.SetBB(addr)

		instr := anal.doc.Disasm(addr)

		bbas.Append(instr)

		if instr.IsBranch {
			bbas.SetBranch(addr)

			if !instr.HasDelaySlot {
				fmt.Printf("WARNING:\tunhandled branch without delay shot\n")
				bbas.Yield(addr)

				if last_addr == 0 && instr.IsConditional {
					break
				}
			}
		}

		if prevInstr != nil && prevInstr.HasDelaySlot {
			bbas.Yield(addr)

			if last_addr == 0 && !prevInstr.IsConditional {
				break
			}
		}

		prevInstr = instr
	}

	bbas.Yield(last_addr)

	return bbas.Count
}