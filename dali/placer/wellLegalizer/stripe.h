//
// Created by Yihang Yang on 4/12/21.
//

#ifndef DALI_DALI_PLACER_WELLLEGALIZER_STRIPE_H_
#define DALI_DALI_PLACER_WELLLEGALIZER_STRIPE_H_

#include "cluster.h"
#include "dali/circuit/block.h"
#include "dali/common/misc.h"

namespace dali {

struct Stripe {
    int lx_;
    int ly_;
    int width_;
    int height_;
    int max_blk_capacity_per_cluster_;

    int contour_;
    int used_height_;
    int cluster_count_;
    Cluster *front_cluster_;
    std::vector<Cluster> cluster_list_;
    bool is_bottom_up_ = false;

    int block_count_;
    std::vector<Block *> block_list_;

    bool is_first_row_orient_N_ = true;
    std::vector<RectI> well_rect_list_;

    int LLX() const { return lx_; }
    int LLY() const { return ly_; }
    int URX() const { return lx_ + width_; }
    int URY() const { return ly_ + height_; }
    int Width() const { return width_; }
    int Height() const { return height_; }
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

enum StripePartitionMode {
    STRICT = 0,
    SCAVENGE = 1
};

}

#endif //DALI_DALI_PLACER_WELLLEGALIZER_STRIPE_H_