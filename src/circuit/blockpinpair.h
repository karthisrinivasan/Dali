//
// Created by yihan on 7/27/2019.
//

#ifndef HPCC_SRC_CIRCUIT_BLOCKPINPAIR_H_
#define HPCC_SRC_CIRCUIT_BLOCKPINPAIR_H_

#include "block.h"
#include "pin.h"

class Block;

class BlockPinPair {
 private:
  Block *block_;
  int pin_num_;
 public:
  BlockPinPair(Block *block, int pin);
  Block *GetBlock() const;
  int BlockNum() const;
  Pin *GetPin() const;
  int PinNum() const;
  double XOffset();
  double YOffset();
  double AbsX() const;
  double AbsY() const;
  const std::string *BlockName() const;
  const std::string *PinName() const;
  bool operator <(const BlockPinPair &rhs) const;
  bool operator >(const BlockPinPair &rhs) const;
  bool operator ==(const BlockPinPair &rhs) const;
  /*friend std::ostream& operator<<(std::ostream& os, const BlockPinPair &block_pin_pair) {
    os << " (" << *(block_pin_pair.BlockName()) << " " << *(block_pin_pair.PinName()) << ") ";
    return os;
  }*/
};

#endif //HPCC_SRC_CIRCUIT_BLOCKPINPAIR_H_
