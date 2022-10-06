package main

import (
	"fmt"
	"os"
	"runtime"

	"github.com/firodj/ppsspp/disasm/pspdisasm/internal"
)

func main() {
	home := os.Getenv("HOME")
	if runtime.GOOS == "windows" {
		home = os.Getenv("USERPROFILE")
	}
	fmt.Println(home)

	doc, err := internal.NewSoraDocument(home + "/Sora", true)
	if err != nil {
		fmt.Println(err)
	}

	entryName := doc.GetLabelName(doc.EntryAddr)
	fmt.Println(doc.EntryAddr)
	if entryName != nil {
		fmt.Println(*entryName)
	}

	doc.Disasm(doc.EntryAddr)

	defer doc.Delete()
}
