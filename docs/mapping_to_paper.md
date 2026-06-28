# Mapping: repository artifacts ⇄ manuscript

For each claim in the paper, this table points to the exact artifact that
supports it. Manuscript JISAS-D-26-02544.

## ProVerif (Section 5.1, Formal Security Analysis)

| Paper item | Artifact | Notes |
|------------|----------|-------|
| Full 3-phase model | `b_puf_a2k.pv` | Registration in `main`, Phase 2/3 in the processes |
| Q1 (SK secrecy) | `proverif/queries/q1_sk_secrecy.txt` + `proverif/logs/` | `RESULT not attacker(SK_secret_test[]) is true` |
| Q2 (auth, operator) | `proverif/queries/q2_auth_corr_user.txt` + log | injective correspondence |
| Q3 (auth, UAV) | `proverif/queries/q3_auth_corr_uav.txt` + log | injective correspondence |
| Q4 (handover integrity) | `proverif/queries/q4_handover_integrity.txt` + log | AEAD ticket; forgery rejected |
| Target-UAV binding (R1-1) | `HVU(...PID_j...)` in `UserProcess`; `=PIDj` pattern in `GSSProcess1` | M1 carries PID_j |
| Public nonces (R1-3) | `new` + `out(c_pub,...)` for NU, N1, N2 | only Si, PUF, K12 are private |
| AEAD ticket (R1-4) | `aenc/adec_auth` under `K12`, 8-field bind | sent on public `c_gss` |

## ESP32-S3 measurements (Section 6)

| Paper item | Artifact | Notes |
|------------|----------|-------|
| Table 4, `T_h` (SHA-256) | `src/primitives.c::prim_sha256_64` | μ, σ over N=1000 |
| Table 4, `T_se` (AES-128-GCM) | `src/primitives.c::prim_aes128gcm_64` | μ, σ over N=1000 |
| Table 4, `T_f` (Fuzzy Extractor) | `src/primitives.c::prim_fuzzy_extract` | repetition decode + SHA-256 |
| Table 4, `T_p` (PUF envelope) | `src/primitives.c::prim_puf_envelope` | response hashing + nonces |
| Warm-up / N | `src/main.c` | 100-iter warm-up, then N=1000 |
| Tables 6 & 8 normalisation | `results/raw_timings.csv` | recompute μ, σ; normalise comparators |
| Deterministic inputs | `src/test_vectors.h` | fixed test vectors (no real keys/biometrics) |

## Not included (and why)
- Raw biometric data and physical PUF readout traces (privacy / hardware-bound).
