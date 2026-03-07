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
  static int32_t MulFP(const int32_t a, const int32_t b) {
    return a * b / kFixedPoint;
  }

  static int32_t DivFP(const int32_t a, const int32_t b) {
    int32_t result = a * kFixedPoint;

    return b != 0 ? result / b : result;
  }

  static int32_t FloatToFP(const float a) {
    return static_cast<int32_t>(a * kFixedPoint);
  }

  static int32_t NormToCoordFP(const int32_t a_fp, const int32_t range_fp, const int32_t half_size_fp) {
    const int32_t norm_fp = DivFP(a_fp, range_fp);
    return MulFP(norm_fp, half_size_fp) + half_size_fp;
  }

  static int32_t RoundFPToInt(const int32_t a) {
    return (a + kFixedPoint / 2) / kFixedPoint;
  }

  static int32_t Gate(const int32_t a, const int32_t min, const int32_t max) {
    if (a < min) {
      return min;
    }
    if (a > max) {
      return max;
    }
    return a;
  }

  static int32_t Abs(const int32_t a) {
    if (a >= 0) {
      return a;
    }
    return -a;
  }

public:
  /**
   * rasterize the stroke into width x height box.
   * Rasterize stroke
   * @param stroke_points array of recorded stroke points
   * @param stroke_points_count number of stroke points
   * @param x_range fill factor for x range
   * @param y_range fill factor for y range
   * @param width width of the raster image
   * @param height height of the raster image
   * @param out_buffer raster image output buffer
   */
  static void RasterizeStroke(
      const int8_t* stroke_points,
      int stroke_points_count,
      float x_range,
      float y_range,
      int width,
      int height,
      int8_t* out_buffer);
};

#endif   // TENSORFLOW_LITE_MICRO_EXAMPLES_SONIC_SCREWDRIVER_RASTERIZE_STROKE_H
