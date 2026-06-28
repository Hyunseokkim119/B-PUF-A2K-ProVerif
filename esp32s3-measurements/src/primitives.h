/* Primitive interface for the B-PUF A2K ESP32-S3 benchmark.
 * Manuscript JISAS-D-26-02544 */
#ifndef BPUF_PRIMITIVES_H
#define BPUF_PRIMITIVES_H

#include <stdint.h>

void prim_init(void);            /* one-time setup (key schedules, contexts) */

void prim_sha256_64(void);       /* T_h  : SHA-256, 64-byte input            */
void prim_aes128gcm_64(void);    /* T_se : AES-128-GCM, 64-byte plaintext    */
void prim_fuzzy_extract(void);   /* T_f  : Fuzzy Extractor reproduction      */
void prim_puf_envelope(void);    /* T_p  : controlled-PUF envelope           */

extern volatile uint8_t g_sink;  /* prevents dead-code elimination           */

#endif /* BPUF_PRIMITIVES_H */
