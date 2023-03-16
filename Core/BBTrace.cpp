#include "BBTrace.h"
#include "Common/File/Path.h"
#include "Common/File/FileUtil.h"
#include "Common/Log.h"
#include "Core/Config.h"
#include "zlib.h"

#include <mutex>

static int BBTrace_ID = 0;
std::string messages[BB_LOG_SIZE];
int ttl_messages[BB_LOG_SIZE] = {0};
u32 crc_messages[BB_LOG_SIZE] = {0};
int message_pos = 0;
std::mutex g_messages_mutex;

BBTrace::BBTrace() {
	bb_count = 1024 * 1024;
	bb_pos = 0;
	last_pc = 0;
	next_adj_bb_ = 0;
	bb_record.resize(bb_count);
	ID_ = ++BBTrace_ID;
}

BBTrace::~BBTrace() {
	Flush();
}

void BBTrace::Naming(const char *str, size_t sz) {
	char naming[32 + 1] = {0};

	if (bb_pos + 1 + (32 >> 2) > bb_count) {
		Flush();
	}

	strncpy(naming, str, sz > 32 ? 32 : sz);
	bb_record[bb_pos++] = KIND_NAME;
	for (int i = 0; i < 32; i += 4)
		bb_record[bb_pos++] = *(u32*)&naming[i];
}

void BBTrace::StartThread(u32 pc) {
	if (bb_pos + 2 > bb_count) {
		Flush();
	}

	bb_record[bb_pos++] = KIND_START;
	bb_record[bb_pos++] = pc;
}

void BBTrace::RecordPC(u32 pc) {
	if (last_pc != 0) {
		if (last_pc + 4 == pc) {
#if 1
			if (pc == next_adj_bb_) {
				next_adj_bb_ = 0;
			} else
#endif
			{
				last_pc = pc;
				return;
			}
		}
	}

	if (bb_pos + 2 > bb_count) {
		Flush();
	}

	bb_record[bb_pos++] = pc;
	bb_record[bb_pos++] = last_pc;
	last_pc = pc;
}

void BBTrace::RecordEnd() {
	if (bb_pos + 2 > bb_count) {
		Flush();
	}
	bb_record[bb_pos++] = KIND_END;
	bb_record[bb_pos++] = last_pc;
	Flush();
}

void BBTrace::Flush() {
	if (bb_pos) DumpBBTrace(this);
	bb_pos = 0;
}

void DumpBBTrace(BBTrace *bbTrace) {
	const char* env_p = nullptr;
#ifdef WIN32
	env_p = std::getenv("USERPROFILE");
#else
	env_p = std::getenv("HOME");
#endif
	Path path(env_p + std::string("/Sora/SoraBBTrace.rec"));

	if (g_Config.bSoraPatch) {
		return;
	}

	FILE *f = File::OpenCFile(path, "ab+");
	if (!f) {
		BBTraceLog("Unable to write DumpBBTrace: " + path.ToString());
		ERROR_LOG(COMMON, "Unable to write DumpBBTrace.");
		return;
	}

	u16 hdr[3] = { KIND_ID, (u16)bbTrace->ID(), KIND_SZ, };
	fwrite(&hdr, sizeof(u16), 3, f);

	u32 sz = bbTrace->Size();
	fwrite(&sz, sizeof(u32), 1, f);

	fwrite(bbTrace->Record(), sizeof(u32), bbTrace->Size(), f);

	fclose(f);
	
	BBTraceLog("Write DumpBBTrace: " + path.ToString());
}

void BBTraceLog(std::string msg) {
	const std::lock_guard<std::mutex> lock(g_messages_mutex);
	u32 crc = crc32(0, Z_NULL, 0);
	crc = crc32(crc, (Byte*)msg.c_str(), msg.length());
	
	for (int i=0, j=message_pos; i<BB_LOG_SIZE; i++,j++) {
		if (j>=BB_LOG_SIZE) j-=BB_LOG_SIZE;
		if (ttl_messages[j] <= 0) {
			break;
		}
		if (crc_messages[j] == crc) {
			return;
		}
	}
	
	int j = message_pos-1;
	if (j<0) j+=BB_LOG_SIZE;

	messages[j] = msg;
	ttl_messages[j] = BB_LOG_TTL;
	crc_messages[j] = crc;
	message_pos = j;
}

void BBTraceLogs(BBLogs& bblogs) {
	const std::lock_guard<std::mutex> lock(g_messages_mutex);
	

	bblogs.size = 0;
	for (int i=0, j=message_pos; i<BB_LOG_SIZE; i++,j++) {
		if (j>=BB_LOG_SIZE) j-=BB_LOG_SIZE;
		if (ttl_messages[j] <= 0) {
			break;
		}
		ttl_messages[j] -= 1;
		bblogs.messages[bblogs.size] = messages[j];
		bblogs.size++;
	}
}
