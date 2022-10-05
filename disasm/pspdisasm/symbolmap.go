package main

type SymbolMap struct {
	ptr bridgeSymbolMap
}

func CreateSymbolMap() *SymbolMap {
	symmap := &SymbolMap{
		ptr: NewSymbolMap(),
	}
	return symmap
}

func (symmap *SymbolMap) Delete() {
	DeleteSymbolMap(symmap.ptr)
}

func (symmap *SymbolMap) GetFunctionSize(startAddress uint32) uint32 {
	return SymbolMap_GetFunctionSize(symmap.ptr, startAddress)
}

func (symmap *SymbolMap) GetFunctionStart(address uint32) uint32 {
	return SymbolMap_GetFunctionStart(symmap.ptr, address)
}

func (symmap *SymbolMap) GetLabelName(address uint32) *string {
	return SymbolMap_GetLabelName(symmap.ptr, address)
}

func (symmap *SymbolMap) AddFunction(name string, address uint32, size uint32, moduleIndex int) {
	SymbolMap_AddFunction(symmap.ptr, name, address, size, moduleIndex)
}

func (symmap *SymbolMap) AddModule(name string, address uint32, size uint32) {
	SymbolMap_AddModule(symmap.ptr, name, address, size)
}