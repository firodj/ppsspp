#pragma once

#include <vector>

#include "Common/CommonTypes.h"

#define KIND_ID 0x4449 // 49'I' 44'D'
#define KIND_SZ 0x5A53 // 53'S' 5A'Z'
#define KIND_START 0x00005453 // 53'S' 54'T'
#define KIND_NAME 0x00004D4E // 4E'N', 4D'M'

class BBTrace {
public:
	BBTrace();
	~BBTrace();

	void StartThread(u32 pc);
	void Naming(const char *str, size_t sz);
	void RecordPC(u32 pc);
	void Flush();
	void SetData(void *data) { data_ = data; }

	static BBTrace *GetFromCurrentThread();

	int ID() { return ID_; }
	void *data() { return data_; }
	int Size() { return bb_pos; }
	void *Record() { return &bb_record[0]; }
	void next_adj_bb(u32 bb) { next_adj_bb_ = bb; }

private:
	std::vector<u32> bb_record;
	int bb_pos, bb_count;
	u32 last_pc;
	u32 next_adj_bb_;
	void *data_;
	int ID_;
};

void DumpBBTrace(BBTrace *bbTrace);
