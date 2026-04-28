# ADR-0017: Reproducibility infrastructure — Dockerfile + flake.nix + Zenodo DOI + papers/

**Status:** Accepted
**Date:** 2026-04-26
**Supersedes:** —

---

## Context

ACM artifact evaluation rewards three badges: **Available**, **Reusable**,
and **Reproduced**. The modal OOPSLA1 outcome is the triple-badge for
artifacts that include a containerized build, a citable DOI, and a fixed
configuration that reviewers can rebuild and re-run end-to-end.

QFault must commit to this infrastructure **before Stage 5c** so that the
QCE26 submission inherits it and CGO27 / OOPSLA submissions follow with
zero friction.

---

## Decision

QFault ships, as **Stage 2.5 deliverables**:

(a) **`Dockerfile`** based on `ubuntu:24.04` with multi-stage build for
    reviewer-friendly size. Pinned versions:
    - gcc-13, clang-18
    - cmake 3.30 (via pip in builder stage)
    - MPFR 4.2, GMP 6.3 (Stage 6 prerequisite — can be deferred but
      including now reduces churn)
    - Stim v1.15.0 (FetchContent-pinned in CMakeLists.txt)
    - MQT QCEC v3.5.0 (FetchContent-pinned)
    - GoogleTest v1.14.0 (already pinned)
    - Haskell `gridsynth` binary built from a pinned newsynth commit.
    - Pin base image **by digest, not tag** (e.g.
      `ubuntu:24.04@sha256:<hash>`).

(b) **`flake.nix`** for Nix-preferring reviewers and bit-exact rebuilds.
    Nixpkgs pinned to a specific commit hash; `flake.lock` committed.

(c) **Zenodo DOI** per release tag, minted via the GitHub-Zenodo
    integration. The `CITATION.cff` file at repo root carries the DOI.

(d) **`papers/<venue>/` directory** per submission with:
    - `paper.tex` and venue-specific style.
    - `figures/` produced by `make figures` from `bench/`.
    - `inputs.lock` with sha256 hashes of all bench input circuits.
    - `README.md` describing the reviewer's reproduction workflow.
    - Submission frozen at the moment the Zenodo DOI is minted; no edits
      thereafter.

---

## Alternatives Considered

| Alternative | Why Rejected |
|---|---|
| Dockerfile only | Misses Nix-preferring reviewers (overrepresented in OOPSLA/PLDI) |
| Nix only | Most reviewers will reach for Docker first |
| GitHub release tarballs only | No DOI; not citable |
| No `papers/` directory | Cannot freeze configuration per submission |
| Singularity / Apptainer | Smaller user base; Docker is the default reviewer workflow |
| Spack | Quantum HPC niche; not the QCE26/CGO27 reviewer baseline |

---

## Consequences

**Positive:**
- Triple-badge target is realistic for CGO27 and onwards.
- Reviewers reach for whichever toolchain they already have (Docker
  default, Nix as alternative).
- `papers/<venue>/` isolation means a CGO27 reviewer reproducing the
  QCE26 paper does not get accidentally upgraded numbers — each
  submission is its own Zenodo-minted snapshot.

**Negative / Trade-offs:**
- Two parallel build paths to maintain; mitigated by treating Nix as
  the source of truth for pinned versions and Docker as a convenience
  wrapper that reads the same version manifest
  (`cmake/dependency_versions.cmake`).

**Risks:**
- Docker base image churn; mitigated by **pinning by digest, not tag**.
- nixpkgs commit pinning means security-patch latency; acceptable for
  research artifact reproducibility.
- Stim's openly-unstable C++ ABI (per ADR-0009) interacts with this:
  any Stim tag bump invalidates older Docker/Nix builds. Document the
  tag-bump cycle every 6–9 months in the v0.x roadmap.

---

## Implementation Notes for AI Sessions

When loading the memory bank:
- `Dockerfile` and `flake.nix` are **co-equal**.
- Zenodo DOI is part of the release process (`/phase-exit` for Stage 5c
  triggers DOI minting).
- `papers/<venue>/` is frozen at submission.
- All version pins live in **`cmake/dependency_versions.cmake`** — a
  single file consumed by `CMakeLists.txt`, the Dockerfile (via build
  args), and `flake.nix` (via Nix overrides). When bumping a version,
  edit one file.
- `make figures-tier{1,2,3}` produces per-tier plots. `make figures`
  runs all three.

---

## References

- ACM Artifact Review and Badging guidelines (current version)
- Zenodo DOI workflow with GitHub:
  https://docs.github.com/en/repositories/archiving-a-github-repository/referencing-and-citing-content
- Nix flakes documentation
- `Dockerfile` (repo root, Stage 2.5 deliverable)
- `flake.nix` (repo root, Stage 2.5 deliverable)
- `cmake/dependency_versions.cmake` (single source of truth)
- ADR-0009 (verification strategy — Stim/QCEC pinning)
- ADR-0016 (conference ladder — uses `papers/<venue>/`)
