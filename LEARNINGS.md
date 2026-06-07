# Radio Mode Enhancements (v6.1+)

- Manual frequency tuning implemented for RDA5807M by modifying `radioFreq` global and calling `radio.setFrequency()`.
- Short press of BTN_C/BTN_D adjusts frequency by 0.1 MHz (represented as 10 units in `radioFreq` which is in 10kHz units, e.g., 10070 = 100.70 MHz).
- Long press of BTN_C/BTN_D triggers hardware seek using `radio.seekDown(true)` and `radio.seekUp(true)`.
- UI hint updated to "C/D: Tune (Hold:Seek) B:Vol (Hold:Exit)".
