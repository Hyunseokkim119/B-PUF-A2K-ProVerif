/* ======================================================================
 *  B-PUF A2K : primitive implementations (mbed TLS v3.4 on ESP32-S3)
 *  Manuscript JISAS-D-26-02544.  Anonymised for peer review.
 *
 *  Each prim_* function performs EXACTLY one invocation of the primitive
 *  being timed, using deterministic inputs from test_vectors.h so the
 *  measurement is reproducible.  Buffers and key schedules are set up
 *  once in prim_init() to keep the timed region to the primitive itself.
 * ====================================================================== */

#include <string.h>
#include "primitives.h"
#include "test_vectors.h"

#include "mbedtls/sha256.h"
#include "mbedtls/gcm.h"
#include "mbedtls/aes.h"

/* ---------- shared state, initialised once ---------- */
static mbedtls_gcm_context  g_gcm;
static uint8_t              g_out[128];
static uint8_t              g_tag[16];

/* sink to defeat dead-code elimination of the timed work */
volatile uint8_t g_sink;

void prim_init(void) {
    mbedtls_gcm_init(&g_gcm);
    /* AES-128 key schedule done once; the timed region is the GCM op only */
    mbedtls_gcm_setkey(&g_gcm, MBEDTLS_CIPHER_ID_AES, TV_AES_KEY, 128);
}

/* ----------------------------------------------------------------------
 *  T_h : SHA-256 over a 64-byte input
 * -------------------------------------------------------------------- */
void prim_sha256_64(void) {
    uint8_t digest[32];
    mbedtls_sha256(TV_MSG64, 64, digest, /*is224=*/0);
    g_sink ^= digest[0];
}

/* ----------------------------------------------------------------------
 *  T_se : AES-128-GCM over a 64-byte plaintext (encrypt + tag)
 * -------------------------------------------------------------------- */
void prim_aes128gcm_64(void) {
    mbedtls_gcm_crypt_and_tag(&g_gcm, MBEDTLS_GCM_ENCRYPT,
                              64,
                              TV_GCM_IV, 12,
                              NULL, 0,            /* no AAD */
                              TV_MSG64, g_out,
                              16, g_tag);
    g_sink ^= g_out[0] ^ g_tag[0];
}

/* ----------------------------------------------------------------------
 *  T_f : Fuzzy Extractor reproduction
 *        Secure-Sketch recovery over a 128-bit biometric source with
 *        BCH-style repetition decoding, followed by SHA-256 extraction.
 *
 *  The repetition code (rate 1/R) corrects up to floor(R/2) flips per
 *  block by majority vote; this is the dominant cost on the device and is
 *  implemented in software (no hardware acceleration), matching Table 4's
 *  note. Inputs are deterministic (a fixed noisy reading w' and helper
 *  data); the recovered key is hashed with SHA-256 to yield K_bio.
 * -------------------------------------------------------------------- */
#define FE_BLOCKS  128          /* 128-bit codeword space        */
#define FE_R       7            /* repetition factor (odd)       */

void prim_fuzzy_extract(void) {
    uint8_t recovered[FE_BLOCKS / 8];
    memset(recovered, 0, sizeof(recovered));

    /* repetition decoding by majority vote over R noisy replicas */
    for (int b = 0; b < FE_BLOCKS; b++) {
        int ones = 0;
        for (int r = 0; r < FE_R; r++) {
            /* xor noisy reading with helper data, both deterministic */
            uint8_t byte = TV_BIO_NOISY[(b * FE_R + r) % TV_BIO_LEN]
                         ^ TV_BIO_HELPER[(b * FE_R + r) % TV_BIO_LEN];
            ones += (byte & 0x1);
        }
        if (ones * 2 > FE_R) recovered[b >> 3] |= (uint8_t)(1u << (b & 7));
    }

    /* SHA-256 extraction -> K_bio */
    uint8_t kbio[32];
    mbedtls_sha256(recovered, sizeof(recovered), kbio, 0);
    g_sink ^= kbio[0];
}

/* ----------------------------------------------------------------------
 *  T_p : controlled-PUF envelope
 *        Covers the cryptographic operations that SURROUND a hardware PUF
 *        readout: the response is combined with two nonces and a
 *        timestamp and hashed (V_1 = h(R_j | N_1 | N_2 | T_3)). A real
 *        ring-oscillator PUF readout (~30-80 us) is added on top on the
 *        target board; here we model the readout with a deterministic
 *        response so the timed region is the obfuscation + hashing.
 * -------------------------------------------------------------------- */
void prim_puf_envelope(void) {
    uint8_t buf[32 + 8 + 8 + 8];   /* R_j(32) | N1(8) | N2(8) | T3(8) */
    memcpy(buf,           TV_PUF_RESPONSE, 32);
    memcpy(buf + 32,      TV_N1, 8);
    memcpy(buf + 40,      TV_N2, 8);
    memcpy(buf + 48,      TV_T3, 8);

    uint8_t v1[32];
    mbedtls_sha256(buf, sizeof(buf), v1, 0);
    g_sink ^= v1[0];
}
