import sys

file_path = 'camera_test/camera_test.ino'
with open(file_path, 'r') as f:
    content = f.read()

# Refactor all neo functions to have _internal suffix and a thread-safe wrapper
neo_funcs = [
    ('neoSolid', 'uint8_t r, uint8_t g, uint8_t b', 'r, g, b'),
    ('neoOff', '', ''),
    ('neoBurst', 'uint8_t r, uint8_t g, uint8_t b, int times', 'r, g, b, times'),
    ('neoPulse', 'uint8_t r, uint8_t g, uint8_t b', 'r, g, b'),
    ('neoBreath', 'uint8_t r, uint8_t g, uint8_t b', 'r, g, b'),
    ('neoSpin', 'uint8_t r, uint8_t g, uint8_t b', 'r, g, b'),
    ('neoFade', 'uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2, uint32_t durationMs', 'r1, g1, b1, r2, g2, b2, durationMs'),
    ('neoRainbow', '', '')
]

for name, params, args in neo_funcs:
    # Check if already has _internal
    if f'void {name}_internal' not in content:
        content = content.replace(f'void {name}({params})', f'void {name}_internal({params})')

    # Create wrapper
    wrapper = f"""void {name}({params}) {{
  if (xSemaphoreTake(neoMutex, pdMS_TO_TICKS(200)) == pdTRUE) {{
    {name}_internal({args});
    xSemaphoreGive(neoMutex);
  }}
}}"""
    # Insert wrapper before _internal
    if f'void {name}({params}) {{' not in content:
        content = content.replace(f'void {name}_internal({params})', f'{wrapper}\nvoid {name}_internal({params})')

# Fix neoTick
neo_tick_fixed = """void neoTick() {
  if (xSemaphoreTake(neoMutex, pdMS_TO_TICKS(5)) != pdTRUE) return;
  uint32_t now = millis();
  if (recActive) {
    if ((now / 1000) % 2 == 0) { strip.setPixelColor(0, 180, 0, 0); }
    else { strip.setPixelColor(0, 60, 0, 0); }
    strip.show();
    xSemaphoreGive(neoMutex);
    return;
  }
  if (g_neoMode == NEO_MODE_OFF || g_neoMode == NEO_MODE_SOLID) {
    xSemaphoreGive(neoMutex);
    return;
  }
  if (g_neoMode == NEO_MODE_BREATH) {
    float v = (exp(sin(now/1500.0*PI)) - 0.36787944) * 0.42545906;
    strip.setPixelColor(0, g_neoR*v, g_neoG*v, g_neoB*v); strip.show();
  } else if (g_neoMode == NEO_MODE_PULSE) {
    float v = (sin(now/1000.0*PI) + 1.0) / 2.0;
    strip.setPixelColor(0, g_neoR*v, g_neoG*v, g_neoB*v); strip.show();
  } else if (g_neoMode == NEO_MODE_SPIN) {
    if (now - g_neoLastMs > 100) {
      g_neoLastMs = now; g_neoState = !g_neoState;
      if (g_neoState) strip.setPixelColor(0, g_neoR, g_neoG, g_neoB);
      else strip.setPixelColor(0, 0, 0, 0);
      strip.show();
    }
  }
  xSemaphoreGive(neoMutex);
}"""

# Use regex to replace neoTick entirely
import re
content = re.sub(r'void neoTick\(\) \{(?:.|\n)*?\}', neo_tick_fixed, content)

with open(file_path, 'w') as f:
    f.write(content)
