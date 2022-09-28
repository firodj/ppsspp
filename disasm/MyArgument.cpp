#include "MyArgument.hpp"

#include <iostream>
#include <sstream>
#include <cstring>
#include <cstdio>

void MyArgument::Scan(const char * opr, size_t sz) {
	char rs[256];
	int imm;

  if (sz == 2) {
    type_ = ArgReg;
    reg_ = std::string(opr, sz);
    return;
  }

  int n = std::sscanf(opr, "%08x(%s)", &imm, rs);
	if (n >= 1) {
    type_ = ArgImm;
		value_ = imm;
		if (n >= 2) {
      type_ = ArgMem;
			reg_ = std::string(rs, std::strlen(rs) - 1);
		}
	} else {
    type_ = ArgReg;
    reg_ = std::string(opr, sz);
  }
}

std::string MyArgument::ValueStr(bool isDec) {
  std::stringstream ss;
  int n = value_;

  if (isDec)
    ss << std::dec;
  else
    ss << std::uppercase << std::hex;

  if (value_ < 0) {
    ss << "-";
    n = -n;
  }

  if (!isDec)
    ss << "0x";

  ss << n;

  return ss.str();
}

std::string MyArgument::Str(bool isDec) {
  std::stringstream ss;

  switch (type_) {
    case ArgImm:
      if (!label_.empty()) {
        ss << label_;
      } else {
        ss << ValueStr(isDec);
      }
      break;
    case ArgReg:
      if (reg_ == "zero")
        ss << 0;
      else
        ss << reg_;
      break;
    case ArgMem:
      ss << "[" << reg_;
      if (value_) {
        ss << " + " << ValueStr(isDec);
      }
      ss << "]";
      break;
    default:
      ss << "??";
  }

  return ss.str();
}


bool MyArgument::IsNegative() {
  return type_ == ArgImm && value_ < 0;
}

bool MyArgument::IsNumber() {
  return type_ == ArgImm;
}

bool MyArgument::IsZero() {
  return
    (type_ == ArgImm && value_ == 0) ||
    (type_ == ArgReg && reg_ == "zero");
}
