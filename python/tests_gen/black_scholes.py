import numpy as np
from enums import OptionType
from scipy.stats import norm

def black_scholes(s: np.ndarray, k: np.ndarray, t: np.ndarray, r: np.ndarray, q: np.ndarray, sigma: np.ndarray,
                  option: OptionType) -> dict[str, np.ndarray]:
    # Pre-calculate common terms
    sqrt_t: np.ndarray = np.sqrt(t)
    sigma_sqrt_t: np.ndarray = sigma * sqrt_t
    d1: np.ndarray = (np.log(s / k) + (r - q + 0.5 * sigma**2) * t) / sigma_sqrt_t
    d2: np.ndarray = d1 - sigma_sqrt_t
    pdf_d1: np.ndarray = norm.pdf(d1)
    cdf_d1: np.ndarray = norm.cdf(d1)
    cdf_d2: np.ndarray = norm.cdf(d2)
    term1: np.ndarray = -(s * pdf_d1 * sigma * np.exp(-q * t)) / (2.0 * sqrt_t)

    # Compute common results
    gamma: np.ndarray = np.exp(-q * t) * pdf_d1 / (s * sigma_sqrt_t)
    vega: np.ndarray = s * np.exp(-q * t) * pdf_d1 * sqrt_t

    # Compute results dependent on option type
    if option == OptionType.CALL:
        price: np.ndarray = s * np.exp(-q * t) * norm.cdf(d1) - k * np.exp(-r * t) * norm.cdf(d2)
        delta: np.ndarray = np.exp(-q * t) * cdf_d1
        theta: np.ndarray = term1 - r * k * np.exp(-r * t) * cdf_d2 + q * s * np.exp(-q * t) * cdf_d1
        rho: np.ndarray = k * t * np.exp(-r * t) * cdf_d2
    else:
        price = k * np.exp(-r * t) * norm.cdf(-d2) - s * np.exp(-q * t) * norm.cdf(-d1)
        delta = np.exp(-q * t) * (cdf_d1 - 1.0)
        theta = term1 + r * k * np.exp(-r * t) * norm.cdf(-d2) - q * s * np.exp(-q * t) * norm.cdf(-d1)
        rho = -k * t * np.exp(-r * t) * norm.cdf(-d2)

    return {
        "price": price,
        "delta": delta,
        "gamma": gamma,
        "vega": vega,
        "theta": theta,
        "rho": rho,
    }