import sys

with open('camera_test/camera_test.ino', 'r') as f:
    content = f.read()

# 1. Replace tilt estimation
old_tilt = """  g_tiltX = atan2f(ay, az) * RAD_TO_DEG;
  g_tiltY = atan2f(-ax, sqrtf(ay * ay + az * az)) * RAD_TO_DEG;"""

new_tilt = """  float rawTiltX = atan2f(ay, az) * RAD_TO_DEG;
  float rawTiltY = atan2f(-ax, sqrtf(ay * ay + az * az)) * RAD_TO_DEG;

  float dt = (kalmanLastMs == 0) ? 0.08f : (now - kalmanLastMs) / 1000.0f;
  kalmanLastMs = now;
  dt = constrain(dt, 0.01f, 0.2f);

  g_tiltX = kalmanX.update(rawTiltX, g_gyroX * RAD_TO_DEG, dt);
  g_tiltY = kalmanY.update(rawTiltY, g_gyroY * RAD_TO_DEG, dt);"""

content = content.replace(old_tilt, new_tilt)

# 2. Replace shake detection
old_shake = """  float mag = sqrtf(g_accX * g_accX + g_accY * g_accY + g_accZ * g_accZ);
  static float magFiltered = 9.8f;
  magFiltered = 0.85f * magFiltered + 0.15f * mag;
  g_shake = (fabsf(magFiltered - 9.8f) > 4.5f);"""

new_shake = """  float mag = sqrtf(ax * ax + ay * ay + az * az);
  static float magFiltered = 9.8f;
  static int shakeCount = 0;
  magFiltered = magFiltered * 0.92f + mag * 0.08f;
  bool rawShake = (fabsf(magFiltered - 9.8f) > 3.0f);
  if (rawShake) shakeCount = min(shakeCount + 1, 5);
  else          shakeCount = max(shakeCount - 1, 0);
  g_shake = (shakeCount >= 3);"""

content = content.replace(old_shake, new_shake)

with open('camera_test/camera_test.ino', 'w') as f:
    f.write(content)
