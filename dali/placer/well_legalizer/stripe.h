/*******************************************************************************
 *
 * Copyright (c) 2021 Yihang Yang
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 ******************************************************************************/
#ifndef DALI_DALI_PLACER_WELLLEGALIZER_STRIPE_H_
#define DALI_DALI_PLACER_WELLLEGALIZER_STRIPE_H_

#include "dali/circuit/block.h"
#include "dali/circuit/circuit.h"
#include "dali/common/config.h"
#include "dali/common/misc.h"
#include "dali/placer/well_legalizer/griddedrow.h"

#if DALI_USE_CPLEX
#include <ilcplex/ilocplex.h>
ILOSTLBEGIN
#endif

namespace dali {

class Stripe {
 public:
  int lx_;
  int ly_;
  int width_;
  int height_;
  int max_blk_capacity_per_cluster_;

  int contour_;
  int used_height_;
  int cluster_count_;
  GriddedRow *front_row_;
  int front_id_;
  std::vector<GriddedRow> gridded_rows_;
  bool is_bottom_up_ = false;

  int block_count_;
  std::vector<Block *> block_list_;
  std::unordered_map<Block *, int> blk_ptr_2_row_id_;

  bool is_first_row_orient_N_ = true;
  std::vector<RectI> well_rect_list_;

  bool is_checkerboard_mode_ = false;
  int well_tap_cell_width_ = -1;
  std::vector<SegI> well_tap_cell_location_even_;
  std::vector<SegI> well_tap_cell_location_odd_;

  int LLX() const { return lx_; }
  int LLY() const { return ly_; }
  int URX() const { return lx_ + width_; }
  int URY() const { return ly_ + height_; }
  int Width() const { return width_; }
  int Height() const { return height_; }

  bool HasNoRowsSpillingOut() const;

  void MinDisplacementAdjustment();

  void SortBlocksBasedOnLLY();
  void SortBlocksBasedOnURY();
  void SortBlocksBasedOnStretchedURY();
  void SortBlocksBasedOnYLocation(int criterion);

  void PrecomputeWellTapCellLocation(
      bool is_checker_board_mode,
      int tap_cell_interval_grid,
      BlockType *well_tap_type_ptr
  );

  void UpdateFrontClusterUpward(int p_height, int n_height);
  void SimplyAddFollowingClusters(Block *p_blk, bool is_upward);
  bool AddBlockToFrontCluster(Block *p_blk, bool is_upward);
  size_t FitBlocksToFrontSpaceUpward(size_t start_id, int current_iteration);
  void LegalizeFrontCluster();
  void UpdateRemainingClusters(int p_height, int n_height, bool is_upward);
  void UpdateBlockStretchLength();

  void UpdateFrontClusterDownward(int p_height, int n_height);
  size_t FitBlocksToFrontSpaceDownward(size_t start_id, int current_iteration);

  void UpdateBlockYLocation();

  size_t AddWellTapCells(
      Circuit *p_ckt,
      BlockType *well_tap_type_ptr,
      size_t start_id
  );

  bool IsLeftmostPlacementLegal();

  void BreakMultiRowCellIntoSingleRowCell();
  void OptimizeDisplacementInEachRow();
  void ComputeAverageLocationForMultiRowCells();
  void IterativeCellReordering();
  void ClearMultiRowCellBreaking();

#if DALI_USE_CPLEX
  std::unordered_map<Block *, IloInt> blk_ptr_2_tmp_id;
  std::unordered_map<IloInt, Block *> blk_tmp_id_2_ptr;
  std::unordered_map<Block *, double> init_x_locs_;
  void SaveInitialLocationX();
  void RestoreInitialLocationX();
  void SortBlocksInEachRow();
  void PopulateVariableArray(IloModel &model, IloNumVarArray &x);
  void AddVariableConstraints(
      IloModel &model, IloNumVarArray &x, IloRangeArray &c
  );
  void ConstructQuadraticObjective(IloModel &model, IloNumVarArray &x);
  void CreateQPModel(IloModel &model, IloNumVarArray &x, IloRangeArray &c);
  void SolveQPProblem(IloCplex &cplex, IloNumVarArray &var);
  bool OptimizeDisplacementUsingQuadraticProgramming();
#endif
};

struct ClusterStripe {
  int lx_;
  int width_;

  int block_count_;
  std::vector<Block *> block_list_;

  std::vector<RectI> well_rect_list_;

  std::vector<std::vector<SegI>> white_space_; // white space in each row
  std::vector<Stripe> stripe_list_;

  int Width() const { return width_; }
  int LLX() const { return lx_; }
  int URX() const { return lx_ + width_; }
  Stripe *GetStripeMatchSeg(SegI seg, int y_loc);
  Stripe *GetStripeMatchBlk(Block *blk_ptr);
  Stripe *GetStripeClosestToBlk(Block *blk_ptr, double &distance);
  void AssignBlockToSimpleStripe();
};

}

#endif //DALI_DALI_PLACER_WELLLEGALIZER_STRIPE_H_
