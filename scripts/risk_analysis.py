import numpy as np
import matplotlib.pyplot as plt

def load_ecdfs(ecdf_file):
    return np.loadtxt(ecdf_file)

def load_weights(weights_file):
    with open(weights_file, 'r') as f:
        lines = f.readlines()
        if not lines: return np.array([])
        return np.fromstring(lines[-1], sep=' ')

def aggregated_loss_dist(ecdfs, weights):
    w = weights / np.sum(weights)
    return np.dot(w, ecdfs)

def ecdf_to_pmf(ecdf):
    return np.diff(np.insert(ecdf, 0, 0.0))

def get_regime(p):
    gdp, unemp, infl, intr, oil = p
    scores = np.zeros(4) 

    # Crisis (0)
    scores[0] += (3.0 + abs(gdp)*0.5) if gdp < -3.0 else (1.0 - gdp*0.3 if gdp < 0 else -gdp*0.5)
    scores[0] += (unemp - 8.0)*0.4 if unemp > 8.0 else ((unemp - 6.0)*0.2 if unemp > 6.0 else -(6.0 - unemp)*0.3)
    scores[0] += abs(infl)*0.4 if infl < 0 else ((1.0 - infl)*0.2 if infl < 1.0 else -(infl - 1.0)*0.1)
    scores[0] += (1.0 - intr)*0.3 if intr < 1.0 else ((2.0 - intr)*0.15 if intr < 2.0 else 0)
    scores[0] += (50.0 - oil)*0.02 if oil < 50.0 else 0
    if gdp < -2.0 and infl > 5.0 and oil < 80.0: scores[0] -= 3.0
    if gdp < -3.0 and unemp < 5.0: scores[0] -= 5.0

    # Recession (1)
    scores[1] += (2.0 - abs(gdp + 0.5)*0.8) if -2.0 <= gdp <= 1.0 else -abs(gdp + 0.5)*0.4
    scores[1] += (1.5 - abs(unemp - 6.5)*0.3) if 5.5 <= unemp <= 8.0 else -abs(unemp - 6.5)*0.2
    scores[1] += (1.0 - abs(infl - 1.5)*0.3) if 0.5 <= infl <= 2.5 else -abs(infl - 1.5)*0.2
    if 1.0 <= intr <= 3.5: scores[1] += 1.0 - abs(intr - 2.0)*0.25
    if 45.0 <= oil <= 75.0: scores[1] += 0.5

    # Normal (2)
    scores[2] += (3.0 - abs(gdp - 2.5)*0.6) if 1.5 <= gdp <= 3.5 else -abs(gdp - 2.5)*0.4
    scores[2] += (2.0 - abs(unemp - 5.0)*0.4) if 4.0 <= unemp <= 6.0 else -abs(unemp - 5.0)*0.3
    scores[2] += (2.5 - abs(infl - 2.0)*0.8) if 1.5 <= infl <= 3.0 else -abs(infl - 2.0)*0.5
    scores[2] += (1.5 - abs(intr - 3.5)*0.3) if 2.5 <= intr <= 4.5 else -abs(intr - 3.5)*0.2
    if 55.0 <= oil <= 85.0: scores[2] += 1.0 - abs(oil - 70.0)*0.02

    # Expansion (3)
    scores[3] += (2.0 + (gdp - 3.5)*0.6) if gdp > 3.5 else ((gdp - 2.5)*1.5 if gdp > 2.5 else -(2.5 - gdp)*0.5)
    scores[3] += (4.0 - unemp)*0.8 if unemp < 4.0 else ((5.0 - unemp)*0.4 if unemp < 5.0 else -(unemp - 5.0)*0.6)
    scores[3] += (2.0 - abs(infl - 2.5)*0.4) if 1.8 <= infl <= 3.5 else (0.5 if 3.5 <= infl <= 5.0 else (-(1.8 - infl)*0.3 if infl < 1.8 else -(infl - 5.0)*0.6))
    scores[3] += (1.5 - abs(intr - 4.5)*0.2) if 3.0 <= intr <= 6.0 else (-(3.0 - intr)*0.3 if intr < 3.0 else 0)
    scores[3] += ((oil - 65.0)*0.02) if 65.0 <= oil <= 95.0 else (0.6 if oil > 95.0 else 0)

    return np.argmax(scores)

def analyze_distribution(ecdfs, weights, bins):
    if len(ecdfs) == 0:
        return None

    agg_ecdf = aggregated_loss_dist(ecdfs, weights)
    pmf = ecdf_to_pmf(agg_ecdf)
    
    el = np.sum(pmf * bins)
    ul = np.sqrt(np.sum(pmf * (bins - el)**2))
    
    # Better VaR calculation with linear interpolation
    def interpolate_var(ecdf, bins, percentile):
        idx = np.searchsorted(ecdf, percentile)
        
        if idx == 0:
            return bins[0]
        if idx >= len(bins):
            return bins[-1]
            
        # Linear interpolation between bins
        if ecdf[idx] == ecdf[idx-1]:  # Flat region
            return bins[idx]
        
        # Interpolate
        weight = (percentile - ecdf[idx-1]) / (ecdf[idx] - ecdf[idx-1])
        return bins[idx-1] + weight * (bins[idx] - bins[idx-1])
    
    var_95 = interpolate_var(agg_ecdf, bins, 0.95)
    var_99 = interpolate_var(agg_ecdf, bins, 0.99)
    var_999 = interpolate_var(agg_ecdf, bins, 0.999)
    
    # Expected Shortfall
    tail_idx = np.searchsorted(agg_ecdf, 0.99)
    tail_pmf = pmf[tail_idx:]
    tail_bins = bins[tail_idx:]
    
    if np.sum(tail_pmf) > 0:
        es_99 = np.sum(tail_pmf * tail_bins) / np.sum(tail_pmf)
    else:
        es_99 = var_99

    return {
        "EL": el,
        "UL": ul,
        "ES": es_99,
        "VaR_95": var_95,
        "VaR_99": var_99,
        "VaR_999": var_999,
        "EC": var_999 - el,
        "Tail_ratio": var_999 / var_99 if var_99 > 0 else 1.0
    }
def segregate_regimes(ecdfs, weights, regimes):
    ecdf_list = []
    weights_list = []
    for i in range(4):
        ecdf_list.append(ecdfs[regimes == i])
        weights_list.append(weights[regimes == i])
    return ecdf_list, weights_list

def plot_regime_distributions(ecdf_list, weights_list, bins, global_results):
    regime_names = ['Crisis', 'Recession', 'Normal', 'Expansion']
    colors = ['#d62728', '#ff7f0e', '#2ca02c', '#1f77b4'] # Red, Orange, Green, Blue
    
    fig, axes = plt.subplots(2, 2, figsize=(15, 10), sharex=True)
    axes = axes.flatten()
    
    for i, name in enumerate(regime_names):
        if len(ecdf_list[i]) == 0:
            axes[i].text(0.5, 0.5, "No Data for Regime", ha='center')
            continue
        
        # Get distribution data
        agg_ecdf = aggregated_loss_dist(ecdf_list[i], weights_list[i])
        pmf = ecdf_to_pmf(agg_ecdf)
        res = analyze_distribution(ecdf_list[i], weights_list[i], bins)
        
        # Plot PMF (The 'Shape' of risk)
        axes[i].fill_between(bins, pmf, color=colors[i], alpha=0.3, label='Loss Density')
        axes[i].plot(bins, pmf, color=colors[i], lw=1.5)
        
        # Add vertical lines for key risk metrics
        axes[i].axvline(res['EL'], color='black', linestyle='--', label=f"EL: {res['EL']:.2f}")
        axes[i].axvline(res['VaR_99'], color='red', linestyle='-', alpha=0.6, label=f"VaR 99%: {res['VaR_99']:.2f}")
        
        axes[i].set_title(f"Regime: {name} (N={len(ecdf_list[i])})")
        axes[i].set_ylabel("Probability")
        axes[i].legend(loc='upper right', fontsize='small')
        axes[i].grid(axis='y', alpha=0.3)

    plt.suptitle("Loss Distributions by Economic Regime", fontsize=16)
    plt.xlabel("Loss Magnitude")
    plt.tight_layout(rect=[0, 0.03, 1, 0.95])
    plt.show()

def plot_aggregated_distribution(ecdfs, weights, bins, results):
    agg_ecdf = aggregated_loss_dist(ecdfs, weights)
    pmf = ecdf_to_pmf(agg_ecdf)

    plt.figure(figsize=(10, 6))
    plt.fill_between(bins, pmf, color='purple', alpha=0.3, label='Aggregated Loss Density')
    plt.plot(bins, pmf, color='purple', lw=1.5)

    plt.axvline(results['EL'], color='black', linestyle='--', label=f"EL: {results['EL']:.2f}")
    plt.axvline(results['VaR_99'], color='red', linestyle='-', alpha=0.6, label=f"VaR 99%: {results['VaR_99']:.2f}")

    plt.title("Aggregated Loss Distribution")
    plt.xlabel("Loss Magnitude")
    plt.ylabel("Probability")
    plt.legend(loc='upper right')
    plt.grid(axis='y', alpha=0.3)
    plt.show()

if __name__ == "__main__":
    ecdfs = load_ecdfs("saved_state/particle_loss_ecdf.txt")
    weights = load_weights("saved_state/weights.txt")
    data = np.loadtxt('./saved_state/particle_grid.txt')
    bins = np.loadtxt('./saved_state/ecdf_bins.txt')
    regimes = np.array([get_regime(row) for row in data])

    ecdf_list, weights_list = segregate_regimes(ecdfs, weights, regimes)

    labels = {
        "EL": "Expected Loss",
        "UL": "Unexpected Loss",
        "ES": "Expected Shortfall (99%)",
        "VaR_99": "VaR (99%)",
        "VaR_95": "VaR (95%)",
        "EC": "Economic Capital",
        "Tail_ratio": "Tail Ratio (VaR_99.9 / VaR_99)"
    }
    regime_names = ['Crisis','Recession','Normal','Expansion']

    print(f"\nTotal particles: {len(ecdfs)}\n")

    max_weight = np.max([np.sum(row) for row in weights_list])
    max_width = 40
    print("Regime Weight Distribution:")
    print("-" * 55)

    for regime in regime_names:
        weight = np.sum(weights_list[regime_names.index(regime)])
        bar_length = int((weight / max_weight) * max_width)
        bar = "#" * bar_length
        print(f"{regime:<10} | {bar} ({round(weight,3)})")

    print("-" * 55 + '\n')

    for regime in range(4):
        print(f"Regime {regime_names[regime]}:")
        regime_results = analyze_distribution(
            ecdf_list[regime],
            weights_list[regime],
            bins
        )
        for key in labels:
            if key in regime_results:
                print(f"{labels[key]}: {round(regime_results[key], 4)}")
        print("\n\n")
    results = analyze_distribution(
        ecdfs,
        weights,
        bins
    )
    print(f"Aggregated distribution:")
    for key in labels:
        if key in results:
            print(f"{labels[key]}: {round(results[key], 4)}")
    
    plot_regime_distributions(ecdf_list, weights_list, bins, results)
    plot_aggregated_distribution(ecdfs, weights, bins, results)

