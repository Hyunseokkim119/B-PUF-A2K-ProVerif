# B-PUF A²K — ProVerif Model and ESP32-S3 Measurements

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![ProVerif 2.05+](https://img.shields.io/badge/ProVerif-2.05%2B-blue)](https://bblanche.gitlabpages.inria.fr/proverif/)

Artifacts accompanying the manuscript:

> **An Anonymous Authentication and Key Agreement Protocol with
> Session-Preserving Cross-Domain Handover Using PUF and Biometrics**
> (Manuscript JISAS-D-26-02544, *Journal of Information Security and
> Applications*).

Authors: Hyunseok Kim (Korea Military Academy) and Sungdo Kim (Changwon
National University).

This repository provides the two artifact sets referenced in the
manuscript's **Data Availability** statement:

1. The **ProVerif** applied-pi-calculus model of all three protocol
   phases (Registration, Authentication, Handover), the four verification
   queries, and the corresponding output logs.
2. The **ESP32-S3** measurement source code (timing harness and
   deterministic input vectors) used to obtain the primitive timings
   reported in Section 6 (Table 4).

---

## Repository layout

```
b_puf_a2k.pv                       Canonical ProVerif model (all 3 phases, Q1–Q4)
proverif/
  queries/                         Textual description of the four queries Q1–Q4
  logs/                            ProVerif console output (RESULT lines)
esp32s3-measurements/
  platformio.ini                   PlatformIO build (framework = espidf)
  CMakeLists.txt, main/            ESP-IDF build files (idf.py)
  src/
    main.c                         Timing harness (warm-up 100, N = 1000)
    primitives.c                   T_h, T_se, T_f, T_p via mbed TLS v3.4
    primitives.h, test_vectors.h   Interface + deterministic input vectors
  results/raw_timings.csv          Raw per-iteration timings (fill from a real run)
  env.md                           Exact toolchain / board configuration
docs/mapping_to_paper.md           Table/figure/query ⇄ artifact cross-reference
LICENSE                            MIT
CITATION.cff                       Citation metadata
```

## 1. Reproducing the ProVerif results

**Requirement:** ProVerif ≥ 2.05.

```bash
proverif -in pi b_puf_a2k.pv | tee proverif/logs/proverif_output.txt
```

The four queries and their expected outcomes:

| Query | Property | Expected |
|-------|----------|----------|
| Q1 | Secrecy of the session key `SK` | `RESULT not attacker(SK_secret_test[]) is true` |
| Q2 | Injective authentication (operator ⇄ GSS) | `... is true` |
| Q3 | Injective authentication (UAV ⇄ GSS) | `... is true` |
| Q4 | Handover-ticket integrity (AEAD) | `... is true` |

**Modelling notes (correspond to the reviewer revisions).**

- *Target-UAV binding (R1-1).* The operator verifier is
  `V_U = HVU(S_i' | PID_i | PID_j | N_U | T_1)` and the first message
  `M1 = {PID_i, PID_j, N_U, T_1, V_U}` explicitly carries and binds the
  target UAV pseudo-identity `PID_j`. The GSS input pattern
  `in(c_pub, (=PIDi, =PIDj, ...))` enforces this binding.
- *Public nonces (R1-3).* All nonces are generated with `new` and sent on
  the public channel, so the Dolev–Yao attacker observes them. Session-key
  secrecy rests on `S_i`, the PUF response, and `K12`, never on the nonces
  — consistent with the computational Real-or-Random proof in Section 5.1.
- *AEAD handover ticket (R1-4).* The ticket is built with an authenticated
  -encryption constructor (`aenc` / `adec_auth`) under `K12`, binding
  `PID_i | PID_j | ID_GSS1 | ID_GSS2 | SID | ver | Lh | Nt` with the
  session key as payload. It is transmitted on the **public** inter-GSS
  channel `c_gss`, so the attacker may attempt forgery/replay; query Q4
  shows that `GSS2` accepts only tickets genuinely issued by `GSS1`.
- *Fuzzy Extractor / PUF.* The biometric Fuzzy Extractor is captured by the
  equational reduction `FE_Rep(b, FE_Gen_HD(b)) = FE_Gen_K(b)`, and the PUF
  by a private function symbol (unclonability). `H1…H5` are independent
  random oracles.

ProVerif may emit *warnings* about unused bound variables (e.g. received
timestamps the abstract model does not re-check); these are benign.

## 2. Reproducing the ESP32-S3 measurements

**Hardware:** ESP32-S3 (dual-core Xtensa LX7 @ 240 MHz, 512 KB SRAM).
**Software:** mbed TLS v3.4 (bundled with ESP-IDF v5.1). See
`esp32s3-measurements/env.md`.

```bash
cd esp32s3-measurements
# PlatformIO
pio run -t upload && pio device monitor -b 115200 | tee serial_capture.txt
# or ESP-IDF
idf.py set-target esp32s3 && idf.py flash monitor | tee serial_capture.txt
```

The harness warms up each primitive for 100 iterations, then times
`N = 1000` iterations and prints mean (μ) and sample standard deviation (σ)
for `T_h`, `T_se`, `T_f`, `T_p`. Copy the `CSV,...` lines into
`results/raw_timings.csv` and transfer the four `SUMMARY,...` values into
Table 4 of the manuscript. The timings must come from a real run on the
board; they are not pre-filled in this repository.

## License

MIT — see [LICENSE](LICENSE).

## Citation

See [CITATION.cff](CITATION.cff).
