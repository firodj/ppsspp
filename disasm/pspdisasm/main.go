package main

import (
	"fmt"
	"os"
	"runtime"
)

func main() {
	home := os.Getenv("HOME")
	if runtime.GOOS == "windows" {
		home = os.Getenv("USERPROFILE")
	}
	fmt.Println(home)

	doc, err := NewSoraDocument(home + "/Sora", true)
	if err != nil {
		fmt.Println(err)
	}

	entryName := doc.GetLabelName(doc.EntryAddr)
	fmt.Println(doc.yaml.Module.NM.EntryAddr)
	if entryName != nil {
		fmt.Println(*entryName)
	}

	doc.Disasm(doc.EntryAddr)

	defer doc.Delete()
}
