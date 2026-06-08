# Radio Mode Enhancements (v6.1+)

- Manual frequency tuning implemented for RDA5807M by modifying `radioFreq` global and calling `radio.setFrequency()`.
- Short press of BTN_C/BTN_D adjusts frequency by 0.1 MHz (represented as 10 units in `radioFreq` which is in 10kHz units, e.g., 10070 = 100.70 MHz).
- Long press of BTN_C/BTN_D triggers hardware seek using `radio.seekDown(true)` and `radio.seekUp(true)`.
- UI hint updated to "C/D: Tune (Hold:Seek) B:Vol (Hold:Exit)".

- Manual frequency input dialog for FM Radio (MODE_RADIO_FREQ_INPUT) allows users to set specific frequencies (50.00 - 115.00 MHz) using a 5-digit interface.
- Replaced auto-scan (long-press BOOT) with manual frequency input to give users precise control over the RDA5807M receiver.
- Frequency range 50-115 MHz enabled by setting the RDA5807M band to 3 (`radio.setBand(3)`).
