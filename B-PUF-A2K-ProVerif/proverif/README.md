# ProVerif models — B-PUF A2K

Manuscript JISAS-D-26-02544. Anonymised for peer review.

## Requirement
ProVerif >= 2.04 (tested with the applied-pi front end).
Install from https://bblanche.gitlabpages.inria.fr/proverif/ or via opam:
`opam install proverif`.

## Files
| File | Phase | Queries |
|------|-------|---------|
| `phase1_registration.pv` | Registration | secrecy of Si, K12, idU |
| `phase2_authentication.pv` | Authentication & Key Agreement | Q1 (SK secrecy), Q2, Q3 |
| `phase3_handover.pv` | Session-Preserving Handover | Q4, migrated-SK secrecy |
| `bpuf_a2k_full.pv` | all three combined | Q1–Q4 |
| `queries/` | textual description of Q1–Q4 | — |
| `logs/` | console output to be captured from your run | — |

## Run
```bash
proverif phase1_registration.pv   | tee logs/phase1_output.txt
proverif phase2_authentication.pv | tee logs/phase2_output.txt
proverif phase3_handover.pv       | tee logs/phase3_output.txt
proverif bpuf_a2k_full.pv         | tee logs/full_output.txt
```

## Modelling conventions
- All nonces are **public** (sent on the public channel), so the
  Dolev–Yao attacker observes them. Their role is freshness, not secrecy.
- Only `Si` (operator secret), the PUF handle `idU`, and `K12` (inter-GSS
  key) are **private**. This matches the RoR proof: session-key secrecy
  rests on `Si` and `Rj`, never on the nonces.
- The handover ticket uses an authenticated-encryption constructor
  (`adec` opens only ciphertexts produced under `K12`), modelling
  AES-GCM integrity: a forged or modified ticket is rejected.

ProVerif may print *warnings* about unused bound variables (e.g. received
timestamps that the abstract model does not re-check); these are benign
and do not affect the verification results.
