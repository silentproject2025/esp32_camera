import sys

with open('camera_test/camera_test.ino', 'r') as f:
    lines = f.readlines()

new_lines = []
struct_added = False

kalman_struct = """struct KalmanAngle {
  float angle   = 0.0f;
  float bias    = 0.0f;
  float P[2][2] = {{1,0},{0,1}};
  float Q_angle = 0.001f;
  float Q_bias  = 0.003f;
  float R_meas  = 0.03f;

  float update(float newAngle, float newRate, float dt) {
    float rate = newRate - bias;
    angle += dt * rate;
    P[0][0] += dt * (dt * P[1][1] - P[0][1] - P[1][0] + Q_angle);
    P[0][1] -= dt * P[1][1];
    P[1][0] -= dt * P[1][1];
    P[1][1] += Q_bias * dt;
    float S = P[0][0] + R_meas;
    float K[2] = { P[0][0] / S, P[1][0] / S };
    float y = newAngle - angle;
    angle += K[0] * y;
    bias  += K[1] * y;
    P[0][0] -= K[0] * P[0][0];
    P[0][1] -= K[0] * P[0][1];
    P[1][0] -= K[1] * P[0][0];
    P[1][1] -= K[1] * P[0][1];
    return angle;
  }
};

static KalmanAngle kalmanX, kalmanY;
static uint32_t kalmanLastMs = 0;

"""

for line in lines:
    if "void mpuTick() {" in line and not struct_added:
        new_lines.append(kalman_struct)
        struct_added = True
    new_lines.append(line)

with open('camera_test/camera_test.ino', 'w') as f:
    f.writelines(new_lines)
