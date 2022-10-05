package main

import (
	"os"
	"path/filepath"

	"gopkg.in/yaml.v3"
)


type PSPSegment struct {
	Addr uint32 `yaml:"addr"`
	Size int `yaml:"size"`
}

type PSPNativeModule struct {
	Name     string       `yaml:"name"`
	Segments []PSPSegment `yaml:"segments"`
}

type PSPModule struct {
	NM        PSPNativeModule `yaml:"nm"`
	TextStart uint32          `yaml:"textStart"`
	TextEnd   uint32          `yaml:"textEnd"`
	ModulePtr uint32          `yaml:"modulePtr"`
}

type SoraFunction struct {
	Name    string `yaml:"name"`
	Address uint32 `yaml:"address"`
	Size    int    `yaml:"size"`
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
	Size     int    `yaml:"size"`
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
	buf []byte
	// HLEModules
	// MemoryDump
	// UseDef Analyzer
	// SymbolMap
	symmap *SymbolMap
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
	doc.buf = data
	GlobalSetMemoryBase(doc.buf)

	return nil
}

func NewSoraDocument(path string, load_analyzed bool) (*SoraDocument, error) {
	main_yaml := filepath.Join(path, "Sora.yaml")
	main_data := filepath.Join(path, "SoraMemory.bin")
	//bb_data   := filepath.Join(path, "SoraBBTrace.rec")
	//anal_yaml := filepath.Join(path, "SoraAnalyzed.yaml")

	doc := &SoraDocument{
		symmap: CreateSymbolMap(),
	}
	GlobalSetSymbolMap(doc.symmap.ptr)
	GlobalSetGetFuncNameFunc(doc.GetFuncName)

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

	return doc, err
}

func (doc *SoraDocument) GetFuncName(moduleIndex int, funcIndex int) *string {

	return nil
}

func (doc *SoraDocument) Delete() {
	FreeAllocatedCString()
	GlobalSetGetFuncNameFunc(nil)
	GlobalSetSymbolMap(nil)
	GlobalSetMemoryBase(nil)
	doc.symmap.Delete()
}