#include <iostream>

#include "FlameGraph.hpp"

#include "MyDocument_internal.hpp"

const int rect_size = 16;
const int name_width = 100;

//--- BlockGraph ---

void
BlockGraph::End(refts n) {
  stop_ = n;
}

//--- StackGraph ---

BlockGraph*
StackGraph::Last() {
  auto it = block_graphs_.rbegin();
  if (it != block_graphs_.rend())
    return &it->second;
  return nullptr;
}

size_t
StackGraph::GetSize() {
  return block_graphs_.size();
}

BlockGraph*
StackGraph::Add(refts n, u32 addr, std::string text)
{
  if (block_graphs_.find(n) != block_graphs_.end()) return nullptr;

  block_graphs_[n].addr_ = addr;
  block_graphs_[n].start_ = n;
  block_graphs_[n].stop_ = n+1;
  block_graphs_[n].fts_ = 0;
  block_graphs_[n].fts_stop_ = 0;
  block_graphs_[n].text_ = text;
  return &block_graphs_[n];
}

//--- FlameGraph ---

FlameGraph::FlameGraph() {
  stack_graphs_.resize(STACKGRAPH_BASE);
}

StackGraph*
FlameGraph::GetStackGraph(int level) {
  if (level < STACKGRAPH_BASE) {
    //std::cout << "ERROR\treach over minimum level " << std::to_string(STACKGRAPH_BASE) << " with " << level << std::endl;
    return nullptr;
  }
  while (GetMaxLevel() < level) {
    stack_graphs_.emplace_back();
  };
  return &stack_graphs_.at(STACKGRAPH_BASE + (level - 1));
}

int
FlameGraph::GetMaxLevel() {
  return stack_graphs_.size() - STACKGRAPH_BASE;
}

void
FlameGraph::StopAll(int maxlevel, refts n)
{
  for (int i=maxlevel; i > 0; i--) {
    auto last_block = GetStackGraph(i)->Last();
    if (last_block) {
      last_block->stop_ = n;
      last_block->fts_stop_ = fts_;
    }
  }
}

BlockGraph*
FlameGraph::AddMarker(refts n, std::string text) {
  if (STACKGRAPH_BASE == 0) {
    std::cout << "ERROR\ncannot AddMarker not enough STACKGRAPH_BASE" << std::endl;
    return nullptr;
  }

  auto &marker = stack_graphs_.at(0);
  return marker.Add(n, 0, text);
}

void
FlameGraph::RenderSVG(svg::Document &doc, int &INOUT_base_y, std::function<std::string(u32)> funcNameGetter) {
  int y_bottom = (GetMaxLevel() * rect_size);
  int base_x = name_width;
  int mode = 1;

  doc << svg::Text(svg::Point(0, INOUT_base_y + y_bottom), name_, svg::Color::Black, svg::Font(10, "Verdana"));

  std::cout << "Name:" << std::dec << name_ << std::endl;
  std::cout << "MaxLevel: " << GetMaxLevel() << std::endl;

  for (int level=1; level <= GetMaxLevel(); level++) {
    int y = INOUT_base_y + y_bottom - (level * rect_size);
    std::cout << "\tL:" << std::dec << level << std::endl;

    auto &block_graphs = GetStackGraph(level)->block_graphs_;

    for (auto it = block_graphs.begin(); it != block_graphs.end(); it++) {
      auto &block = it->second;
      int x, x_width;
      if (mode == 0) {
        x = block.start_ * rect_size;
        x_width = (block.stop_ - block.start_) * rect_size;
      } else if (mode == 1) {
        x = block.fts_ * rect_size;
        x_width = (block.fts_stop_ - block.fts_) * rect_size;
      }

      std::cout << "\t\tF: " << std::dec << block.start_ << " - " << std::dec << block.stop_;

      std::string text = funcNameGetter(block.addr_);
      std::cout << " Name: " << text;
      std::cout << " Fts: " << block.fts_;
      std::cout << std::endl;

      doc << svg::Rectangle(svg::Point(base_x + x, y), x_width - 1, rect_size - 1, svg::Color::Red);
      doc << svg::Text(svg::Point(base_x + x + 8, y), text, svg::Color::Blue, svg::Font(10, "Verdana")).Rotate(270);
    }
  }

  INOUT_base_y += y_bottom;
}

int FlameGraph::CalcDimWidth() {
  int max_x = name_width;
  int mode = 1;

  for (int level=1; level <= GetMaxLevel(); level++) {
    auto &block_graphs = GetStackGraph(level)->block_graphs_;
    auto it = block_graphs.rbegin();

    if (it != block_graphs.rend()) {
      auto &last_block = it->second;
      if (mode == 0) {
        max_x = name_width + (last_block.stop_ * rect_size);
        break;
      } else if (mode == 1) {
        int x = name_width + (last_block.fts_stop_ * rect_size);
        if (max_x < x) max_x = x;
      }
    }
  }

  return max_x;
}

int FlameGraph::CalcDimHeight() {
  return GetMaxLevel() * rect_size;
}

BlockGraph*
FlameGraph::AddBlock(int level, refts n, u32 addr, std::string text) {
  StackGraph* stack_graph = GetStackGraph(level);
  if (!stack_graph) return nullptr;

  BlockGraph *block = stack_graph->Add(n, addr, text);
  block->fts_ = fts_++;

  return block;
}

void
FlameGraph::EndBlock(int level, refts n) {
  StackGraph* stack_graph = GetStackGraph(level);
  if (!stack_graph) return;

  auto last_block = stack_graph->Last();
  if (last_block) {
    last_block->stop_ = n;
    last_block->fts_stop_ = fts_;
  }
}
