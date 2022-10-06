package internal

import (
	"fmt"

	"github.com/firodj/ppsspp/disasm/pspdisasm/models"
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


func (anal *FunctionAnalyzer) ProcessBB(start_addr uint32, last_addr uint32) int {
	var bbas BBAnalState
	bbas.Init()
	var prevInstr *models.MipsOpcode = nil

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