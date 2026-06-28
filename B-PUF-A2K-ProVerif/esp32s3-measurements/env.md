# Measurement environment

| Item | Value |
|------|-------|
| Board | ESP32-S3 (dual-core Xtensa LX7) |
| CPU frequency | 240 MHz (`CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ=240`) |
| SRAM | 512 KB |
| Crypto library | mbed TLS v3.4 (bundled with ESP-IDF v5.1) |
| Toolchain | ESP-IDF v5.1 (or PlatformIO `espressif32` with `framework=espidf`) |
| Instruction cache | enabled |
| Task watchdog | disabled during the measurement window (`esp_task_wdt_deinit`) |
| Timer source | Xtensa CCOUNT cycle counter (`esp_cpu_get_cycle_count`) |
| Warm-up | 100 iterations per primitive |
| Timed iterations | N = 1000 per primitive |
| Serial monitor | 115200 baud |

## Reproduction steps

```bash
# Option 1 — PlatformIO
pio run -t upload
pio device monitor -b 115200 | tee serial_capture.txt

# Option 2 — ESP-IDF
idf.py set-target esp32s3
idf.py flash monitor | tee serial_capture.txt
```

Then extract the CSV rows (lines beginning with `CSV,`) into
`results/raw_timings.csv`, and read the four `SUMMARY,...` lines for the
mean and standard deviation of T_h, T_se, T_f, T_p. Transfer those values
into Table 4 of the manuscript.

> Do not hand-edit the timings: they must come from a real run on the
> board. The `SUMMARY` lines and the per-iteration CSV must be consistent.

## sdkconfig knobs that matter

- `CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ=240`
- `CONFIG_COMPILER_OPTIMIZATION_PERF=y` (or `-O2`) for representative timings
- `CONFIG_MBEDTLS_HARDWARE_AES` — leave at its default; report whichever
  setting you used, since hardware AES changes T_se substantially.
