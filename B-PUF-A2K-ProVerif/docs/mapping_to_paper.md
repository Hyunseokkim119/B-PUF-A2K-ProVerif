# Mapping: repository artifacts ⇄ manuscript

This file lets a reviewer locate, for each claim in the paper, the exact artifact
that supports it. It directly answers the reviewers' verifiability concerns.

## ProVerif (Section 5.1, Formal Security Analysis)

| Paper item | Artifact | Notes |
|------------|----------|-------|
| §5.1 Phase-1 model | `proverif/phase1_registration.pv` | offline registration, CRP/biometric enrolment |
| §5.1 Phase-2 model | `proverif/phase2_authentication.pv` | 3-party AKE, target-UAV binding in `V_U` |
| §5.1 Phase-3 model | `proverif/phase3_handover.pv` | AEAD ticket migration under `K_12` |
| §5.1 Q1 (SK secrecy) | `proverif/queries/q1_sk_secrecy.txt` + `logs/phase2_output.txt` | `RESULT secrecy of SK is true` |
| §5.1 Q2 (auth, operator) | `proverif/queries/q2_auth_corr_user.txt` + log | injective correspondence |
| §5.1 Q3 (auth, UAV) | `proverif/queries/q3_auth_corr_uav.txt` + log | injective correspondence |
| §5.1 Q4 (handover integrity) | `proverif/queries/q4_handover_integrity.txt` + `logs/phase3_output.txt` | AEAD forgery rejected |
| §5.1 "nonces are public" claim | `free N_U, N_1, N_2 : ...` declarations in `phase2_authentication.pv` | only `S_i, R_j, K_12` are `[private]` |

## ESP32-S3 measurements (Section 6)

| Paper item | Artifact | Notes |
|------------|----------|-------|
| Table 4, `T_h` (SHA-256) | `src/primitives.c::measure_hash()` | μ, σ over N=1000 |
| Table 4, `T_se` (AES-128-GCM) | `src/primitives.c::measure_aead()` | μ, σ over N=1000 |
| Table 4, `T_f` (Fuzzy Extractor) | `src/primitives.c::measure_fe()` | Secure Sketch + SHA-256 |
| Table 4, `T_p` (PUF envelope) | `src/primitives.c::measure_puf_envelope()` | response hashing + nonces |
| Table 4 warm-up / N | `src/main.c` | 100-iteration warm-up, then N=1000 |
| Tables 6 & 8 normalisation | `results/raw_timings.csv` | recompute μ, σ; normalise comparison schemes |
| §6 energy analysis | derived from Table 4 timings × 250 mW | no separate artifact; computation shown in paper |

## What is NOT in the repository (and why)

- No raw biometric data and no physical PUF readout traces (privacy / hardware-bound;
  the PUF envelope timing covers the surrounding crypto, as stated in Table 4's note).
- No author/institution identifiers during the review period (single-anonymized review;
  identifiers restored at camera-ready).
