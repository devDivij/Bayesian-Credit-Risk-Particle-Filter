# Bayesian Credit Risk Particle Filter

**A real-time Bayesian Particle Filter for Credit Risk Modeling using a static Sobol scenario grid.**

This repository implements a computationally efficient Bayesian credit-risk model that updates particle weights monthly based on observed defaults. It delivers portfolio-level risk metrics (VaR, Expected Shortfall, Economic Capital, regime probabilities) without re-sampling the macroeconomic scenario grid.

## 📖 Documentation (Read First)

The complete technical details and methodology are in the `docs/` directory:

- **`docs/Technical Write-up.pdf`** – Technical Implementation (particle filter, PD logistic regression, loss simulation, Bayesian weight updates, advantages, etc.)
- **`docs/Case Study.pdf`** – Full end-to-end example with 500-debtor synthetic portfolio, regime shifts after 2 and 7 defaults, and visual outputs.

**Start here** — the PDFs explain every formula, data flow, and output you will see when running the code.

## Repository Structure

```
Bayesian-Credit-Risk-Particle-Filter/
├── CMakeLists.txt              # CMake build configuration (C++ core)
├── LICENSE
├── README.md
├── .gitignore
│
├── data/
│   ├── debtors/
│   │   ├── debtor_data.txt     # ← Portfolio: 500 debtors (id, feature_1–3, EAD, exposure_class)
│   │   └── defaults.txt        # ← Observed defaults (one line per period; already has placeholder IDs 111 and 112)
│   └── particle_grid/
│       └── new-joe-kuo-6.21201 # Sobol low-discrepancy sequence for the 985 particles
│
├── docs/
│   ├── Bayesian Credit Risk Particle Filter.pdf     # Technical Implementation
│   └── Case Study.pdf                               # Full example with outputs
│
├── include/
│   ├── model_config.hpp        # ← Edit here: all constants, coefficients, sensitivities etc.
│   └── (other header files for c++ code)
│
├── scripts/
│   ├── risk_analysis.py        # Auto-runs after build: prints tables + saves plots
│   └── truncate.py             # Deletes everything in saved_state/ to start fresh
│
├── src/
│   ├── main.cpp                # Main executable – loads data, does Bayesian updates, saves state
│   └── (other c++ logic files)
│
├── saved_state/                # ← Auto-created on first run (weights, PD matrices, CSV metrics, plots)
│   └── (all output files)
│
└── build/                      # (created by you during cmake)
```

## Prerequisites

- **C++17** compiler (g++ / clang++)
- **CMake** ≥ 3.10
- **Python 3.8+** with `matplotlib`, `pandas`, `numpy` (for `risk_analysis.py` and plots)

## Build & Run (CMake Project)

```bash
# 1. Clone
git clone https://github.com/devDivij/Bayesian-Credit-Risk-Particle-Filter.git
cd Bayesian-Credit-Risk-Particle-Filter

# 2. Build
mkdir -p build && cd build
cmake ..
make -j$(nproc)

# 3. Run the model (from build/)
./credit_risk_filter
```
OR run from VS Code CMake extensions

**What happens on first run**
- Loads 985 Sobol particles + 500 debtors
- Equal initial weights
- Computes raw (unadjusted) loss distributions
- Saves everything to `../saved_state/`

**What happens on subsequent runs**
- Detects existing `saved_state/`
- Reads new defaults from `data/defaults.txt`
- Performs Bayesian weight update (likelihood ratio)
- Re-aggregates risk metrics instantly (no re-simulation)

## Configuration – Where to Change Inputs

1. **Model parameters & coefficients**  
   `include/model_config.hpp`  
   (systematic factor sensitivities per exposure class, macro correlations, number of particles, etc.)

2. **Portfolio**  
   `data/debtors/debtor_data.txt`  
   Format (one debtor per line):  
   `id feature_1 feature_2 feature_3 EAD exposure_class`  
   (already populated with 500 synthetic debtors; replace freely)

3. **Observed defaults** (the key Bayesian input)  
   `data/defaults.txt`  
   - First run: delete the placeholder IDs (111 and 112) or leave empty for neutral start  
   - After each month: **append a new line** with the IDs that defaulted this month  
   - Example:
     ```
     111 112
     45 67 89          # ← new line for month 2
     23 154            # ← new line for month 3
     ```

   Re-run the executable after every edit → weights update automatically.

## Reset / Start Fresh

```bash
cd scripts
python truncate.py
```

This clears `saved_state/` so the next run starts with equal weights again.

## Outputs

All results are written to `saved_state/`:

- Console printout (regime weights, aggregated VaR/ES/EC, regime-wise tables)
- `risk_analysis.py` automatically generates:
  - Aggregated Loss Distribution plot
  - Regime-specific density plot

Just run the executable — the Python analysis runs as part of the workflow (or manually `python ../scripts/risk_analysis.py` if you want only plots).

## Example Workflow (matches Case Study)

```bash
# Intialize equal weights to each particle (Distributions generated are false and need Bayesian calibration)
cd build
./credit_risk_filter

# Month 1 – 2 defaults (change the ids of people who defaulted in defaults.txt or test with the placeholder ids: 111 112)
./credit_risk_filter
# → Now the distributions have been updated to reflect actual loss distribution functions of the debtor portfolio
# → Plots saved

# Month 2 – 7 defaults
# Append new line to data/debtors/defaults.txt
./credit_risk_filter
# → Crisis and Recession weights jump and expected losses increase
```

## Troubleshooting

- **"No saved_state found"** → normal on first run
- **Plots missing** → make sure `matplotlib` is installed and re-run `risk_analysis.py`
- **Changing portfolio/constants** → edit `model_config.hpp` + `debtor_data.txt` and rebuild, by deleting the build directory first using this command and building the project again as mentioned before. DO NOT delete the build directory manually.
```bash
rm -rf build
```

## License

Creative Commons Attribution-NonCommercial-NoDerivs 4.0 International (CC BY-NC-ND 4.0) – see `LICENSE`.