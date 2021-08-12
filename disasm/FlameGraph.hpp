#pragma once

#include <map>
#include <vector>
#include <functional>

#include "simple_svg_1.0.0.hpp"

#include "Types.hpp"

#ifndef STACKGRAPH_BASE
#define STACKGRAPH_BASE 1
#endif

class BlockGraph {
public:
  u32 addr_;
  refts start_, stop_;
  refts fts_, fts_stop_;
  std::string text_;

  void End(refts n);
};

class StackGraph {
public:
  int level_;
  std::map<refts, BlockGraph> block_graphs_;

  BlockGraph* Add(refts n, u32 addr, std::string text = {});

  size_t GetSize();
  BlockGraph* Last();
};

class FlameGraph {
public:
  FlameGraph();
  int GetMaxLevel();
  void StopAll(int maxlevel, refts n);

  StackGraph* GetStackGraph(int level);
  BlockGraph* AddBlock(int level, refts n, u32 addr, std::string text = {});
  void EndBlock(int level, refts n);

  BlockGraph* AddMarker(refts n, std::string text);
  void RenderSVG(svg::Document &doc, int &INOUT_base_y, std::function<std::string(u32)> funcNameGetter);
  int CalcDimWidth();
  int CalcDimHeight();

  std::vector<StackGraph> stack_graphs_;
  std::string name_;
  refts fts_;
};
