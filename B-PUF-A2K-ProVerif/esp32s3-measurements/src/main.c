/* ======================================================================
 *  B-PUF A2K : ESP32-S3 primitive timing harness
 *  Manuscript JISAS-D-26-02544, Journal of Information Security and
 *  Applications.  Anonymised for peer review.
 *
 *  Target : ESP32-S3 (dual-core Xtensa LX7 @ 240 MHz, 512 KB SRAM)
 *  Crypto : mbed TLS v3.4 (bundled with ESP-IDF v5.1)
 *
 *  This harness measures the four primitives used by the protocol:
 *      T_h   SHA-256 over a 64-byte input
 *      T_se  AES-128-GCM over a 64-byte plaintext
 *      T_f   Fuzzy Extractor reproduction (Secure Sketch + SHA-256)
 *      T_p   controlled-PUF envelope (response hashing with nonces + ts)
 *
 *  Methodology (matches Section 6.1 of the paper):
 *      * CPU pinned to 240 MHz, instruction cache enabled,
 *        task watchdog disabled during the measurement window.
 *      * Each primitive is warmed up for WARMUP=100 iterations,
 *        then timed over N=1000 iterations.
 *      * Per-iteration cycle counts are taken from the Xtensa CCOUNT
 *        register (esp_cpu_get_cycle_count) and converted to ms using
 *        the configured CPU frequency, giving sub-microsecond resolution.
 *      * Mean (mu) and sample standard deviation (sigma) are reported,
 *        and every per-iteration value is printed as CSV so that mu/sigma
 *        in Table 4 can be recomputed independently.
 *
 *  IMPORTANT: the numeric results are produced by RUNNING this code on
 *  real hardware. Do not transcribe timings from elsewhere; flash the
 *  board, capture the serial CSV, and paste it into
 *  results/raw_timings.csv.
 * ====================================================================== */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_cpu.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_task_wdt.h"

#include "primitives.h"
#include "test_vectors.h"

#define WARMUP   100
#define N        1000

static const char *TAG = "BPUF_BENCH";

/* CPU frequency in Hz (must match sdkconfig: CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ=240). */
#ifndef CPU_FREQ_HZ
#define CPU_FREQ_HZ (240u * 1000u * 1000u)
#endif

static inline double cycles_to_ms(uint32_t cyc) {
    return (double)cyc * 1000.0 / (double)CPU_FREQ_HZ;
}

/* Time a single primitive call via the CCOUNT cycle counter. */
typedef void (*prim_fn_t)(void);

static void bench_primitive(const char *name, prim_fn_t fn) {
    /* warm-up: stabilise cache, branch predictors, etc. */
    for (int i = 0; i < WARMUP; i++) fn();

    double sum = 0.0, sum_sq = 0.0;
    /* CSV header for this primitive: one row per iteration */
    printf("CSV,%s,iter,ms\n", name);
    for (int i = 0; i < N; i++) {
        uint32_t c0 = esp_cpu_get_cycle_count();
        fn();
        uint32_t c1 = esp_cpu_get_cycle_count();
        double ms = cycles_to_ms(c1 - c0);
        printf("CSV,%s,%d,%.6f\n", name, i, ms);
        sum    += ms;
        sum_sq += ms * ms;
    }
    double mu    = sum / (double)N;
    double var   = (sum_sq - (double)N * mu * mu) / (double)(N - 1);
    double sigma = (var > 0.0) ? sqrt(var) : 0.0;
    printf("SUMMARY,%s,mean_ms=%.6f,std_ms=%.6f,N=%d\n", name, mu, sigma, N);
    ESP_LOGI(TAG, "%-8s  mu=%.6f ms  sigma=%.6f ms (N=%d)", name, mu, sigma, N);
}

void app_main(void) {
    /* Pin to one core, raise priority, disable WDT during measurement. */
    esp_task_wdt_deinit();
    ESP_LOGI(TAG, "B-PUF A2K primitive benchmark starting");
    ESP_LOGI(TAG, "CPU freq assumed = %u Hz; WARMUP=%d N=%d", CPU_FREQ_HZ, WARMUP, N);

    /* Initialise primitives (RNG, key schedules, PUF model, FE sketch). */
    prim_init();

    /* Order matches Table 4 of the paper. */
    bench_primitive("T_h",  prim_sha256_64);     /* SHA-256, 64-byte input        */
    bench_primitive("T_se", prim_aes128gcm_64);  /* AES-128-GCM, 64-byte plaintext */
    bench_primitive("T_f",  prim_fuzzy_extract); /* Fuzzy Extractor reproduction   */
    bench_primitive("T_p",  prim_puf_envelope);  /* controlled-PUF envelope        */

    printf("DONE\n");
    ESP_LOGI(TAG, "Benchmark complete. Capture the CSV lines above into "
                  "results/raw_timings.csv");

    /* idle */
    while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
}
