import sys

def add_audio_output_bt():
    with open('camera_test/camera_test.ino', 'r') as f:
        content = f.read()

    class_logic = """
// ─────────────────────────────────────────────────────────────────────────────
//  AUDIO BRIDGE (ESP8266Audio -> ESP32-A2DP)
// ─────────────────────────────────────────────────────────────────────────────

#define BT_BUFFER_SIZE 4096
int16_t btBuffer[BT_BUFFER_SIZE * 2]; // Stereo
volatile int btWritePtr = 0;
volatile int btReadPtr = 0;

int32_t btMp3DataCallback(uint8_t *data, int32_t len) {
  if (len <= 0) return 0;
  int16_t *samples = (int16_t*)data;
  int count = len / 4; // 2 channels * 2 bytes
  int available = (btWritePtr - btReadPtr + (BT_BUFFER_SIZE * 2)) % (BT_BUFFER_SIZE * 2);
  int toRead = min(count, available / 2);

  for (int i = 0; i < toRead; i++) {
    samples[i*2]   = btBuffer[btReadPtr];
    btReadPtr = (btReadPtr + 1) % (BT_BUFFER_SIZE * 2);
    samples[i*2+1] = btBuffer[btReadPtr];
    btReadPtr = (btReadPtr + 1) % (BT_BUFFER_SIZE * 2);
  }
  return toRead * 4;
}

class AudioOutputBT : public AudioOutput {
  public:
    AudioOutputBT() { mono = false; }
    virtual bool begin() override { return true; }
    virtual bool ConsumeSample(int16_t sample[2]) override {
      int nextWrite = (btWritePtr + 2) % (BT_BUFFER_SIZE * 2);
      if (nextWrite == btReadPtr) return false; // Buffer full
      btBuffer[btWritePtr] = sample[0];
      btWritePtr = (btWritePtr + 1) % (BT_BUFFER_SIZE * 2);
      btBuffer[btWritePtr] = sample[1];
      btWritePtr = (btWritePtr + 1) % (BT_BUFFER_SIZE * 2);
      return true;
    }
    virtual bool stop() override { return true; }
};

AudioOutputBT *outBT = nullptr;
"""
    # Insert after globals
    insertion_point = content.find("static int btFileScroll = 0;")
    if insertion_point != -1:
        end_of_line = content.find("\n", insertion_point) + 1
        new_content = content[:end_of_line] + class_logic + content[end_of_line:]
        with open('camera_test/camera_test.ino', 'w') as f:
            f.write(new_content)

if __name__ == "__main__":
    add_audio_output_bt()
