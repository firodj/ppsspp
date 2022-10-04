package main

import (
	"fmt"
	"os"
	"runtime"

	"github.com/firodj/ppsspp/disasm/bridge"
)

func main() {
	home := os.Getenv("HOME")
	if runtime.GOOS == "windows" {
		home = os.Getenv("USERPROFILE")
	}
	fmt.Println(home)

	doc := bridge.NewDocument(home + "/Sora", true)
	defer doc.Delete()

	addrs := []uint32{0x8A38A70, 0x8A38A74, 0x8804140}
	for _, addr := range addrs {
		inst := doc.Disasm(addr)
		fmt.Println(inst.AsString())
	}

	fmt.Println("HLE Modules:", doc.HLEModules.Size())
	for idx := 0; idx < doc.HLEModules.Size(); idx++ {
		modl := doc.HLEModules.At(idx)
		fmt.Println(idx, modl.GetName())
	}

  fmt.Println("Done.")
}
