#ifndef SONIC_SCREWDRIVER_IMU_PROVIDER_H
#define SONIC_SCREWDRIVER_IMU_PROVIDER_H

#ifdef NANO33_BLE_REV2
#include <Arduino_BMI270_BMM150.h>
#else
#include <Arduino_LSM9DS1.h>
#endif

#include <ArduinoBLE.h>

#include "rasterize_stroke.h"

constexpr int stroke_transmit_stride = 2;
constexpr int stroke_transmit_max_length = 160;
constexpr int stroke_max_length = stroke_transmit_max_length * stroke_transmit_stride;
constexpr int stroke_points_byte_count = 2 * sizeof(int8_t) * stroke_transmit_max_length;
constexpr int stroke_struct_byte_count = (2 * sizeof(int32_t)) + stroke_points_byte_count;
constexpr int moving_sample_count = 50;
constexpr int acceleration_data_length = 600 * 3;
constexpr int gyroscope_data_length = 600 * 3;
constexpr long max_movement_wait = 500L;

enum COORDINATES {
  C_X = 0,
  C_Y = 1,
  C_Z = 2
};

class ImuProvider {
  long movement_stopped = 0L;
  bool done_just_triggered = false;
  float current_velocity[3] = {0.0f, 0.0f, 0.0f};
  float current_gravity[3] = {0.0f, 0.0f, 0.0f};
  float current_gyroscope_drift[3] = {0.0f, 0.0f, 0.0f};
  
  int32_t *stroke_state = reinterpret_cast<int32_t*>(stroke_struct_buffer);

  // A buffer holding the last 600 sets of 3-channel values from the accelerometer.
  float acceleration_data[acceleration_data_length] = {};
  // The next free entry in the data array.
  int acceleration_data_index = 0;
  float acceleration_sample_rate = 0.0f;
  
  // A buffer holding the last 600 sets of 3-channel values from the gyroscope.
  float gyroscope_data[gyroscope_data_length] = {};
  float orientation_data[gyroscope_data_length] = {};
  // The next free entry in the data array.
  int gyroscope_data_index = 0;
  float gyroscope_sample_rate = 0.0f;
  
  enum GestureState {
    eWaiting,
    eDrawing,
    ePausing,
    eDone
  };

  Rasterizer rasterizer;

protected:
  friend class Rasterizer;

  int32_t stroke_length = 0;
  uint8_t stroke_struct_buffer[stroke_struct_byte_count] = {};
  int8_t *stroke_points = reinterpret_cast<int8_t*>(stroke_struct_buffer + sizeof(int32_t) * 2);
  int32_t *stroke_transmit_length = reinterpret_cast<int32_t*>(stroke_struct_buffer + sizeof(int32_t));

public:
  void SetupIMU();
  bool doneJustTriggered() const;
  void ReadAccelerometerAndGyroscope(int *new_accelerometer_samples, int *new_gyroscope_samples);
  void ReadGyroscope(int *new_gyroscope_samples);
  void EstimateGravityDirection();
  void UpdateVelocity(int new_samples);
  void EstimateGyroscopeDrift();
  void UpdateOrientation(int new_samples);
  void UpdateStroke(int new_samples);
  void writeStroke(BLECharacteristic& strokeCharacteristic) {
    strokeCharacteristic.writeValue(stroke_struct_buffer, stroke_struct_byte_count);
  }
  void RasterizeStroke(
    float x_range,
    float y_range,
    int width,
    int height,
    int8_t* out_buffer);
protected:
  float VectorMagnitude(const float* vec) ;
  void NormalizeVector(const float* in_vec, float* out_vec);

  // returning a python tuple in C++ ?
  // static float DotProduct(const float* a, const float* b) {
  //   return (a[0] * b[0], a[1] * b[1], a[2] * b[2]);
  // }

  bool IsMoving(int samples_before);

  // Store a single capped scaled value as signed byte to target buffer position
  static void storeCappedValue(float axis, int8_t *stroke_entry);

  int getStartIndex(int pos) const {
    return (gyroscope_data_index + (gyroscope_data_length - 3 * pos)) % gyroscope_data_length;
  }
};

#endif   // SONIC_SCREWDRIVER_IMU_PROVIDER_H
