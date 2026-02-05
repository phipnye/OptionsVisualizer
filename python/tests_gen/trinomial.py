import numpy as np
from enums import OptionType

_TRINOMIAL_DEPTH: int = 100  # copy from constants.hpp

def _intrinsic(s: np.ndarray, k: np.ndarray, option_type: OptionType) -> np.ndarray:
    if option_type == OptionType.CALL:
        return np.maximum(s - k, 0.0)

    if option_type == OptionType.PUT:
        return np.maximum(k - s, 0.0)

    raise ValueError("Unsupported option type for trinomial pricing")

def _trinomial_price(s: np.ndarray, k: np.ndarray, t: np.ndarray, r: np.ndarray, q: np.ndarray, sigma: np.ndarray,
                    option: OptionType) -> np.ndarray:
    d_t: np.ndarray = t / _TRINOMIAL_DEPTH
    sigma_sq: np.ndarray = sigma * sigma
    u: np.ndarray = np.exp(sigma * np.sqrt(3.0 * d_t))
    discount_factor: np.ndarray = np.exp(-r * d_t)
    scaling_term: np.ndarray = np.sqrt(d_t / (12.0 * sigma_sq))
    log_stock_drift: np.ndarray = (r - q) - 0.5 * sigma_sq
    drift_factor: np.ndarray = scaling_term * log_stock_drift
    p_u: np.ndarray = drift_factor + (1.0 / 6.0)
    p_d: np.ndarray = -drift_factor + (1.0 / 6.0)
    p_m: np.ndarray = 1.0 - p_u - p_d
    max_nodes: int = 2 * _TRINOMIAL_DEPTH + 1
    next_vals: list[np.ndarray] = [np.zeros_like(s) for _ in range(max_nodes)]
    curr_vals: list[np.ndarray] = [np.zeros_like(s) for _ in range(max_nodes)]

    # Layer at maturity
    for node in range(max_nodes):
        s_T: np.ndarray = s * np.pow(u, -_TRINOMIAL_DEPTH + node)
        next_vals[node] = _intrinsic(s_T, k, option)

    for depth in range(_TRINOMIAL_DEPTH - 1, -1, -1):
        n_nodes: int = 2 * depth + 1

        for node in range(n_nodes):
            val_u: np.ndarray = next_vals[node + 2]
            val_m: np.ndarray = next_vals[node + 1]
            val_d: np.ndarray = next_vals[node]
            continuation: np.ndarray = (p_u * val_u + p_m * val_m + p_d * val_d) * discount_factor
            s_t: np.ndarray = s * np.pow(u, -depth + node)
            exercise: np.ndarray = _intrinsic(s_t, k, option)
            curr_vals[node] = np.maximum(continuation, exercise)

        next_vals, curr_vals = curr_vals, next_vals

    return next_vals[0]

def first_order_cdm(lo: np.ndarray, hi: np.ndarray, eps: float | np.ndarray) -> np.ndarray:
    return (hi - lo) / (2.0 * eps)

def second_order_cdm(lo: np.ndarray, base: np.ndarray, hi: np.ndarray, eps: float | np.ndarray) -> np.ndarray:
    return (hi - 2.0 * base + lo) / (eps * eps)


def trinomial(s: np.ndarray, k: np.ndarray, t: np.ndarray, r: np.ndarray, q: np.ndarray, sigma: np.ndarray,
              option: OptionType) -> dict[str, np.ndarray]:
    # Copy from C++
    h_spot: np.ndarray = s * 0.05
    h_tau: np.ndarray = t * 0.01
    h_rho: np.ndarray = r * 0.01
    h_sigma: np.ndarray = sigma * 0.01

    # Base price
    base: np.ndarray = _trinomial_price(s, k, t, r, q, sigma, option)

    # Spot perturbations
    lo_spot: np.ndarray = _trinomial_price(s - h_spot, k, t, r, q, sigma, option)
    hi_spot: np.ndarray = _trinomial_price(s + h_spot, k, t, r, q, sigma, option)

    # Sigma perturbations
    lo_sigma: np.ndarray = _trinomial_price(s, k, t, r, q, sigma - h_sigma, option)
    hi_sigma: np.ndarray = _trinomial_price(s, k, t, r, q, sigma + h_sigma, option)

    # TTM perturbations
    lo_tau: np.ndarray = _trinomial_price(s, k, t - h_tau, r, q, sigma, option)
    hi_tau: np.ndarray = _trinomial_price(s, k, t + h_tau, r, q, sigma, option)

    # Risk-free rate perturbations
    lo_rho: np.ndarray = _trinomial_price(s, k, t, r - h_rho, q, sigma, option)
    hi_rho: np.ndarray = _trinomial_price(s, k, t, r + h_rho, q, sigma, option)

    # Use CDM to compute derivatives
    delta: np.ndarray = first_order_cdm(lo_spot, hi_spot, h_spot)
    vega: np.ndarray = first_order_cdm(lo_sigma, hi_sigma, h_sigma)
    theta: np.ndarray = -first_order_cdm(lo_tau, hi_tau, h_tau)
    rho: np.ndarray = first_order_cdm(lo_rho, hi_rho, h_rho)
    gamma: np.ndarray = second_order_cdm(lo_spot, base, hi_spot, h_spot)
    return {"price": base, "delta": delta, "gamma": gamma, "vega": vega, "theta": theta, "rho": rho}
