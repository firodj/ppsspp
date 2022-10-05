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
