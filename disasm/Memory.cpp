#include <iostream>

#include "Common/Common.h"
#include "Core/MemMap.h"
#include "Core/MIPS/MIPS.h"
#include "Core/Opcode.h"

namespace Memory {

MemoryInitedLock::MemoryInitedLock() {}
MemoryInitedLock::~MemoryInitedLock() {}
MemoryInitedLock Lock() { return MemoryInitedLock(); }

__forceinline static Opcode Read_Instruction(u32 address, bool resolveReplacements, Opcode inst)
{
	if (!MIPS_IS_EMUHACK(inst.encoding)) {
		return inst;
	}

	if (MIPS_IS_RUNBLOCK(inst.encoding)) {
		return inst;
	}
	else if (resolveReplacements && MIPS_IS_REPLACEMENT(inst.encoding)) {
		return inst;
	}
	else {
		return inst;
	}
}

Opcode Read_Instruction(u32 address, bool resolveReplacements)
{
	Opcode inst = Opcode(Read_U32(address));
	return Read_Instruction(address, resolveReplacements, inst);
}

Opcode ReadUnchecked_Instruction(u32 address, bool resolveReplacements)
{
	Opcode inst = Opcode(ReadUnchecked_U32(address));
	return Read_Instruction(address, resolveReplacements, inst);
}

Opcode Read_Opcode_JIT(u32 address)
{
	Opcode inst = Opcode(Read_U32(address));
	return inst;
}

u8* base = nullptr;
u32 g_MemorySize;

}

void Disasm_MemoryException(u32 address, u32 pc, const char* desc) {
	std::cout << desc << ": Invalid address 0x" << std::hex << address << " PC 0x" << std::hex << pc << std::endl;
}
