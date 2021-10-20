//
// Created by Yihang Yang on 8/7/19.
//

#ifndef DALI_DALI_PLACER_GLOBALPLACER_GPSIMPL_CELLCUTPOINT_H_
#define DALI_DALI_PLACER_GLOBALPLACER_GPSIMPL_CELLCUTPOINT_H_

#include <iostream>

namespace dali {

struct CellCutPoint {
  CellCutPoint() : x(0), y(0) {}
  CellCutPoint(double x0, double y0) : x(x0), y(y0) {}
  double x;
  double y;

  void init() {
    x = 0;
    y = 0;
  };
  bool operator<(const CellCutPoint &rhs) const {
    return (x < rhs.x) || ((x == rhs.x) && (y < rhs.y));
  }
  bool operator>(const CellCutPoint &rhs) const {
    return (x > rhs.x) || ((x == rhs.x) && (y > rhs.y));
  }
  bool operator==(const CellCutPoint &rhs) const {
    return (x == rhs.x) && (y == rhs.y);
  }
};

}

#endif //DALI_DALI_PLACER_GLOBALPLACER_GPSIMPL_CELLCUTPOINT_H_