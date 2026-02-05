"""
Generate tests manually that we can use to backtest C++ engine's results against
"""

import os
import sys

# Ok for a one-off script (allows us to import SETTINGS relative to project directory)
PROJ_DIR: str = os.path.dirname(os.path.dirname(os.path.dirname(__file__)))
sys.path.append(PROJ_DIR)

import numpy as np
from black_scholes import black_scholes
from enums import OptionType
from python.src.config import SETTINGS
from trinomial import trinomial
from typing import Any


def generate_test_results(nrow: int, ncol: int, seed: int = 1) -> dict[str, Any]:
    rng: np.random.Generator = np.random.default_rng(seed)
    shape: tuple[int, int] = (nrow, ncol)
    s: np.ndarray = rng.uniform(SETTINGS.SPOT_MIN, SETTINGS.SPOT_MAX, shape)
    k: np.ndarray = s * rng.uniform(*SETTINGS.STRIKE_VAL_PCTS, shape)  # base strike on spot
    t: np.ndarray = rng.uniform(SETTINGS.TAU_MIN, SETTINGS.TAU_MAX, shape)
    r: np.ndarray = rng.uniform(SETTINGS.RATE_MIN, SETTINGS.RATE_MAX, shape)
    q: np.ndarray = rng.uniform(SETTINGS.DIV_MIN, SETTINGS.DIV_MAX, shape)
    sigma: np.ndarray = rng.uniform(SETTINGS.SIGMA_MIN, SETTINGS.SIGMA_MAX, shape)
    amer_res: dict[str, dict[str, np.ndarray]] = {
        opt.name.lower(): trinomial(s, k, t, r, q, sigma, opt) for opt in (OptionType.CALL, OptionType.PUT)
    }
    euro_res: dict[str, dict[str, np.ndarray]] = {
        opt.name.lower(): black_scholes(s, k, t, r, q, sigma, opt) for opt in (OptionType.CALL, OptionType.PUT)
    }

    return {
        "s": s,
        "k": k,
        "t": t,
        "r": r,
        "q": q,
        "sigma": sigma,
        "amer_res": amer_res,
        "euro_res": euro_res,
    }


def test_data_path(x: str = "") -> str:
    return os.path.join(PROJ_DIR, 'tests', 'data', x)


def save_csv(x: np.ndarray, fname: str) -> None:
    np.savetxt(test_data_path(fname), x, delimiter=',', fmt="%.18e")


def produce_test_cases(res: dict[str, Any]) -> None:
    os.makedirs(test_data_path(), exist_ok=True)

    # Scalar grids
    for name in ("s", "k", "t", "r", "q", "sigma"):
        save_csv(res[name], f"{name}.csv")

    # American and Europena option results
    for opt_class in ("amer", "euro"):
        for opt_name, metrics in res[f"{opt_class}_res"].items():
            for metric_name, arr in metrics.items():
                save_csv(arr, f"{opt_class}_{opt_name}_{metric_name}.csv")


if __name__ == "__main__":
    with open(test_data_path("input_test_size.txt"), 'r') as f:
        test_size: int = int(f.read().strip())

    print(f"Producing {test_size} test cases")
    results: dict[str, Any] = generate_test_results(nrow=test_size, ncol=1)
    produce_test_cases(results)
