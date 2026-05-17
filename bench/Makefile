# bench/Makefile — QFault Stage 2.5 benchmark orchestration.
#
# Targets:
#   make tier1        Run Tier-1 single-rotation synthesis benchmark
#   make tier2        Run Tier-2 algorithm-level end-to-end benchmark
#   make figures-tier1  Generate PDF plots from tier1.csv
#   make figures-tier2  Generate PDF plots from tier2.csv
#   make figures      Generate all PDF plots
#   make regression   Check tier1.csv against committed baseline (5% tolerance)
#   make clean        Remove generated CSVs and plots
#
# Variables (override on command line):
#   PRESET       CMake preset (default: gcc13-release)
#   N_ANGLES     Angles to sweep in Tier-1 (default: 10000)
#   EPS          Synthesis epsilon (default: 1e-10)

PRESET   ?= gcc13-release
N_ANGLES ?= 10000
EPS      ?= 1e-10

REPO_ROOT := $(shell cd .. && pwd)
RESULTS   := $(REPO_ROOT)/bench/results
PLOTS     := $(REPO_ROOT)/bench/plots

.PHONY: tier1 tier2 figures-tier1 figures-tier2 figures regression clean

tier1:
	@echo "=== Tier-1 synthesis benchmark ($(N_ANGLES) angles, eps=$(EPS)) ==="
	N_ANGLES=$(N_ANGLES) EPS=$(EPS) PRESET=$(PRESET) \
	    bash $(REPO_ROOT)/bench/tier1/run.sh

tier2:
	@echo "=== Tier-2 algorithm-level benchmark ==="
	PRESET=$(PRESET) bash $(REPO_ROOT)/bench/tier2/run.sh

figures-tier1: $(RESULTS)/tier1.csv
	@echo "=== Generating tier-1 plots ==="
	python3 $(REPO_ROOT)/bench/scripts/plot.py \
	    --results-dir $(RESULTS) --plots-dir $(PLOTS)

figures-tier2: $(RESULTS)/tier2.csv
	@echo "=== Generating tier-2 plots ==="
	python3 $(REPO_ROOT)/bench/scripts/plot.py \
	    --results-dir $(RESULTS) --plots-dir $(PLOTS)

figures: figures-tier1 figures-tier2

regression:
	@echo "=== Regression check vs Stage 2 baseline (tol=5%) ==="
	python3 $(REPO_ROOT)/bench/scripts/check_regression.py --tol 0.05

clean:
	rm -f $(RESULTS)/tier1.csv $(RESULTS)/tier2.csv
	rm -f $(PLOTS)/*.pdf
	@echo "Cleaned results/ and plots/"

$(RESULTS)/tier1.csv:
	$(MAKE) tier1

$(RESULTS)/tier2.csv:
	$(MAKE) tier2
