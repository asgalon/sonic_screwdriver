//
// Created by Peter Köllner on 7/3/26.
//

#include "imu_provider.h"
#include "util.h"

#include <cmath>

void ImuProvider::SetupIMU() {

  // Make sure we are pulling measurements into a FIFO.
  // If you see an error on this line, make sure you have at least v1.1.0 of the
  // Arduino_LSM9DS1 library installed.
  IMU.setContinuousMode();

  acceleration_sample_rate = IMU.accelerationSampleRate();
  gyroscope_sample_rate = IMU.gyroscopeSampleRate();
}

bool ImuProvider::doneJustTriggered() const {
  return done_just_triggered;
}

void ImuProvider::ReadAccelerometerAndGyroscope(int *new_accelerometer_samples, int *new_gyroscope_samples) {
  // Keep track of whether we stored any new data
  *new_accelerometer_samples = 0;
  *new_gyroscope_samples = 0;
  // Loop through new samples and add to buffer
  if (IMU.accelerationAvailable()) {
    ReadGyroscope(new_gyroscope_samples);

    const int acceleration_index = (acceleration_data_index % acceleration_data_length);
    acceleration_data_index += 3;
    float* current_acceleration_data = &acceleration_data[acceleration_index];
    // Read each sample, removing it from the device's FIFO buffer
    if (!IMU.readAcceleration(
        current_acceleration_data[0], current_acceleration_data[1], current_acceleration_data[2])) {
      Serial.println("Failed to read acceleration data");
    }
    *new_accelerometer_samples += 1;
  }
}

void ImuProvider::ReadGyroscope(int *new_gyroscope_samples) {
  // Loop through new samples and add to buffer
  const int gyroscope_index = gyroscope_data_index % gyroscope_data_length;
  gyroscope_data_index += 3;

  float* current_gyroscope_data = &gyroscope_data[gyroscope_index];
  // Read each sample, removing it from the device's FIFO buffer
  if (!IMU.readGyroscope(current_gyroscope_data[0], current_gyroscope_data[1], current_gyroscope_data[2])) {
    Serial.println("Failed to read gyroscope data");
  }
  *new_gyroscope_samples += 1;
}

void ImuProvider::EstimateGravityDirection() {
  int samples_to_average = 100;
  if (samples_to_average >= acceleration_data_index) {
    samples_to_average = acceleration_data_index;
  }

  const int start_index = getStartIndex(samples_to_average + 1);

  float total[3] = { 0.0f, 0.0f, 0.0f };

  for (int i = 0; i < samples_to_average; ++i) {
    const int index = ((start_index + (i * 3)) % acceleration_data_length);
    const float* entry = &acceleration_data[index];

    for (int k = C_X; k <= C_Z; k++) {
      total[k] += entry[k];
    }
  }

  for (int k = C_X; k <= C_Z; k++) {
    current_gravity[k] = total[k] / samples_to_average;
  }
}

void ImuProvider::UpdateVelocity(int new_samples) {
  const int start_index = getStartIndex(new_samples + 1);

  for (int i = 0; i < new_samples; ++i) {
    const int index = ((start_index + (i * 3)) % acceleration_data_length);
    const float* entry = &acceleration_data[index];

    for (int k = C_X; k <= C_Z; k++) {
      constexpr float friction_fudge = 0.98f;
      // Try to remove gravity from the raw acceleration values.
      const float a_minus_gravity = entry[k] - current_gravity[k];
      // Update velocity based on the normalized acceleration.
     current_velocity[k] += a_minus_gravity;

      // Dampen the velocity slightly with a fudge factor to stop it exploding.
      current_velocity[k] *= friction_fudge;
    }
  }
}

void ImuProvider::EstimateGyroscopeDrift() {
  // Estimate and update the drift of the gyroscope when the Ardiuno is not moving
  const bool isMoving = VectorMagnitude(current_velocity) > 0.1f;
  if (isMoving) {
    return;
  }

  int samples_to_average = 20;
  if (samples_to_average >= gyroscope_data_index) {
    samples_to_average = gyroscope_data_index;
  }

  const int start_index = getStartIndex(samples_to_average + 1);

  float total[3] =  { 0.0f, 0.0f, 0.0f };

  for (int i = 0; i < samples_to_average; ++i) {
    const int index = ((start_index + (i * 3)) % gyroscope_data_length);
    const float* entry = &gyroscope_data[index];

    for (int k = C_X; k <= C_Z; k++) {
      total[k] += entry[k];
    }
  }

  for (int k = C_X; k <= C_Z; k++) {
    current_gyroscope_drift[k] = total[k] / samples_to_average;
  }
}

void ImuProvider::UpdateOrientation(int new_samples) {
  //update the current orientation by integrating the angular velocity over time

  const int start_index = getStartIndex(new_samples);

  // The gyroscope values are in degrees-per-second, so to approximate
  // degrees in the integrated orientation, we need to divide each value
  // by the number of samples each second.
  const float recip_sample_rate = 1.0f / gyroscope_sample_rate;

  for (int i = 0; i < new_samples; ++i) {
    const int index = ((start_index + (i * 3)) % gyroscope_data_length);
    const float* entry = &gyroscope_data[index];
    float* current_orientation = &orientation_data[index];
    const int previous_index = (index + (gyroscope_data_length - 3)) % gyroscope_data_length;
    const float* previous_orientation = &orientation_data[previous_index];

    for (int k = C_X; k <= C_Z; k++) {
      // Try to remove sensor errors from the raw gyroscope values.
      const float d_minus_drift = entry[k] - current_gyroscope_drift[k];

      // Convert from degrees-per-second to appropriate units for this
      // time interval.
      float d_normalized = d_minus_drift * recip_sample_rate;

      // Update orientation based on the gyroscope data.
      current_orientation[k] = previous_orientation[k] + d_normalized;
    }
  }
}


void ImuProvider::UpdateStroke(int new_samples) {
  //Take the angular values and project them into an XY plane

  constexpr int minimum_stroke_length = moving_sample_count + 10;

  done_just_triggered = false;

  //iterate through the new samples
  for (int i = 0; i < new_samples; ++i) {
    const int current_head = new_samples - 1 - i;
    const bool is_moving = IsMoving(current_head);
    const int32_t old_state = *stroke_state;

    //determine if there is a break between gestures
    switch (old_state) {
      case eWaiting:
      case eDone:
        if (is_moving) {
          stroke_length = moving_sample_count;
          *stroke_state = eDrawing;
        }
        break;
      case eDrawing:
        if (!is_moving) {
          if (stroke_length > minimum_stroke_length) {
            movement_stopped = millis();
            *stroke_state = ePausing;
          } else {
            stroke_length = 0;
            *stroke_state = eWaiting;
          }
        }
        break;
      case ePausing: {
          if (!is_moving) {
            const long now = millis();
            if (now - movement_stopped > 500) {
              *stroke_state = eDone;
            }
          } else {
            *stroke_state = eDrawing;
          }
        }
        break;
      default: ;
    }

    //if the stroke is too small we skip to the next iteration
    const bool is_waiting = *stroke_state == eWaiting;
    if (is_waiting) {
      continue;
    }

    stroke_length += 1;
    if (stroke_length > stroke_max_length) {
      stroke_length = stroke_max_length;
    }

    // Only recalculate the full stroke if it's needed.
    const bool draw_last_point = i == new_samples - 1 && *stroke_state == eDrawing;
    done_just_triggered = old_state != eDone && *stroke_state == eDone;
    if (!(done_just_triggered || draw_last_point)) {
      continue;
    }

    const int start_index = getStartIndex(stroke_length + current_head);

    //accumulate the x, y, and z orientation data
    float total[3] = { 0.0f, 0.0f, 0.0f };

    for (int j = 0; j < stroke_length; ++j) {
      const int index = ((start_index + (j * 3)) % gyroscope_data_length);
      const float* entry = &orientation_data[index];
      for (int n= C_X; n <= C_Z; n++) {
        total[n] += entry[n];
      }
    }

    const float mean[3] = {
      total[C_X] / stroke_length,
      total[C_Y] / stroke_length,
      total[C_Z] / stroke_length
    };

    //Account for the roll orientation of the Arduino
    const float gy = current_gravity[1];
    const float gz = current_gravity[2];
    float gmag = sqrtf(gy * gy + gz * gz);
    if (gmag < 0.0001f) {
      gmag = 0.0001f;
    }
    const float ngy = gy / gmag;
    const float ngz = gz / gmag;

    const float xaxisz = -ngz;
    const float xaxisy = -ngy;

    const float yaxisz = -ngy;
    const float yaxisy = ngz;

    *stroke_transmit_length = stroke_length / stroke_transmit_stride;

    //project the angular orientation into the 2d X/Y plane
    float min[2];
    float max[2];

    for (int j = 0; j < *stroke_transmit_length; ++j) {
      const int orientation_index = (start_index + j * stroke_transmit_stride * 3) % gyroscope_data_length;
      const float* orientation_entry = &orientation_data[orientation_index];

      float n[3];

      for (int k = C_X; k <= C_Z; k++) {
        constexpr float range = 90.0f;
        n[k] = (orientation_entry[k] - mean[k]) / range;
      }

      const float x_axis = xaxisz * n[C_Z] + xaxisy * n[C_Y];
      const float y_axis = yaxisz * n[C_Z] + yaxisy * n[C_Y];

      const int stroke_index = j * 2;
      int8_t *stroke_entry = &stroke_points[stroke_index];

      //cap the x/y values at -128 and 127 (int8)
      storeCappedValue(x_axis, stroke_entry);
      storeCappedValue(y_axis, &stroke_entry[1]);

      const bool is_first = j == 0;
      if (is_first || x_axis < min[C_X]) {
        min[C_X] = x_axis;
      }
      if (is_first || y_axis < min[C_Y]) {
        min[C_Y] = y_axis;
      }
      if (is_first || x_axis > max[C_X]) {
        max[C_X] = x_axis;
      }
      if (is_first || y_axis > max[C_Y]) {
        max[C_Y] = y_axis;
      }
    }

    // If the stroke is too small, cancel it.
    if (done_just_triggered) {
      constexpr float minimum_stroke_size = 0.2f;
      const float x_range = max[C_X] - min[C_X];
      const float y_range = max[C_Y] - min[C_Y];
      if (x_range < minimum_stroke_size &&
        y_range < minimum_stroke_size) {
        done_just_triggered = false;
        *stroke_state = eWaiting;
        *stroke_transmit_length = 0;
        stroke_length = 0;
      }
    }
  }
}

void ImuProvider::RasterizeStroke(float x_range, float y_range, int width, int height, int8_t* out_buffer) {
  rasterizer.RasterizeStroke(stroke_points, *stroke_transmit_length, x_range, y_range, width, height, out_buffer);
}

float ImuProvider::VectorMagnitude(const float* vec) {
  const float x = vec[0];
  const float y = vec[1];
  const float z = vec[2];
  return sqrtf(x * x + y * y + z * z);
}

void ImuProvider::NormalizeVector(const float* in_vec, float* out_vec) {
  const float magnitude = VectorMagnitude(in_vec);
  const float x = in_vec[0];
  const float y = in_vec[1];
  const float z = in_vec[2];
  out_vec[0] = x / magnitude;
  out_vec[1] = y / magnitude;
  out_vec[2] = z / magnitude;
}

// returning a python tuple in C++ ?
// static float DotProduct(const float* a, const float* b) {
//   return (a[0] * b[0], a[1] * b[1], a[2] * b[2]);
// }

bool ImuProvider::IsMoving(int samples_before) {
  // calculate if the Arduino is moving using the mean squared difference
  // of the current and previous gyroscope data
  // Note: this is different from how we calculate isMoving in EstimateGyroscopeDrift()
  constexpr float moving_threshold = 10.0f;

  if ((gyroscope_data_index - samples_before) < moving_sample_count) {
    return false;
  }

  const int start_index = getStartIndex(moving_sample_count + samples_before);

  float total = 0.0f;

  for (int i = 0; i < moving_sample_count; ++i) {
    const int index = ((start_index + (i * 3)) % gyroscope_data_length);
    float* current_orientation = &orientation_data[index];
    const int previous_index = (index + (gyroscope_data_length - 3)) % gyroscope_data_length;
    const float* previous_orientation = &orientation_data[previous_index];
    const float dx = current_orientation[0] - previous_orientation[0];
    const float dy = current_orientation[1] - previous_orientation[1];
    const float dz = current_orientation[2] - previous_orientation[2];
    const float mag_squared = dx * dx + dy * dy + dz * dz;
    total += mag_squared;
  }
  const bool is_moving = total > moving_threshold;
  return is_moving;
}

void ImuProvider::storeCappedValue(const float axis, int8_t *stroke_entry) {
  const int32_t unchecked = static_cast<int32_t>(roundf(axis * 128.0f));

  if (unchecked > 127) {
    *stroke_entry = 127;
  } else if (unchecked < -128) {
    *stroke_entry = -128;
  } else {
    *stroke_entry = unchecked;
  }
}
