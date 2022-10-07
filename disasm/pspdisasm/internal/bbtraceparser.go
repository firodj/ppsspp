package internal

import (
	"encoding/binary"
	"fmt"
	"io"
	"os"
)
const (
	KIND_ID uint16 = 0x4449 // 49'I' 44'D'
	KIND_SZ uint16 = 0x5A53 // 53'S' 5A'Z'
  KIND_START uint32 = 0x00005453 // 53'S' 54'T'
	KIND_NAME uint32 = 0x00004D4E // 4E'N', 4D'M'
)

type RefTs int

type BBTraceStackItem struct {
	Addr uint32
	RA   uint32
	Fun *SoraFunction
	// FNTreeNodeID
}

type BBTraceThreadState struct {
	ID int
	PC uint32
	RegSP int
	Stack []BBTraceStackItem
	Executing bool
	// FlameGraph
	// FNHierarchy

	Name string
}

type BBTraceParam struct {
	ID uint16
	Kind uint32
	PC uint32
	LastPC uint32
	Nts RefTs
}

type BBTraceYield func (param BBTraceParam)

type BBTraceParser struct {
	doc       *SoraDocument
	filename  string
	Nts       RefTs
	Fts       RefTs
	CurrentID uint16
}

func NewBBTraceParser(doc *SoraDocument, filename string) *BBTraceParser {
	bbtparser := &BBTraceParser{
		doc: doc,
		filename: filename,
		CurrentID: 0,
		Nts: 0,
		Fts: 0,
	}
	return bbtparser
}

func (this *BBTraceParser) Parse(cb BBTraceYield, length int) error {
	bin, err := os.Open(this.filename)
	if err != nil {
		return err
	}
	defer bin.Close()

	ok := true
	this.Nts = 1
	this.Fts = 1
	if length < 1 {
		length = 1
	}
	var curID uint16 = 0

	buf32 := make([]byte, 4)
	buf16 := make([]byte, 2)

	for ok {

		_, err := bin.Read(buf16)
		if err != nil {
			if err != io.EOF {
				return err
			}
			break
		}

		kind := uint16(binary.LittleEndian.Uint16(buf16))
		if kind != KIND_ID {
			return fmt.Errorf("ERROR:\tunmatched kind 'ID', found: 0x%x\n", kind)
		}

		_, err = bin.Read(buf16)
		if err != nil {
			return err
		}
		curID = uint16(binary.LittleEndian.Uint16(buf16))
		fmt.Println(curID)

		_, err = bin.Read(buf16)
		if err != nil {
			return err
		}
		kind = uint16(binary.LittleEndian.Uint16(buf16))

		if kind != KIND_SZ {
			return fmt.Errorf("ERROR:\tunmatched kind 'SZ', found: 0x%x\n", kind)
		}

		_, err = bin.Read(buf32)
		if err != nil {
			return err
		}
		size := uint32(binary.LittleEndian.Uint32(buf32))

		records := make([]byte, size * 4)
		_, err = bin.Read(records)
		if err != nil {
			return err
		}
	}

	return nil
}