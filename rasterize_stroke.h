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
  void RasterizeStroke(
      const int8_t* stroke_points,
      int stroke_points_count,
      float x_range,
      float y_range,
      int width,
      int height,
      int8_t* out_buffer);

#endif   // TENSORFLOW_LITE_MICRO_EXAMPLES_SONIC_SCREWDRIVER_RASTERIZE_STROKE_H
