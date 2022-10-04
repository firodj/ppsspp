#ifndef __GO_BRIDGE_H__
#define __GO_BRIDGE_H__

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

int xgox(int y);

typedef void* Document;
typedef void* Instruction;
typedef void* HLEModule;

Document NewDocument();
void DeleteDocument(Document doc);
int DocumentInit(Document doc, const char* path, int load_analyzed);
Instruction DocumentDisasm(Document doc, uint32_t addr);
const char* InstructionAsString(Instruction instr);
int DocumentSizeOfHLEModules(Document doc);
HLEModule DocumentHLEModuleAt(Document doc, int idx);
const char* HLEModuleGetName(HLEModule modl);

#ifdef __cplusplus
}
#endif

#endif