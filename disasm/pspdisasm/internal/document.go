package internal

import (
	"fmt"
	"os"
	"path/filepath"
	"unsafe"

	"gopkg.in/yaml.v3"

	"github.com/firodj/ppsspp/disasm/pspdisasm/bridge"
)


type PSPSegment struct {
	Addr uint32 `yaml:"addr"`
	Size int    `yaml:"size"`
}

type PSPNativeModule struct {
	Name      string       `yaml:"name"`
	Segments  []PSPSegment `yaml:"segments"`
	EntryAddr uint32       `yaml:"entry_addr"`
}

type PSPModule struct {
	NM        PSPNativeModule `yaml:"nm"`
	TextStart uint32          `yaml:"textStart"`
	TextEnd   uint32          `yaml:"textEnd"`
	ModulePtr uint32          `yaml:"modulePtr"`
}

type SoraFunction struct {
	Name        string   `yaml:"name"`
	Address     uint32   `yaml:"address"`
	Size        *uint32  `yaml:"size"`
	LastAddress *uint32  `yaml:last_address"`
	BBAddresses []uint32 `yaml:bb_addresses"`
}

type PSPHLEFunction struct {
	Idx     string `yaml:"idx"`
	Nid     uint32 `yaml:"nid"`
	Name    string `yaml:"name"`
	ArgMask string `yaml:"argmask"`
	RetMask string `yaml:"retmask"`
	Flags   uint32 `yaml:"flags"`
}

type PSPHLEModule struct {
	Name  string           `yaml:"name"`
	Funcs []PSPHLEFunction `yaml:"funcs"`
}

type PSPLoadedModule struct {
	Name     string `yaml:"name"`
	Address  uint32 `yaml:"address"`
	Size     uint32 `yaml:"size"`
	IsActive bool   `yaml:"isActive"`
}

type PSPMemory struct {
	Start uint32 `yaml:"start"`
	Size  int    `yaml:"size"`
}

type SoraYaml struct {
	Module        PSPModule         `yaml:"module"`
	Memory        PSPMemory         `yaml:"memory"`
	LoadedModules []PSPLoadedModule `yaml:"loaded_modules"`
	Functions     []SoraFunction    `yaml:"functions"`
	HLEModules    []PSPHLEModule    `yaml:"hle_modules"`
}

type SoraDocument struct {
	yaml SoraYaml
	buf  unsafe.Pointer
	// HLEModules
	// MemoryDump
	// UseDef Analyzer
	// SymbolMap
	symmap *SymbolMap

	mapAddrToFunc map[uint32]int
	mapNameToFunc map[string][]int

	EntryAddr uint32
}

func (doc *SoraDocument) LoadYaml(filename string) error {
	file, err := os.Open(filename)
	if err != nil {
		return err
	}
	defer file.Close()

	err = yaml.NewDecoder(file).Decode(&doc.yaml)
	if err != nil {
		return err
	}

	return nil
}

func (doc *SoraDocument) LoadMemory(filename string) error {
	data, err := os.ReadFile(filename)
	if err != nil {
		return err
	}
	doc.buf = bridge.GlobalSetMemoryBase(data, doc.yaml.Memory.Start)

	return nil
}

func NewSoraDocument(path string, load_analyzed bool) (*SoraDocument, error) {
	main_yaml := filepath.Join(path, "Sora.yaml")
	main_data := filepath.Join(path, "SoraMemory.bin")
	//bb_data   := filepath.Join(path, "SoraBBTrace.rec")
	//anal_yaml := filepath.Join(path, "SoraAnalyzed.yaml")

	doc := &SoraDocument{
		symmap: CreateSymbolMap(),
		mapAddrToFunc: make(map[uint32]int),
		mapNameToFunc: make(map[string][]int),
	}
	bridge.GlobalSetSymbolMap(doc.symmap.ptr)
	bridge.GlobalSetGetFuncNameFunc(doc.GetHLEFuncName)

	err := doc.LoadYaml(main_yaml)
	if err != nil {
		return nil, err
	}

	err = doc.LoadMemory(main_data)
	if err != nil {
		return nil, err
	}

	for _, modl := range doc.yaml.LoadedModules {
		doc.symmap.AddModule(modl.Name, modl.Address, uint32(modl.Size))
	}

	for idx := range doc.yaml.Functions {
		fun := &doc.yaml.Functions[idx]
		if fun.LastAddress != nil {
			fun.Size = new(uint32)
			*fun.Size = *fun.LastAddress - fun.Address + 4
		} else if fun.Size != nil {
			fun.LastAddress = new(uint32)
			*fun.LastAddress = fun.Address + *fun.Size - 4
		}

		doc.RegisterExistingFunction(idx)
	}

	doc.EntryAddr = doc.yaml.Module.NM.EntryAddr

	return doc, err
}

func (doc *SoraDocument) GetLabelName(addr uint32) *string {
	return doc.symmap.GetLabelName(addr)
}

func (doc *SoraDocument) RegisterNameFunction(idx int) {
	fun := &doc.yaml.Functions[idx]

	if _, ok := doc.mapNameToFunc[fun.Name]; !ok {
		doc.mapNameToFunc[fun.Name] = make([]int, 0)
	}

	for _, exidx := range doc.mapNameToFunc[fun.Name] {
		if exidx == idx {
			return
		}
	}

	doc.mapNameToFunc[fun.Name] = append(doc.mapNameToFunc[fun.Name], idx)
}

func (doc *SoraDocument) RegisterExistingFunction(idx int)  {
	fun := &doc.yaml.Functions[idx]

	doc.symmap.AddFunction(fun.Name, fun.Address, *fun.Size, -1)

	doc.mapAddrToFunc[fun.Address] = idx

	doc.RegisterNameFunction(idx)
}

func (doc *SoraDocument) CreateNewFunction(addr uint32, last_addr uint32) int {
	if _, ok := doc.mapAddrToFunc[addr]; ok {
		fmt.Printf("WARNING:\tduplicate address CreateNewFunction addr:0x%08x\n", addr);
		return -1
	}

	idx := len(doc.yaml.Functions)

	doc.yaml.Functions = append(doc.yaml.Functions, SoraFunction{
		Address: addr,
		Name: fmt.Sprintf("z_un_%08x", addr),
		LastAddress: new(uint32),
		Size: new(uint32),
	})

	fun := &doc.yaml.Functions[idx]
	*fun.LastAddress = last_addr
	*fun.Size = *fun.LastAddress - fun.Address + 4

  doc.symmap.AddFunction(fun.Name, fun.Address, *fun.Size, -1)

	doc.mapAddrToFunc[fun.Address] = idx

	doc.RegisterNameFunction(idx)

	return idx
}

func (doc *SoraDocument) GetHLEFuncName(moduleIndex int, funcIndex int) string {
	if moduleIndex < len(doc.yaml.HLEModules) {
		modl := &doc.yaml.HLEModules[moduleIndex]
		if funcIndex < len(modl.Funcs) {
			fun := &modl.Funcs[funcIndex]
			return fmt.Sprintf("%s::%s", modl.Name, fun.Name)
		} else {
			return fmt.Sprintf("%s::func%x", modl.Name, funcIndex)
		}
	}
	return fmt.Sprintf("HLE(%x,%x)", moduleIndex, funcIndex)
}

func (doc *SoraDocument) Delete() {
	bridge.FreeAllocatedCString()
	bridge.GlobalSetGetFuncNameFunc(nil)
	bridge.GlobalSetSymbolMap(nil)
	bridge.GlobalSetMemoryBase(nil, 0)
	doc.symmap.Delete()
}

func (doc *SoraDocument) Disasm(address uint32) *bridge.MipsOpcode {
	if !bridge.MemoryIsValidAddress(address) {
		fmt.Println("invalid address")
		return nil
	}

	return bridge.MIPSAnalystGetOpcodeInfo(address)
}

func (doc *SoraDocument) GetFunctionByAddress(address uint32) (int, *SoraFunction)  {
	if idx, ok := doc.mapAddrToFunc[address]; ok {
		return idx, &doc.yaml.Functions[idx]
	}
	return -1, nil
}

func (doc *SoraDocument) GetFunctionByIndex(idx int) *SoraFunction {
	if idx < 0 || idx >= len(doc.yaml.Functions) {
		return nil
	}
	return &doc.yaml.Functions[idx]
}
