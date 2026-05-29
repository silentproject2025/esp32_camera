import sys

file_path = 'camera_test/camera_test.ino'
with open(file_path, 'r') as f:
    content = f.read()

# 1. Add workerTask before setup()
worker_task_code = """void workerTask(void* param) {
  const TickType_t mpuInterval = pdMS_TO_TICKS(MPU_READ_MS);
  const TickType_t neoInterval = pdMS_TO_TICKS(30);
  TickType_t lastMpu = 0, lastNeo = 0;

  for (;;) {
    TickType_t now = xTaskGetTickCount();

    // MPU tick
    if (now - lastMpu >= mpuInterval) {
      if (xSemaphoreTake(mpuMutex, 0) == pdTRUE) {
        mpuTick();
        xSemaphoreGive(mpuMutex);
      }
      lastMpu = now;
    }

    // Neo tick
    if (now - lastNeo >= neoInterval) {
      neoTick();
      lastNeo = now;
    }

    // SD write from queue (non-blocking check)
    RecFrame rf;
    if (recFrameQueue &&
        xQueueReceive(recFrameQueue, &rf, 0) == pdTRUE) {
      if (recFile && rf.jpg && rf.len > 0) {
        fwrite(rf.jpg, 1, rf.len, recFile);
        recFrameCount++;
      }
      if (rf.jpg) free(rf.jpg);
    }

    vTaskDelay(pdMS_TO_TICKS(5));
    esp_task_wdt_reset();
  }
}
"""

if 'void workerTask(void* param)' not in content:
    content = content.replace('void setup(){', worker_task_code + '\nvoid setup(){')

# 2. Add mutexes and task creation to setup()
setup_init_code = """  mpuMutex = xSemaphoreCreateMutex();
  neoMutex = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(
    workerTask,       // task function
    "worker",         // name
    8192,             // stack size
    nullptr,          // param
    1,                // priority (low)
    &workerTaskHandle,
    1                 // Core 1
  );
  resetAllButtons();"""

# Find the end of setup()
if 'resetAllButtons();\n}' in content:
    content = content.replace('resetAllButtons();\n}', setup_init_code + '\n}')

with open(file_path, 'w') as f:
    f.write(content)
