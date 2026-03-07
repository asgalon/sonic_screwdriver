/* Copyright 2020 The TensorFlow Authors. All Rights Reserved.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#ifndef TENSORFLOW_LITE_MICRO_EXAMPLES_SONIC_SCREWDRIVER_RASTERIZE_STROKE_H
#define TENSORFLOW_LITE_MICRO_EXAMPLES_SONIC_SCREWDRIVER_RASTERIZE_STROKE_H

#include <cstdint>

constexpr int kFixedPoint = 256;

//
// Rasterizer lightweight class, does not have any data members
//
class Rasterizer {
  static int32_t MulFP(int32_t a, int32_t b) {
    return a * b / kFixedPoint;
  }

  static int32_t DivFP(int32_t a, int32_t b) {
    if (b == 0) {
      b = 1;
    }
    return a * kFixedPoint / b;
  }

  static int32_t FloatToFP(float a) {
    return static_cast<int32_t>(a * kFixedPoint);
  }

  static int32_t NormToCoordFP(int32_t a_fp, int32_t range_fp, int32_t half_size_fp) {
    const int32_t norm_fp = DivFP(a_fp, range_fp);
    return MulFP(norm_fp, half_size_fp) + half_size_fp;
  }

  static int32_t RoundFPToInt(int32_t a) {
    return (a + kFixedPoint / 2) / kFixedPoint;
  }

  static int32_t Gate(int32_t a, int32_t min, int32_t max) {
    if (a < min) {
      return min;
    } else if (a > max) {
      return max;
    } else {
      return a;
    }
  }

  static int32_t Abs(int32_t a) {
    if (a > 0) {
      return a;
    } else {
      return -a;
    }
  }

public:
  void RasterizeStroke(
      const int8_t* stroke_points,
      int stroke_points_count,
      float x_range,
      float y_range,
      int width,
      int height,
      int8_t* out_buffer);
};

#endif   // TENSORFLOW_LITE_MICRO_EXAMPLES_SONIC_SCREWDRIVER_RASTERIZE_STROKE_H
