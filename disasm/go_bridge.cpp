#include "go_bridge.h"

#include "MyDocument.hpp"

#include <stdio.h>

int xgox(int y) {
  printf("Hello World: %d\n", y);
  return y;
}

Document NewDocument() {
  return reinterpret_cast<Document>(new MyDocument());
}

void DeleteDocument(Document doc) {
  MyDocument *_doc =  reinterpret_cast<MyDocument*>(doc);
  delete _doc;
}

int DocumentInit(Document doc, const char* path, int load_analyzed) {
  MyDocument *_doc =  reinterpret_cast<MyDocument*>(doc);
  return _doc->Init(path, load_analyzed);
}

Instruction DocumentDisasm(Document doc, uint32_t addr) {
  MyDocument *_doc =  reinterpret_cast<MyDocument*>(doc);
  return reinterpret_cast<Instruction>(_doc->Disasm(addr));
}

const char* InstructionAsString(Instruction instr) {
  MyInstruction *_instr = reinterpret_cast<MyInstruction*>(instr);
  return _instr->AsString().c_str();
}

int DocumentSizeOfHLEModules(Document doc) {
  MyDocument *_doc =  reinterpret_cast<MyDocument*>(doc);
  return _doc->hleModules().size();
}

HLEModule DocumentHLEModuleAt(Document doc, int idx) {
  MyDocument *_doc =  reinterpret_cast<MyDocument*>(doc);
  if (idx >= _doc->hleModules().size()) return nullptr;
  return reinterpret_cast<HLEModule>(&_doc->hleModules().at(idx));
}

const char* HLEModuleGetName(HLEModule modl) {
  MyHLEModule *_modl = reinterpret_cast<MyHLEModule*>(modl);
	return _modl->name.c_str();
}
