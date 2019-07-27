//
// Created by Yihang Yang on 2019-07-25.
//

#include "misc.h"

void Assert(bool e, const std::string &error_message) {
  if (!e) {
    std::cout << error_message << std::endl;
    assert(e);
  }
}