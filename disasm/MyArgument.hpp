#pragma once

#include <string>
#include <vector>

#include "Types.hpp"

typedef enum {
  ArgNone = 0,
  ArgImm,
  ArgReg,
  ArgMem,
} ArgType;

class MyArgument {
public:
  ArgType type_;
	std::string label_; // for Symbol
	int value_; // or offset
	std::string reg_;
	bool is_codelocation_;

	MyArgument(): value_(0), type_(ArgNone), is_codelocation_(false) {}

	void Scan(const char * opr, size_t sz);

	bool IsNegative();
	bool IsNumber();
	bool IsZero();

  std::string Str(bool isDec = false);
  std::string ValueStr(bool isDec = false);
};
