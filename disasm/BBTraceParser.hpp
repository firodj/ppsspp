#pragma once

#include "Types.hpp"

#include <map>
#include <vector>
#include <string>
#include <functional>
#include "FlameGraph.hpp"

class MyDocument;
class MyFunction;
class FNTreeNode;

typedef u32 FNTreeNodeID;
typedef std::vector<FNTreeNodeID> FNChildren;

class FNTreeNode {
public:
  FNTreeNode(u32 func_addr): parentID_(0), ID_(0), func_addr_(func_addr),
    func_(nullptr) {

  }

  FNTreeNodeID parentID_;
  FNChildren children_;
  FNTreeNodeID ID_;
  u32 func_addr_;
  MyFunction *func_;
};

class FNHierarchy {
public:
  FNHierarchy();
  FNTreeNodeID AddNode(u32 func_addr, FNTreeNodeID parentID = 0);
  void DumpNode(int level = 0, FNTreeNodeID parentID = 0);
  FNTreeNode* At(FNTreeNodeID id);

  FNChildren first_;
  std::vector<FNTreeNode> nodes_;
};

struct BBTStackItem {
  u32 addr;
  u32 ra;
  MyFunction *func;
  FNTreeNodeID hierarchyID;
};

class BBTThreadState {
public:
  int ID;
  u32 pc;
  int reg_sp;
  std::vector<BBTStackItem> stack;

  bool executing;
  FlameGraph flame_graph;

  const std::string& name();
  void name(std::string str);

  FNHierarchy hierarchy;

private:
  std::string name_;
};

struct BBTParseParam {
  u16 ID;
  u32 kind;
  u32 pc;
  u32 last_pc;
  refts nts;
};

class BBTraceParser {
public:
  BBTraceParser(MyDocument *doc);
	void Parse(std::function<void(BBTParseParam)> func, int length = 20);

  void ParseRec(int ID, u32 bb_addr);
  void SetCurrentThread(u16 ID);
  u32 SetCurrentThreadPC(u16 ID, u32 addr);

	std::string filename_;

  u16 currentID_;
  std::map<u16, BBTThreadState> threads_;

  static void ParsingBB(BBTraceParser &self, BBTParseParam &par);
  void EndParsing();
  void RenderFlameGraph();

  void DumpAllHierarchy();

  MyDocument *doc() { return doc_; }
  //void nts(refts _nts) { nts_ = _nts; }
  refts nts() { return nts_; }

  bool shown_;

private:
  MyDocument *doc_;
  refts nts_;
  refts fts_;
};
