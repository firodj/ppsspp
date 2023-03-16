#include "ppsspp_config.h"
#include <algorithm>
#include <map>
#include <unordered_map>

#include "Common/CommonTypes.h"
#include "Common/Data/Convert/SmallDataConvert.h"
#include "Common/Log.h"
#include "Common/Swap.h"
#include "Core/Config.h"
#include "Core/System.h"
#include "Core/Debugger/Breakpoints.h"
#include "Core/Debugger/MemBlockInfo.h"
#include "Core/Debugger/SymbolMap.h"
#include "Core/MemMap.h"
#include "Core/MIPS/JitCommon/JitCommon.h"
#include "Core/MIPS/MIPSCodeUtils.h"
#include "Core/MIPS/MIPSAnalyst.h"
#include "Core/HLE/ReplaceTables.h"
#include "Core/HLE/FunctionWrappers.h"
#include "Core/HLE/sceDisplay.h"

#include "GPU/Math3D.h"
#include "GPU/GPU.h"
#include "GPU/GPUInterface.h"
#include "GPU/GPUState.h"
#include "Core/HLE/sceKernelThread.h"

int sceKernelSetCompiledSdkVersion380_390(int sdkVersion);
int sceKernelSetCompilerVersion(int version);

int Replace_soranokiseki_tc_start() {
    u32 &sp = currentMIPS->r[MIPS_REG_SP];
    sp = sp + -0x20;

    int argSize = PARAM(0);
    u32 argBlockPtr = PARAM(1);

    int sdkVersion = 0x3080010;
    sceKernelSetCompiledSdkVersion380_390(sdkVersion);
    
    int compilerVersion = 0x30306;
    sceKernelSetCompilerVersion(PARAM(0));

    // A0
    u32 threadNameAddr = 0x8a60000 + -0x38c8;
    const char *threadName = Memory::GetCharPointer(threadNameAddr);

    u32 entry = 0x8804228; // A1
    u32 prio  = 0x20; // A2
    int stacksize = Memory::Read_U32(0x8a80000 + -0x5d34) << 10; // A3

    u32 attr = 0x80000000; // A4/T0
    u32 optionAddr = 0; // A5/T1

    SceUID threadToStartID = sceKernelCreateThread(threadName, entry, prio, stacksize, attr, optionAddr); 

    sceKernelStartThread(threadToStartID, argSize, argBlockPtr);

    RETURN(0);
    sp = sp + 0x20;

	return 0;
}

void ReplaceSoraFunctions() {
	if (!g_Config.bSoraPatch) {
		return;
	}
	
	WriteReplaceInstructionByName(0x08804114, "sonranokiseki_tc_start");
}

#if 0
void start() {

start:  // 0x08804114   // visited
    sp = sp + -0x20; // addiu        sp, sp, -0x20
    v0 = 0x3080000;  // lui  v0, 0x308
    (u32)[sp + 0xc] = s3;    // sw   s3, 0xC(sp)
    s3 = a0; // move s3, a0
    a0 = v0 | 0x10;  // ori  a0, v0, 0x10
    (u32)[sp + 0x8] = s2;    // sw   s2, 0x8(sp)
    s2 = a1; // move s2, a1
    (u32)[sp + 0x4] = s1;    // sw   s1, 0x4(sp)
    (u32)[sp] = s0;  // sw   s0, 0x0(sp)
    s0 = 0x00000;    // lui  s0, 0x0
    (u32)[sp + 0x10] = ra;   // sw   ra, 0x10(sp)
    s1 = s0;
    v0 = zz_sceKernelSetCompiledSdkVersion380_390(a0)      /* { ra = 0x08804148; goto zz_sceKernelSetCompiledSdkVersion380_390; } */       // jal  ->$08a38a70
        // addiu        s1, s0, 0x0
start__0x34:    // 0x08804148   // visited
    v0 = 0x30000    // lui  v0, 0x3
    a0 = v0 | 0x306;
    v0 = zz_sceKernelSetCompilerVersion(a0)        /* { ra = 0x08804154; goto zz_sceKernelSetCompilerVersion; } */ // jal  ->$08a38a88
        // ori  a0, v0, 0x306
start__0x40:    // 0x08804154   // visited
    t1 = 0x00000;
    if (s1 == 0) goto start__0x74   // beq  s1, zero, ->$08804188
        // lui  t1, 0x0
start__0x74:    // 0x08804188   // visited
    v1 = t1 // addiu        v1, t1, 0x0
    a0 = v1;
    if (v1 == 0) goto start__0x108  // beq  v1, zero, ->$0880421c
        // move a0, v1
start__0x108:   // 0x0880421c   // visited
    t2 = 0x8a60000  // lui  t2, 0x8A6
    a0 = t2 + -0x38c8;
    goto start__0x80;       // j    ->$08804194
        // addiu        a0, t2, -0x38C8
start__0x80:    // 0x08804194   // visited
    v1 = 0x00000    // lui  v1, 0x0
    t3 = v1 // addiu        t3, v1, 0x0
    a2 = 0x20;
    if (t3 == 0) goto start__0x94   // beq  t3, zero, ->$088041a8
        // li   a2, 0x20
start__0x94:    // 0x088041a8   // visited
    v1 = 0x8a80000  // lui  v1, 0x8A8
    t4 = v1 + -0x5d34       // addiu        t4, v1, -0x5D34
    a3 = 0x40000;
    if (t4 == 0) goto start__0xac   // beq  t4, zero, ->$088041c0
        // lui  a3, 0x4
start__0xa4:    // 0x088041b8   // visited
    t5 = (u32)[v1 + -0x5d34]        // lw   t5, -0x5D34(v1)
    a3 = t5 << 10   // sll  a3, t5, 0xA
    a1 = 0x00000    // lui  a1, 0x0
    t6 = a1 // addiu        t6, a1, 0x0
    t0 = 0x80000000;
    if (t6 == 0) goto start__0xc8   // beq  t6, zero, ->$088041dc
        // lui  t0, 0x8000
start__0xc8:    // 0x088041dc   // visited
    s1 = 0x8800000  // lui  s1, 0x880
    a1 = s1 + 0x4228        // addiu        a1, s1, 0x4228
    t1 = 0x0;
    v0 = zz_sceKernelCreateThread(a0, a1, a2, a3, t0, t1)      /* { ra = 0x088041ec; goto zz_sceKernelCreateThread; } */       // jal  ->$08a38b28
        // li   t1, 0
start__0xd8:    // 0x088041ec   // visited
    a0 = v0 // move a0, v0
    a1 = s3 // move a1, s3
    a2 = s2;
    v0 = zz_sceKernelStartThread(a0, a1, a2)       /* { ra = 0x088041fc; goto zz_sceKernelStartThread; } */        // jal  ->$08a38ac8
        // move a2, s2
start__0xe8:    // 0x088041fc   // visited
    ra = (u32)[sp + 0x10]   // lw   ra, 0x10(sp)
    s3 = (u32)[sp + 0xc]    // lw   s3, 0xC(sp)
    s2 = (u32)[sp + 0x8]    // lw   s2, 0x8(sp)
    s1 = (u32)[sp + 0x4]    // lw   s1, 0x4(sp)
    s0 = (u32)[sp]  // lw   s0, 0x0(sp)
    v0 = 0x0        // li   v0, 0
    sp = sp + 0x20;
    return v0       /* { goto -> ra; } */   // jr   ->ra
        // addiu        sp, sp, 0x20
}
#endif
