import numpy as np
import matplotlib.pyplot as plt
from tabulate import tabulate

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
    
    # Crisis (0): Severe contraction, high unemployment, financial stress
    # Key: GDP deeply negative OR unemployment very high
    if gdp < -2.0:
        scores[0] += 5.0 + abs(gdp) * 0.8  # Severe GDP contraction is strong crisis signal
    elif gdp < 0:
        scores[0] += 2.0 - gdp * 0.5
    else:
        scores[0] -= gdp * 0.6  # Positive GDP argues against crisis
    
    if unemp > 9.0:
        scores[0] += 4.0 + (unemp - 9.0) * 0.5  # Very high unemployment
    elif unemp > 7.0:
        scores[0] += 2.0 + (unemp - 7.0) * 0.8
    else:
        scores[0] -= (7.0 - unemp) * 0.4
    
    # Crisis tends to have deflationary pressure or extreme inflation
    if infl < 0:
        scores[0] += 2.0 + abs(infl) * 0.5  # Deflation is crisis signal
    elif infl > 6.0:
        scores[0] += 1.5 + (infl - 6.0) * 0.3  # High inflation can indicate crisis
    
    # Low interest rates (emergency stimulus) or very high (panic)
    if intr < 1.0:
        scores[0] += 1.5
    elif intr > 8.0:
        scores[0] += 1.0
    
    # Oil collapse can signal crisis
    if oil < 40.0:
        scores[0] += (40.0 - oil) * 0.03
    
    # Recession (1): Mild contraction, elevated unemployment
    # Key: GDP near zero or slightly negative, unemployment 6-9%
    if -2.0 <= gdp <= 1.5:
        scores[1] += 3.0 - abs(gdp - 0.0) * 0.5  # Peak at GDP=0
    else:
        scores[1] -= abs(gdp - 0.0) * 0.4
    
    if 6.0 <= unemp <= 9.0:
        scores[1] += 3.5 - abs(unemp - 7.0) * 0.3  # Peak at 7% unemployment
    else:
        scores[1] -= abs(unemp - 7.0) * 0.35
    
    if 0.5 <= infl <= 3.0:
        scores[1] += 1.5 - abs(infl - 1.5) * 0.3
    else:
        scores[1] -= abs(infl - 1.5) * 0.25
    
    if 1.0 <= intr <= 4.0:
        scores[1] += 1.0 - abs(intr - 2.5) * 0.2
    
    if 45.0 <= oil <= 70.0:
        scores[1] += 1.0 - abs(oil - 55.0) * 0.02
    
    # Normal (2): Steady growth, moderate unemployment, stable inflation
    # Key: GDP 1.5-3.5%, unemployment 4-6%, inflation 1.5-3%
    if 1.5 <= gdp <= 3.5:
        scores[2] += 4.0 - abs(gdp - 2.5) * 0.5  # Peak at 2.5%
    else:
        scores[2] -= abs(gdp - 2.5) * 0.5
    
    if 4.0 <= unemp <= 6.5:
        scores[2] += 3.5 - abs(unemp - 5.0) * 0.4  # Peak at 5%
    else:
        scores[2] -= abs(unemp - 5.0) * 0.45
    
    if 1.5 <= infl <= 3.5:
        scores[2] += 3.0 - abs(infl - 2.5) * 0.6  # Peak at 2.5% (target inflation)
    else:
        scores[2] -= abs(infl - 2.5) * 0.5
    
    if 2.5 <= intr <= 5.0:
        scores[2] += 2.0 - abs(intr - 3.5) * 0.3  # Peak at 3.5%
    else:
        scores[2] -= abs(intr - 3.5) * 0.3
    
    if 55.0 <= oil <= 85.0:
        scores[2] += 1.5 - abs(oil - 70.0) * 0.02  # Peak at $70
    
    # Expansion (3): Strong growth, low unemployment, rising asset prices
    # Key: GDP > 3.5%, unemployment < 4.5%
    if gdp > 3.5:
        scores[3] += 3.0 + (gdp - 3.5) * 0.7  # Strong positive GDP
    elif gdp > 2.0:
        scores[3] += (gdp - 2.0) * 1.2
    else:
        scores[3] -= (2.0 - gdp) * 0.6
    
    if unemp < 4.5:
        scores[3] += 4.5 + (4.5 - unemp) * 0.9  # Very low unemployment
    elif unemp < 6.0:
        scores[3] += (6.0 - unemp) * 0.5
    else:
        scores[3] -= (unemp - 6.0) * 0.7
    
    # Expansion can have moderate inflation (overheating) or low (goldilocks)
    if 1.8 <= infl <= 4.0:
        scores[3] += 2.0 - abs(infl - 2.5) * 0.3
    elif infl > 4.0:
        scores[3] += 0.5 - (infl - 4.0) * 0.4  # Too hot
    else:
        scores[3] -= (1.8 - infl) * 0.4
    
    if 3.0 <= intr <= 6.0:
        scores[3] += 1.5 - abs(intr - 4.5) * 0.2
    else:
        scores[3] -= abs(intr - 4.5) * 0.25
    
    # Rising oil prices during expansion
    if 70.0 <= oil <= 100.0:
        scores[3] += (oil - 70.0) * 0.025
    elif oil > 100.0:
        scores[3] += 0.75  # Very high oil
    else:
        scores[3] -= (70.0 - oil) * 0.02
    
    return np.argmax(scores)



def analyze_distribution(ecdfs, weights, bins):
    if len(ecdfs) == 0:
        return None

    agg_ecdf = aggregated_loss_dist(ecdfs, weights)
    pmf = ecdf_to_pmf(agg_ecdf)
    
    el = np.sum(pmf * bins)
    ul = np.sqrt(np.sum(pmf * (bins - el)**2))
    
    def interpolate_var(ecdf, bins, percentile):
        idx = np.searchsorted(ecdf, percentile)
        
        # Checkers
        if idx == 0:
            return bins[0]
        if idx >= len(bins):
            return bins[-1]
            
        if ecdf[idx] == ecdf[idx-1]:
            return bins[idx]
        
        slope = (percentile - ecdf[idx-1]) / (ecdf[idx] - ecdf[idx-1])
        return bins[idx-1] + slope * (bins[idx] - bins[idx-1])
    
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
        "EL": f'$ {el:,.0f}',
        "UL": f'$ {ul:,.0f}',
        "ES": f'$ {es_99:,.0f}',
        "VaR_95": f'$ {var_95:,.0f}',
        "VaR_99": f'$ {var_99:,.0f}',
        "VaR_999": f'$ {var_999:,.0f}',
        "EC": f'$ {var_999-el:,.0f}',
        "Tail_ratio": f'{var_999 / var_99:.3f}'
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
        axes[i].axvline(int(res['EL'].replace('$', '').replace(',', '').strip()), color='black', linestyle='--', label=f"EL: {res['EL']}")
        axes[i].axvline(int(res['VaR_99'].replace('$', '').replace(',', '').strip()), color='red', linestyle='-', alpha=0.6, label=f"VaR 99%: {res['VaR_99']}")

        axes[i].set_title(f"Regime: {name} (N={len(ecdf_list[i])})")
        axes[i].set_ylabel("Probability")
        axes[i].legend(loc='upper right', fontsize='small')
        axes[i].grid(axis='y', alpha=0.3)

    plt.suptitle("Loss Distributions by Economic Regime", fontsize=16)
    plt.xlabel("Loss Magnitude")
    plt.tight_layout(rect=[0, 0.03, 1, 0.95])
    plt.savefig('./saved_state/regime_loss_distributions.png', dpi=300, transparent=False)
    plt.show()

def plot_aggregated_distribution(ecdfs, weights, bins, results):
    agg_ecdf = aggregated_loss_dist(ecdfs, weights)
    pmf = ecdf_to_pmf(agg_ecdf)

    plt.figure(figsize=(10, 6))
    plt.fill_between(bins, pmf, color='purple', alpha=0.3, label='Aggregated Loss Density')
    plt.plot(bins, pmf, color='purple', lw=1.5)

    plt.axvline(int(results['EL'].replace('$', '').replace(',', '').strip()), color='black', linestyle='--', label=f"EL: {results['EL']}")
    plt.axvline(int(results['VaR_99'].replace('$', '').replace(',', '').strip()), color='red', linestyle='-', alpha=0.6, label=f"VaR 99%: {results['VaR_99']}")

    plt.title("Aggregated Loss Distribution")
    plt.xlabel("Loss Magnitude")
    plt.ylabel("Probability")
    plt.legend(loc='upper right')
    plt.grid(axis='y', alpha=0.3)
    plt.savefig('./saved_state/aggregated_loss_distribution.png', dpi=300, transparent=False)
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

    print(f"Total particles: {len(ecdfs)}\n")

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

    results = analyze_distribution(
        ecdfs,
        weights,
        bins
    )
    print(f"Aggregated distribution:")
    for key in labels:
        if key in results:
            print(f"{labels[key]}: {results[key]}")
    data = []
    print("\nRegime wise Distributions:")

    for regime in range(4):
        regime_results = analyze_distribution(
            ecdf_list[regime],
            weights_list[regime],
            bins
        )
        data.append([regime_results[k] for k in labels.keys()])
    print(tabulate(list(zip(*data)), headers=regime_names, showindex=labels.values(), tablefmt='fancy_grid', stralign="center"))
    
    
    plot_aggregated_distribution(ecdfs, weights, bins, results)
    plot_regime_distributions(ecdf_list, weights_list, bins, results)

