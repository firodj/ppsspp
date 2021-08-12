#pragma once

#include <vector>
#include <string>

#include "Common/CommonTypes.h"

#define KIND_ID 0x4449 // 'I'49 _ 'D'44
#define KIND_SZ 0x5A53 // 'S'53 _ 'Z'5A
#define KIND_START 0x00005453 // 'S'53 _ 'T'54
#define KIND_NAME 0x00004D4E // 'N'4E _ 'M'4D
#define KIND_END 0x00004445 // 'E'45 _ 'D'44
#define BB_LOG_SIZE 10
#define BB_LOG_TTL 100

typedef struct {
	std::string messages[BB_LOG_SIZE];
	int size;
} BBLogs;

class BBTrace {
public:
	BBTrace();
	~BBTrace();

	void StartThread(u32 pc);
	void Naming(const char *str, size_t sz);
	void RecordPC(u32 pc);
	void RecordEnd();
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
void BBTraceLog(std::string msg);
void BBTraceLogs(BBLogs& bblogs);
