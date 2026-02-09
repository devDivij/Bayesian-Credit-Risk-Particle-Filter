import numpy as np

def load_ecdfs(ecdf_file):
    ecdfs = []
    with open(ecdf_file, "r") as f:
        for line in f:
            ecdfs.append([float(x) for x in line.split()])

    ecdfs = np.array(ecdfs)

    return ecdfs

def load_weights(weights_file):
    with open(weights_file, 'r') as f:
        lines = f.readlines()
        last_line = lines[-1].strip()  # Get the last line and remove whitespace
        weights = np.array([float(x) for x in last_line.split()])
    return weights

def aggregated_loss_dist(ecdfs, weights):
    n_distributions, n_bins = ecdfs.shape
    aggregated_dist = np.zeros(n_bins)

    for i in range(n_distributions):
        aggregated_dist += weights[i] * ecdfs[i]

    return aggregated_dist

def ecdf_to_pmf(ecdf):
    pmf = np.diff(np.concatenate(([0.0], ecdf)))
    return pmf

def analyze_distribution(ecdf_file, weights_file, bins_file):

    bins = np.loadtxt(bins_file)
    weights = load_weights(weights_file)
    ecdfs = load_ecdfs(ecdf_file)

    loss_dist_ecdf = aggregated_loss_dist(ecdfs, weights)
    loss_dist_ecdf = loss_dist_ecdf / loss_dist_ecdf[-1]

    idx0 = np.searchsorted(loss_dist_ecdf,1.00)

    loss_dist_ecdf.resize(idx0+1,refcheck=False)
    bins.resize(idx0+1,refcheck=False)

    loss_dist_pmf = ecdf_to_pmf(loss_dist_ecdf)
    loss_dist_pmf = loss_dist_pmf / np.sum(loss_dist_pmf)
    results = {}

    # Expected Loss
    EL = np.sum(loss_dist_pmf*bins)

    # Unexpected Loss
    UL = np.sqrt(np.sum(loss_dist_pmf*(bins-EL)**2))

    # Value-at-Risk
    idx1 = np.searchsorted(loss_dist_ecdf, 0.99)
    idx2 = np.searchsorted(loss_dist_ecdf, 0.95)
    idx3 = np.searchsorted(loss_dist_ecdf, 0.999)
    VaR_99= bins[idx1]
    VaR_95= bins[idx2]
    VaR_999= bins[idx3]

    # Expected Shortfall
    ES = np.sum(loss_dist_pmf[idx1:idx0+1]*bins[idx1:idx0+1])/np.sum(loss_dist_pmf[idx1:idx0+1])

    # Economic Capital
    EC = VaR_999 - EL

    results["EL"] = EL
    results["UL"] = UL
    results["ES"] = ES
    results["VaR_99"] = VaR_99
    results["VaR_95"] = VaR_95
    results["EC"] = EC
    results["Tail_ratio"] = VaR_999/VaR_99

    return results



if __name__ == "__main__":
    results = analyze_distribution(
        ecdf_file="saved_state/particle_loss_ecdf.txt",
        weights_file="saved_state/weights.txt",
        bins_file="saved_state/ecdf_bins.txt"
    )

    # Mapping from shorthand to descriptive label
    labels = {
        "EL": "Expected Loss",
        "UL": "Unexpected Loss",
        "ES": "Expected Shortfall (99%)",
        "VaR_99": "VaR (99%)",
        "VaR_95": "VaR (95%)",
        "EC": "Economic Capital",
        "Tail_ratio": "Tail Ratio (VaR_99.9 / VaR_99)"
    }

    for key in labels:
        if key in results:
            print(f"{labels[key]}: {round(results[key], 4)}")
