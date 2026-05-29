import sys

file_path = 'camera_test/camera_test.ino'
with open(file_path, 'r') as f:
    content = f.read()

# Fix signatures where I forced (uint8_t r, uint8_t g, uint8_t b)
content = content.replace(
    'void neoOff (uint8_t r, uint8_t g, uint8_t b) {',
    'void neoOff () {'
)
content = content.replace(
    'neoOff_internal(r, g, b);',
    'neoOff_internal();'
)

content = content.replace(
    'void neoRainbow (uint8_t r, uint8_t g, uint8_t b) {',
    'void neoRainbow () {'
)
content = content.replace(
    'neoRainbow_internal(r, g, b);',
    'neoRainbow_internal();'
)

content = content.replace(
    'void neoBurst (uint8_t r, uint8_t g, uint8_t b) {',
    'void neoBurst (uint8_t r, uint8_t g, uint8_t b, int times) {'
)
content = content.replace(
    'neoBurst_internal(r, g, b);',
    'neoBurst_internal(r, g, b, times);'
)

content = content.replace(
    'void neoFade (uint8_t r, uint8_t g, uint8_t b) {',
    'void neoFade (uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2, uint32_t durationMs) {'
)
content = content.replace(
    'neoFade_internal(r, g, b);',
    'neoFade_internal(r1, g1, b1, r2, g2, b2, durationMs);'
)

with open(file_path, 'w') as f:
    f.write(content)
