#include "BBTrace.h"
#include "Common/File/Path.h"
#include "Common/File/FileUtil.h"
#include "Common/Log.h"

static int BBTrace_ID = 0;

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
			if (pc == next_adj_bb_) {
				next_adj_bb_ = 0;
			} else {
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

	FILE *f = File::OpenCFile(path, "ab+");
	if (!f) {
		ERROR_LOG(COMMON, "Unable to write DumpBBTrace.");
		return;
	}

	u16 hdr[3] = { KIND_ID, (u16)bbTrace->ID(), KIND_SZ, };
	fwrite(&hdr, sizeof(u16), 3, f);

	u32 sz = bbTrace->Size();
	fwrite(&sz, sizeof(u32), 1, f);

	fwrite(bbTrace->Record(), sizeof(u32), bbTrace->Size(), f);

	fclose(f);
}
