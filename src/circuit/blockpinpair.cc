//
// Created by yihan on 7/27/2019.
//

#include "blockpinpair.h"

BlockPinPair::BlockPinPair(Block *block, int pin): block_(block), pin_num_(pin) {}

Block *BlockPinPair::GetBlock() const {
  return block_;
}

int BlockPinPair::BlockNum() const {
  return block_->Num();
}

Pin *BlockPinPair::GetPin() const {
  return &(block_->Type()->pin_list[pin_num_]);
}

int BlockPinPair::PinNum() const {
  return pin_num_;
}

double BlockPinPair::XOffset() {
  if (block_->Orient() == N) {
    return block_->Type()->pin_list[pin_num_].XOffset();
  } else {
    std::cout << "Currently, only N orientation is supported\n";
    exit(1);
  }
}

double BlockPinPair::YOffset() {
  if (block_->Orient() == N) {
    return block_->Type()->pin_list[pin_num_].YOffset();
  } else {
    std::cout << "Currently, only N orientation is supported\n";
    exit(1);
  }
}

double BlockPinPair::AbsX() const {
  if (block_->Orient() == N) {
    return block_->Type()->pin_list[pin_num_].XOffset() + block_->LLX();
  } else {
    std::cout << "Currently, only N orientation is supported, exit program\n";
    exit(1);
  }
}

double BlockPinPair::AbsY() const {
  if (block_->Orient() == N) {
    return block_->Type()->pin_list[pin_num_].YOffset() + block_->LLY();
  } else {
    std::cout << "Currently, only N orientation is supported, exit program\n";
    exit(1);
  }
}

const std::string *BlockPinPair::BlockName() const {
  return GetBlock()->Name();
}

const std::string *BlockPinPair::PinName() const {
  return GetPin()->Name();
}

bool BlockPinPair::operator <(const BlockPinPair &rhs) const {
  return (BlockNum() < rhs.BlockNum()) || ((BlockNum() == rhs.BlockNum()) && (PinNum() < rhs.PinNum()));
}

bool BlockPinPair::operator >(const BlockPinPair &rhs) const {
  return (BlockNum() < rhs.BlockNum()) || ((BlockNum() == rhs.BlockNum()) && (PinNum() > rhs.PinNum()));
}

bool BlockPinPair::operator ==(const BlockPinPair &rhs) const {
  return (BlockNum() == rhs.BlockNum()) && (PinNum() == rhs.PinNum());
}
