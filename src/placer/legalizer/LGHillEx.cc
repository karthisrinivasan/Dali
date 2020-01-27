//
// Created by Yihang Yang on 1/2/20.
//

#include "LGHillEx.h"

#include <cfloat>

#include <algorithm>

LGHillEx::LGHillEx()
    : Placer(),
      is_push_(true),
      legalize_from_left_(true),
      cur_iter_(0),
      max_iter_(10),
      k_width_(0.001),
      k_height_(0.001),
      k_left_(1) {}

void LGHillEx::InitLegalizer() {
  row_start_.resize(top_ - bottom_ + 1, left_);
  IndexLocPair<int> tmp_index_loc_pair(0, 0, 0);
  index_loc_list_.resize(circuit_->block_list.size(), tmp_index_loc_pair);
}

bool LGHillEx::IsSpaceLegal(Block &block) {
  /****
 * check if the block is out of the placement region
 * check whether any part of the space is used
 * ****/

  auto blk_left = int(std::round(block.LLX()));
  auto blk_right = int(std::round(block.URX()));
  auto blk_bottom = int(std::round(block.LLY()));
  auto blk_top = int(std::round(block.URY()));


  // 1. check whether the x location of this block is out of the region
  if (blk_left < Left() || blk_right > Right()) {
    return false;
  }
  // 2. check whether the y location of this block is out of the region
  if (blk_bottom < Bottom() || blk_top > Top()) {
    return false;
  }

  // 3. check if any part of the current location is used
  auto start_row = (unsigned int) (blk_bottom - Bottom());
  unsigned int end_row = start_row + block.Height() - 1;
  bool is_avail = true;
  for (unsigned int i = start_row; i <= end_row; ++i) {
    if (row_start_[i] > blk_left) {
      is_avail = false;
      break;
    }
  }

  if (is_avail) {
    block.SetLoc(blk_left, blk_bottom);
  }
  return is_avail;
}

void LGHillEx::UseSpace(Block const &block) {
  /****
   * Mark the space used by this block by changing the start point of available space in each related row
   * ****/
  auto start_row = (unsigned int) (block.LLY() - Bottom());
  unsigned int end_row = start_row + block.Height() - 1;
  if (end_row >= row_start_.size()) {
    //std::cout << "  ly:     " << int(block.LLY())       << "\n"
    //          << "  height: " << block.Height()   << "\n"
    //          << "  top:    " << Top()    << "\n"
    //          << "  bottom: " << Bottom() << "\n";
    Assert(false, "Cannot use space out of range");
  }

  int end_x = int(block.URX());
  for (unsigned int i = start_row; i <= end_row; ++i) {
    row_start_[i] = end_x;
  }
}

bool LGHillEx::PushBlock(Block &block) {
  /****
   * For each row:
   *    Find the non-overlap location
   *    Move the location rightwards to respect overlap rules
   *
   * Cost function is displacement
   * ****/
  auto init_x = int(std::round(block.LLX()));
  auto init_y = int(std::round(block.LLY()));
  auto height = int(block.Height());
  auto right_most = right_ - int(block.Width());
  int start_row = 0;
  int end_row = Top() - Bottom() - height;
  //std::cout << "    Starting row: " << start_row << "\n"
  //          << "    Ending row:   " << end_row   << "\n"
  //          << "    Block Height: " << block.Height() << "\n"
  //          << "    Top of the placement region " << Top() << "\n"
  //          << "    Number of rows: " << row_start_.size() << "\n"
  //          << "    LX and LY: " << int(block.LLX()) << "  " << int(block.LLY()) << "\n";

  int best_row = 0;
  int best_loc = INT_MIN;
  int min_cost = INT_MAX;

  int tmp_cost;
  int tmp_end_row = 0;
  int tmp_loc;

  int min_row = std::max(0, init_y - bottom_ - 2 * height);
  int max_row = std::min(end_row, init_y - bottom_ + 2 * height);

  bool all_row_fail = true;

  while (all_row_fail) {
    int old_min_row = min_row;
    int old_max_row = max_row;
    min_row = std::max(0, min_row - 2 * height);
    max_row = std::min(end_row, max_row + 2 * height);

    for (int tmp_row = min_row; tmp_row < max_row; ++tmp_row) {
      if (tmp_row >= old_min_row && tmp_row < old_max_row) continue;
      // 1. find the non-overlap location
      tmp_end_row = tmp_row + height - 1;
      tmp_loc = Left();
      for (int i = tmp_row; i <= tmp_end_row; ++i) {
        tmp_loc = std::max(tmp_loc, row_start_[i]);
      }
      if (tmp_loc > right_most) continue;
      all_row_fail = false;
      tmp_cost = std::abs(tmp_loc - init_x) + std::abs(tmp_row + Bottom() - init_y);
      if (tmp_cost < min_cost) {
        best_loc = tmp_loc;
        best_row = tmp_row;
        min_cost = tmp_cost;
      }

    }

    if (min_row <= 0 && max_row >= end_row) break;
  }

  int res_x = best_loc;
  int res_y = best_row + Bottom();

  //std::cout << res.x << "  " << res.y << "  " << min_cost << "  " << block.Num() << "\n";

  if (min_cost < INT_MAX) {
    block.SetLoc(res_x, res_y);
    return true;
  }

  return false;
}

bool LGHillEx::PushLegalizationFromLeft() {
  std::vector<Block> &block_list = *BlockList();
  for (size_t i = 0; i < index_loc_list_.size(); ++i) {
    index_loc_list_[i].num = i;
    index_loc_list_[i].x = block_list[i].LLX();
    index_loc_list_[i].y = block_list[i].LLY();
  }
  std::sort(index_loc_list_.begin(), index_loc_list_.end());
  unsigned int sz = index_loc_list_.size();
  unsigned int block_num;
  for (unsigned int i = 0; i < sz; ++i) {
    block_num = index_loc_list_[i].num;
    auto &block = block_list[block_num];
    if (block.IsFixed()) {
      continue;
    }
    bool is_cur_loc_legal = IsSpaceLegal(block);
    if (!is_cur_loc_legal) {
      bool loc_found = PushBlock(block);
      if (!loc_found) {
        GenMATLABTable("lg_result.txt");
        if (globalVerboseLevel >= LOG_CRITICAL) {
          std::cout << "  WARNING:  " << cur_iter_ << "-iteration push legalization\n"
                    << "        this may disturb the global placement, and may impact the placement quality\n";
        }
        FastShift(i);
        return false;
      }
    }
    UseSpace(block);
    //std::cout << block.LLX() << "  " << block.LLY() << "\n";
  }
  return true;
}

bool LGHillEx::PushLegalizationFromRight() {
  std::vector<Block> &block_list = *BlockList();
  for (size_t i = 0; i < index_loc_list_.size(); ++i) {
    index_loc_list_[i].num = i;
    index_loc_list_[i].x = block_list[i].LLX();
    index_loc_list_[i].y = block_list[i].LLY();
  }
  std::sort(index_loc_list_.begin(), index_loc_list_.end());
  unsigned int sz = index_loc_list_.size();
  unsigned int block_num;
  for (unsigned int i = 0; i < sz; ++i) {
    block_num = index_loc_list_[i].num;
    auto &block = block_list[block_num];
    if (block.IsFixed()) {
      continue;
    }
    bool is_cur_loc_legal = IsSpaceLegal(block);
    if (!is_cur_loc_legal) {
      bool loc_found = PushBlock(block);
      if (!loc_found) {
        GenMATLABTable("lg_result.txt");
        if (globalVerboseLevel >= LOG_CRITICAL) {
          std::cout << "  WARNING:  " << cur_iter_ << "-iteration push legalization\n"
                    << "        this may disturb the global placement, and may impact the placement quality\n";
        }
        FastShift(i);
        return false;
      }
    }
    UseSpace(block);
    //std::cout << block.LLX() << "  " << block.LLY() << "\n";
  }
  return true;
}

double LGHillEx::EstimatedHPWL(Block &block, int x, int y) {
  double max_x = x;
  double max_y = y;
  double min_x = x;
  double min_y = y;
  double tot_hpwl = 0;
  for (auto &net_num: block.net_list) {
    for (auto &blk_pin_pair: circuit_->net_list[net_num].blk_pin_list) {
      if (blk_pin_pair.GetBlock() != &block) {
        min_x = std::min(min_x, blk_pin_pair.AbsX());
        min_y = std::min(min_y, blk_pin_pair.AbsY());
        max_x = std::max(max_x, blk_pin_pair.AbsX());
        max_y = std::max(max_y, blk_pin_pair.AbsY());
      }
    }
    tot_hpwl += (max_x - min_x) + (max_y - min_y);
  }

  return tot_hpwl;
}

bool LGHillEx::PullBlockLeft(Block &block) {
  /****
   * For each row:
   *    if the row is not used before, do not change the location of this block (this kind of block is termed starting_block)
   *    if all rows are used, find the location with the minimum HPWL
   * Cost function is displacement
   * ****/
  int init_x = int(block.LLX());
  int init_y = int(block.LLY());

  int height = int(block.Height());
  int start_row = 0;
  int end_row = Top() - Bottom() - height;
  //std::cout << "    Starting row: " << start_row << "\n"
  //          << "    Ending row:   " << end_row   << "\n"
  //          << "    Block Height: " << block.Height() << "\n"
  //          << "    Top of the placement region " << Top() << "\n"
  //          << "    Number of rows: " << row_start_.size() << "\n"
  //          << "    LX and LY: " << int(block.LLX()) << "  " << int(block.LLY()) << "\n";

  int best_row = 0;
  int best_loc = INT_MIN;
  auto min_cost = DBL_MAX;
  double tmp_cost;
  int tmp_end_row = 0;
  int tmp_x;
  int tmp_y;

  bool is_starting_block = false;
  double tmp_hpwl = 0;
  double tmp_displacement = 0;
  for (int tmp_row = start_row; tmp_row <= end_row; ++tmp_row) {
    if (row_start_[tmp_row] == Left()) {
      is_starting_block = true;
      break;
    }

    tmp_end_row = tmp_row + height - 1;
    tmp_x = Left();
    for (int i = tmp_row; i <= tmp_end_row; ++i) {
      tmp_x = std::max(tmp_x, row_start_[i]);
    }

    tmp_y = tmp_row + Bottom();

    tmp_hpwl = EstimatedHPWL(block, tmp_x, tmp_y);
    tmp_displacement = std::abs(tmp_x - init_x) + std::abs(tmp_y - init_y);

    tmp_cost = tmp_displacement + tmp_hpwl;
    if (tmp_cost < min_cost) {
      best_loc = tmp_x;
      best_row = tmp_row;
      min_cost = tmp_cost;
    }
  }
  if (is_starting_block) {
    return true;
  }

  int res_x = best_loc;
  int res_y = best_row + Bottom();

  //std::cout << res.x << "  " << res.y << "  " << min_cost << "  " << block.Num() << "\n";

  if (min_cost < DBL_MAX) {
    block.SetLoc(res_x, res_y);
    return true;
  }

  return false;
}

void LGHillEx::PullLegalizationFromLeft() {
  row_start_.assign(row_start_.size(), Left());
  std::vector<Block> &block_list = *BlockList();
  for (size_t i = 0; i < index_loc_list_.size(); ++i) {
    index_loc_list_[i].num = i;
    index_loc_list_[i].x = block_list[i].LLX();
    index_loc_list_[i].y = block_list[i].LLY();
    block_list[i].SetPlaceStatus(UNPLACED);
  }
  std::sort(index_loc_list_.begin(), index_loc_list_.end());
  for (auto &pair: index_loc_list_) {
    auto &block = block_list[pair.num];
    bool is_cur_loc_legal = IsSpaceLegal(block);
    if (!is_cur_loc_legal) {
      bool loc_found = PullBlockLeft(block);
      if (!loc_found) {
        Assert(false, "Cannot find legal location");
      }
    }
    UseSpace(block);
    block.SetPlaceStatus(PLACED);
  }
}

bool LGHillEx::PullBlockRight(Block &block) {
  /****
   * For each row:
   *    if the row is not used before, do not change the location of this block (this kind of block is termed starting_block)
   *    if all rows are used, find the location with the minimum partial HPWL
   *    the partial HPWL is calculated in the following way:
   *        only calculate the HPWL of placed cells
   * Cost function is displacement
   * ****/
  int init_x = int(block.URX());
  int init_y = int(block.LLY());

  int height = int(block.Height());
  int start_row = 0;
  int end_row = Top() - Bottom() - height;
  //std::cout << "    Starting row: " << start_row << "\n"
  //          << "    Ending row:   " << end_row   << "\n"
  //          << "    Block Height: " << block.Height() << "\n"
  //          << "    Top of the placement region " << Top() << "\n"
  //          << "    Number of rows: " << row_start_.size() << "\n"
  //          << "    LX and LY: " << int(block.LLX()) << "  " << int(block.LLY()) << "\n";

  int best_row = 0;
  int best_loc = INT_MIN;
  double min_cost = DBL_MAX;
  double tmp_cost;
  int tmp_end_row = 0;
  int tmp_x;
  int tmp_y;

  bool is_starting_block = false;
  double tmp_hpwl = 0;
  double tmp_displacement = 0;
  for (int tmp_row = start_row; tmp_row <= end_row; ++tmp_row) {
    if (row_start_[tmp_row] == Right()) {
      is_starting_block = true;
      break;
    }

    tmp_end_row = tmp_row + height - 1;
    tmp_x = Right();
    for (int i = tmp_row; i <= tmp_end_row; ++i) {
      tmp_x = std::max(tmp_x, row_start_[i]);
    }

    tmp_y = tmp_row + Bottom();

    tmp_hpwl = EstimatedHPWL(block, tmp_x, tmp_y);
    tmp_displacement = std::abs(tmp_x - init_x) + std::abs(tmp_y - init_y);

    tmp_cost = tmp_displacement + tmp_hpwl;
    if (tmp_cost < min_cost) {
      best_loc = tmp_x;
      best_row = tmp_row;
      min_cost = tmp_cost;
    }
  }
  if (is_starting_block) {
    return true;
  }

  int res_x = best_loc;
  int res_y = best_row + Bottom();

  //std::cout << res.x << "  " << res.y << "  " << min_cost << "  " << block.Num() << "\n";

  if (min_cost < DBL_MAX) {
    block.SetURX(res_x);
    block.SetLLY(res_y);
    return true;
  }

  return false;
}

void LGHillEx::PullLegalizationFromRight() {
  row_start_.assign(row_start_.size(), Right());
  std::vector<Block> &block_list = *BlockList();
  for (size_t i = 0; i < index_loc_list_.size(); ++i) {
    index_loc_list_[i].num = i;
    index_loc_list_[i].x = block_list[i].URX();
    index_loc_list_[i].y = block_list[i].URY();
    block_list[i].SetPlaceStatus(UNPLACED);
  }
  std::sort(index_loc_list_.begin(),
            index_loc_list_.end(),
            [](const IndexLocPair<int> &lhs, const IndexLocPair<int> &rhs) {
              return (lhs.x > rhs.x) || (lhs.x == rhs.x && lhs.y > rhs.y);
            });
  for (auto &pair: index_loc_list_) {
    auto &block = block_list[pair.num];
    bool is_cur_loc_legal = IsSpaceLegal(block);
    if (!is_cur_loc_legal) {
      bool loc_found = PullBlockRight(block);
      if (!loc_found) {
        Assert(false, "Cannot find legal location");
      }
    }
    UseSpace(block);
    block.SetPlaceStatus(PLACED);
  }
}

bool LGHillEx::IsCurrentLocLegal(Value2D<int> &loc, int width, int height) {
  bool loc_out_range = (loc.x + width > right_) || (loc.x < left_) || (loc.y + height > top_) || (loc.y < bottom_);
  if (loc_out_range) {
    return false;
  }

  bool all_row_avail = true;
  int start_row = loc.y - bottom_;
  int end_row = start_row + height - 1;
  for (int i = start_row; i <= end_row; ++i) {
    if (row_start_[i] > loc.x) {
      all_row_avail = false;
      break;
    }
  }

  return all_row_avail;
}

bool LGHillEx::FindLoc(Value2D<int> &loc, int width, int height) {
  bool is_successful = true;

  int init_row;
  int left_bound;

  int max_search_row;
  int search_start_row;
  int search_end_row;

  int best_row;
  int best_loc_x;
  double min_cost;

  double tmp_cost;
  int tmp_end_row;
  int tmp_x;
  int tmp_y;

  left_bound = (int) std::round(loc.x - k_left_ * width);
  //left_bound = loc.x;

  init_row = int(loc.y - bottom_);
  max_search_row = top_ - bottom_ - height;

  search_start_row = std::max(0, init_row - 4 * height);
  search_end_row = std::min(max_search_row, init_row + 5 * height);

  best_row = 0;
  best_loc_x = INT_MIN;
  min_cost = DBL_MAX;

  for (int tmp_start_row = search_start_row; tmp_start_row <= search_end_row; ++tmp_start_row) {
    tmp_end_row = tmp_start_row + height - 1;
    tmp_x = std::max(left_, left_bound);

    for (int n = tmp_start_row; n <= tmp_end_row; ++n) {
      tmp_x = std::max(tmp_x, row_start_[n]);
    }

    //if (tmp_x + width > right_) continue;

    tmp_y = tmp_start_row + bottom_;
    //double tmp_hpwl = EstimatedHPWL(block, tmp_x, tmp_y);

    tmp_cost = std::abs(tmp_x - loc.x) + std::abs(tmp_y - loc.y);
    if (tmp_cost < min_cost) {
      best_loc_x = tmp_x;
      best_row = tmp_start_row;
      min_cost = tmp_cost;
    }
  }

  if (best_loc_x < left_ || best_loc_x + width > right_) {
    int old_start_row = search_start_row;
    int old_end_row = search_end_row;
    search_start_row = std::max(0, search_start_row - cur_iter_ * height);
    search_end_row = std::min(max_search_row, search_end_row + cur_iter_ * height);
    for (int tmp_start_row = search_start_row; tmp_start_row < old_start_row; ++tmp_start_row) {
      tmp_end_row = tmp_start_row + height - 1;
      tmp_x = std::max(left_, left_bound);

      for (int n = tmp_start_row; n <= tmp_end_row; ++n) {
        tmp_x = std::max(tmp_x, row_start_[n]);
      }

      if (tmp_x + width > right_) continue;

      tmp_y = tmp_start_row + bottom_;
      //double tmp_hpwl = EstimatedHPWL(block, tmp_x, tmp_y);

      tmp_cost = std::abs(tmp_x - loc.x) + std::abs(tmp_y - loc.y);
      if (tmp_cost < min_cost) {
        best_loc_x = tmp_x;
        best_row = tmp_start_row;
        min_cost = tmp_cost;
      }
    }
    for (int tmp_start_row = old_end_row; tmp_start_row < search_end_row; ++tmp_start_row) {
      tmp_end_row = tmp_start_row + height - 1;
      tmp_x = std::max(left_, left_bound);

      for (int n = tmp_start_row; n <= tmp_end_row; ++n) {
        tmp_x = std::max(tmp_x, row_start_[n]);
      }

      if (tmp_x + width > right_) continue;

      tmp_y = tmp_start_row + bottom_;
      //double tmp_hpwl = EstimatedHPWL(block, tmp_x, tmp_y);

      tmp_cost = std::abs(tmp_x - loc.x) + std::abs(tmp_y - loc.y);
      if (tmp_cost < min_cost) {
        best_loc_x = tmp_x;
        best_row = tmp_start_row;
        min_cost = tmp_cost;
      }
    }

  }

  // if still cannot find a legal location, enter fail mode
  is_successful = (best_loc_x >= left_) && (best_loc_x + width <= right_);

  loc.x = best_loc_x;
  loc.y = best_row + bottom_;

  return is_successful;
}

void LGHillEx::FastShift(int failure_point) {
  /****
   * This method is to FastShift() the blocks following the failure_point (included)
   * to reasonable locations in order to keep block orders
   *    1. when the current location of this block is illegal
   *    2. when there is no possible legal location on the right hand side of this block.
   * if failure_point is the first block
   * (this is not supposed to happen, if this happens, something is wrong with the global placement)
   *    we shift the bounding box of all blocks to the placement region,
   *    the bounding box left and bottom boundaries touch the left and bottom boundaries of placement region,
   *    note that the bounding box might be larger than the placement region, but should not be much larger
   * else:
   *    we shift the bounding box of the remaining blocks to the right hand side of the block just placed
   *    the bottom boundary of the bounding box will not be changed
   *    only the left boundary of the bounding box will be shifted to the right hand side of the block just placed
   * ****/
  std::vector<Block> &block_list = *BlockList();
  double bounding_left;
  if (failure_point == 0) {
    std::cout << "WARNING: unexpected case happens during legalization (failure point is 0)!\n";
  } else {
    double init_diff = index_loc_list_[failure_point - 1].x - index_loc_list_[failure_point].x;
    int failed_block = index_loc_list_[failure_point].num;
    bounding_left = block_list[failed_block].LLX();
    int last_placed_block = index_loc_list_[failure_point - 1].num;
    int left_new = (int) std::round(block_list[last_placed_block].LLX());
    //std::cout << left_new << "  " << bounding_left << "\n";
    int sz = index_loc_list_.size();
    int block_num;
    for (int i = failure_point; i < sz; ++i) {
      block_num = index_loc_list_[i].num;
      if (block_list[block_num].IsMovable()) {
        block_list[block_num].IncreX(left_new - bounding_left + init_diff);
      }
    }
  }
}

bool LGHillEx::LocalLegalization() {
  /****
   * 1. first sort all the circuit based on their location and size from low to high
   *    effective_loc = current_lx - k_width_ * width - k_height_ * height;
   * 2. for each cell, find the leftmost legal location, the location is left-bounded by:
   *    left_bound = current_lx - k_left_ * width;
   *    and
   *    left boundary of the placement region
   * 3. local search range is bounded by
   *    a). [left_bound, right_] (the range in the x direction)
   *    b). [init_y - height, init_y + 2 * height] (the range in the y direction)
   *    if legal location cannot be found in this range, extend the y_direction by height at each end
   * 4. if still no legal location can be found, do the reverse legalization procedure till reach the maximum iteration
   ****/
  bool is_successful = true;
  row_start_.assign(row_start_.size(), left_);
  std::vector<Block> &block_list = *BlockList();

  int sz = index_loc_list_.size();
  for (int i = 0; i < sz; ++i) {
    index_loc_list_[i].num = i;
    index_loc_list_[i].x = block_list[i].LLX() - k_width_ * block_list[i].Width() - k_height_ * block_list[i].Height();
    index_loc_list_[i].y = block_list[i].LLY();
  }
  std::sort(index_loc_list_.begin(), index_loc_list_.end());

  double init_x;
  double init_y;
  int height;
  int width;

  Value2D<int> res;
  bool is_current_loc_legal;
  bool is_legal_loc_found;

  int i;
  for (i = 0; i < sz; ++i) {
    //std::cout << i << "\n";
    auto &block = block_list[index_loc_list_[i].num];

    init_x = int(block.LLX());
    init_y = int(block.LLY());
    height = int(block.Height());
    width = int(block.Width());

    res.x = init_x;
    res.y = init_y;

    is_current_loc_legal = IsCurrentLocLegal(res, width, height);

    if (!is_current_loc_legal) {
      is_legal_loc_found = FindLoc(res, width, height);
      if (!is_legal_loc_found) {
        is_successful = false;
        //break;
      }
    }

    block.SetLoc(res.x, res.y);

    UseSpace(block);
  }

  /*if (!is_successful) {
    FastShift(i);
  }*/

  return is_successful;
}

bool LGHillEx::IsCurrentLocLegalRight(Value2D<int> &loc, int width, int height) {
  bool loc_out_range = (loc.x > right_) || (loc.x - width < left_) || (loc.y + height > top_) || (loc.y < bottom_);
  //std::cout << loc.y + height << "  " << loc_out_range << "\n";
  if (loc_out_range) {
    return false;
  }

  bool all_row_avail = true;
  int start_row = loc.y - bottom_;
  int end_row = start_row + height - 1;
  for (int i = start_row; i <= end_row; ++i) {
    if (row_start_[i] < loc.x) {
      all_row_avail = false;
      break;
    }
  }

  return all_row_avail;
}

bool LGHillEx::FindLocRight(Value2D<int> &loc, int width, int height) {
  bool is_successful = true;

  int init_row;
  int right_bound;

  int max_search_row;
  int search_start_row;
  int search_end_row;

  int best_row;
  int best_loc_x;
  double min_cost;

  double tmp_cost;
  int tmp_end_row;
  int tmp_x;
  int tmp_y;

  right_bound = (int) std::round(loc.x + k_left_ * width);
  //right_bound = loc.x;

  init_row = int(loc.y - bottom_);
  max_search_row = top_ - bottom_ - height;

  search_start_row = std::max(0, init_row - 4 * height);
  search_end_row = std::min(max_search_row, init_row + 5 * height);

  best_row = 0;
  best_loc_x = INT_MAX;
  min_cost = DBL_MAX;

  for (int tmp_start_row = search_start_row; tmp_start_row <= search_end_row; ++tmp_start_row) {
    tmp_end_row = tmp_start_row + height - 1;
    tmp_x = std::min(right_, right_bound);

    for (int n = tmp_start_row; n <= tmp_end_row; ++n) {
      tmp_x = std::min(tmp_x, row_start_[n]);
    }

    //if (tmp_x - width < left_) continue;

    tmp_y = tmp_start_row + bottom_;
    //double tmp_hpwl = EstimatedHPWL(block, tmp_x, tmp_y);

    tmp_cost = std::abs(tmp_x - loc.x) + std::abs(tmp_y - loc.y);
    if (tmp_cost < min_cost) {
      best_loc_x = tmp_x;
      best_row = tmp_start_row;
      min_cost = tmp_cost;
    }
  }

  if (best_loc_x > right_ || best_loc_x - width < left_) {
    int old_start_row = search_start_row;
    int old_end_row = search_end_row;
    search_start_row = std::max(0, search_start_row - cur_iter_ * height);
    search_end_row = std::min(max_search_row, search_end_row + cur_iter_ * height);
    for (int tmp_start_row = search_start_row; tmp_start_row <= old_start_row; ++tmp_start_row) {
      tmp_end_row = tmp_start_row + height - 1;
      tmp_x = std::min(right_, right_bound);

      for (int n = tmp_start_row; n <= tmp_end_row; ++n) {
        tmp_x = std::min(tmp_x, row_start_[n]);
      }

      if (tmp_x - width < left_) continue;

      tmp_y = tmp_start_row + bottom_;
      //double tmp_hpwl = EstimatedHPWL(block, tmp_x, tmp_y);

      tmp_cost = std::abs(tmp_x - loc.x) + std::abs(tmp_y - loc.y);
      if (tmp_cost < min_cost) {
        best_loc_x = tmp_x;
        best_row = tmp_start_row;
        min_cost = tmp_cost;
      }
    }
    for (int tmp_start_row = old_end_row; tmp_start_row <= search_end_row; ++tmp_start_row) {
      tmp_end_row = tmp_start_row + height - 1;
      tmp_x = std::min(right_, right_bound);

      for (int n = tmp_start_row; n <= tmp_end_row; ++n) {
        tmp_x = std::min(tmp_x, row_start_[n]);
      }

      if (tmp_x - width < left_) continue;

      tmp_y = tmp_start_row + bottom_;
      //double tmp_hpwl = EstimatedHPWL(block, tmp_x, tmp_y);

      tmp_cost = std::abs(tmp_x - loc.x) + std::abs(tmp_y - loc.y);
      if (tmp_cost < min_cost) {
        best_loc_x = tmp_x;
        best_row = tmp_start_row;
        min_cost = tmp_cost;
      }
    }

  }

  // if still cannot find a legal location, enter fail mode
  is_successful = (best_loc_x - width >= left_) || (best_loc_x <= right_);

  loc.x = best_loc_x;
  loc.y = best_row + bottom_;

  return is_successful;
}

void LGHillEx::FastShiftRight(int failure_point) {
  /****
   * This method is to FastShift() the blocks following the failure_point (included)
   * to reasonable locations in order to keep block orders
   *    1. when the current location of this block is illegal
   *    2. when there is no possible legal location on the right hand side of this block.
   * if failure_point is the first block
   * (this is not supposed to happen, if this happens, something is wrong with the global placement)
   *    we shift the bounding box of all blocks to the placement region,
   *    the bounding box left and bottom boundaries touch the left and bottom boundaries of placement region,
   *    note that the bounding box might be larger than the placement region, but should not be much larger
   * else:
   *    we shift the bounding box of the remaining blocks to the right hand side of the block just placed
   *    the bottom boundary of the bounding box will not be changed
   *    only the left boundary of the bounding box will be shifted to the right hand side of the block just placed
   * ****/
  std::vector<Block> &block_list = *BlockList();
  double bounding_right;
  if (failure_point == 0) {
    std::cout << "WARNING: unexpected case happens during legalization (reverse failure point is 0)!\n";

  } else {
    double init_diff = index_loc_list_[failure_point - 1].x - index_loc_list_[failure_point].x;
    bounding_right = index_loc_list_[failure_point].x;
    int last_placed_block = index_loc_list_[failure_point - 1].num;
    int right_new = (int) std::round(block_list[last_placed_block].URX());
    //std::cout << left_new << "  " << bounding_left << "\n";
    int sz = index_loc_list_.size();
    int block_num = -1;
    for (int i = failure_point; i < sz; ++i) {
      block_num = index_loc_list_[i].num;
      if (block_list[block_num].IsMovable()) {
        block_list[block_num].DecreX(right_new - bounding_right + init_diff);
      }
    }
  }
}

bool LGHillEx::LocalLegalizationRight() {
  /****
   * 1. first sort all the circuit based on their location and size from high to low
   *    effective_loc = current_rx - k_width_ * width - k_height_ * height;
   * 2. for each cell, find the rightmost legal location, the location is right-bounded by:
   *    right_bound = current_rx + k_left_ * width;
   *    and
   *    right boundary of the placement region
   * 3. local search range is bounded by
   *    a). [left_, right_bound] (the range in the x direction)
   *    b). [init_y - height, init_y + 2 * height] (the range in the y direction)
   *    if legal location cannot be found in this range, extend the y_direction by height at each end
   * 4. if still no legal location can be found, do the reverse legalization procedure till reach the maximum iteration
   ****/
  row_start_.assign(row_start_.size(), right_);
  std::vector<Block> &block_list = *BlockList();

  int sz = index_loc_list_.size();
  for (int i = 0; i < sz; ++i) {
    index_loc_list_[i].num = i;
    index_loc_list_[i].x = block_list[i].URX() + k_width_ * block_list[i].Width() + k_height_ * block_list[i].Height();
    index_loc_list_[i].y = block_list[i].LLY();
  }
  std::sort(index_loc_list_.begin(),
            index_loc_list_.end(),
            [](const IndexLocPair<int> &lhs, const IndexLocPair<int> &rhs) {
              return (lhs.x > rhs.x) || (lhs.x == rhs.x && lhs.y > rhs.y);
            });

  int init_x;
  int init_y;
  int height;
  int width;

  bool is_current_loc_legal;
  bool is_legal_loc_found;

  Value2D<int> res;
  bool is_successful = true;
  int i;
  for (i = 0; i < sz; ++i) {
    //std::cout << i << "\n";
    auto &block = block_list[index_loc_list_[i].num];

    init_x = int(block.URX());
    init_y = int(block.LLY());
    height = int(block.Height());
    width = int(block.Width());

    res.x = init_x;
    res.y = init_y;

    is_current_loc_legal = IsCurrentLocLegalRight(res, width, height);

    if (!is_current_loc_legal) {
      is_legal_loc_found = FindLocRight(res, width, height);
      if (!is_legal_loc_found) {
        is_successful = false;
        //break;
      }
    }

    block.SetURX(res.x);
    block.SetLLY(res.y);

    //std::cout << res.x << "  " << res.y << "  " << block.Num() << "\n";

    auto start_row = int(block.LLY() - Bottom());
    unsigned int end_row = start_row + block.Height() - 1;
    /*if (end_row >= row_start_.size()) {
      std::cout << "  ly:     " << block.LLY() << "\n"
                << "  height: " << block.Height() << "\n"
                << "  top:    " << Top() << "\n"
                << "  bottom: " << Bottom() << "\n"
                << "  is legal: " << is_current_loc_legal << "\n";
      Assert(false, "Cannot use space out of range");
    }*/

    assert(end_row < row_start_.size());
    assert(start_row >= 0);

    int end_x = int(block.LLX());
    for (unsigned int r = start_row; r <= end_row; ++r) {
      row_start_[r] = end_x;
    }
  }

  /*if (!is_successful) {
    FastShiftRight(i);
  }*/

  return is_successful;
}

void LGHillEx::StartPlacement() {
  double wall_time = get_wall_time();
  double cpu_time = get_cpu_time();

  InitLegalizer();

  if (globalVerboseLevel >= LOG_CRITICAL) {
    std::cout << "Start PushPull Legalization\n";
  }

  bool is_success = false;
  for (cur_iter_ = 0; cur_iter_ < max_iter_; ++cur_iter_) {
    if (legalize_from_left_) {
      is_success = LocalLegalization();
    } else {
      is_success = LocalLegalizationRight();
    }
    legalize_from_left_ = !legalize_from_left_;
    ++k_left_;
    GenMATLABTable("lg" + std::to_string(cur_iter_) + "_result.txt");
    ReportHPWL(LOG_CRITICAL);
    if (is_success) {
      break;
    }
  }
  if (!is_success) {
    std::cout << "Legalization fails\n";
  }

  if (globalVerboseLevel >= LOG_CRITICAL) {
    std::cout << "\033[0;36m"
              << "PushPull Legalization complete (" << cur_iter_ + 1 << ")\n"
              << "\033[0m";
  }

  ReportHPWL(LOG_CRITICAL);

  wall_time = get_wall_time() - wall_time;
  cpu_time = get_cpu_time() - cpu_time;
  if (globalVerboseLevel >= LOG_CRITICAL) {
    std::cout << "(wall time: "
              << wall_time << "s, cpu time: "
              << cpu_time << "s)\n";
  }

}