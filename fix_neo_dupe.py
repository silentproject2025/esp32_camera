import sys

file_path = 'camera_test/camera_test.ino'
with open(file_path, 'r') as f:
    content = f.read()

# Find the start of setup()
setup_start = content.find('void setup(){')
if setup_start != -1:
    before_setup = content[:setup_start]
    after_setup = content[setup_start:]

    # Remove the duplicate neoTick implementation from the end of before_setup
    # The duplicate starts with "    else { strip.setPixelColor(0, 60, 0, 0); }" roughly.

    bad_fragment = """    else { strip.setPixelColor(0, 60, 0, 0); }
    strip.show(); return;
  }
  if (g_neoMode == NEO_MODE_OFF || g_neoMode == NEO_MODE_SOLID) return;
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
}"""
    before_setup = before_setup.replace(bad_fragment, "")

    with open(file_path, 'w') as f:
        f.write(before_setup + after_setup)
