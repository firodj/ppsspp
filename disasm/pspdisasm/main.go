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

	defer doc.Delete()
}
