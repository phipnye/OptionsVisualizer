from config import SETTINGS

_NUMERIC_LIMITS: dict[str, tuple[float, float]] = {
    "spot": (SETTINGS.SPOT_MIN, SETTINGS.SPOT_MAX),
    "strike_range": (SETTINGS.SPOT_MIN * SETTINGS.STRIKE_RANGE_PCTS[0],
                     SETTINGS.SPOT_MAX * SETTINGS.STRIKE_RANGE_PCTS[1]),
    "tau": (SETTINGS.TAU_MIN, SETTINGS.TAU_MAX),
    "r": (SETTINGS.RATE_MIN, SETTINGS.RATE_MAX),
    "q": (SETTINGS.DIV_MIN, SETTINGS.DIV_MAX),
    "sigma_range": (SETTINGS.SIGMA_MIN, SETTINGS.SIGMA_MAX),
}


def is_valid_value(name: str, val: float | list | tuple) -> bool:
    if val is None:
        return False

    lo: float
    hi: float
    lo, hi = _NUMERIC_LIMITS[name]

    if isinstance(val, (list, tuple)):
        return len(val) == 2 and val[0] <= val[1] and lo <= val[0] <= hi and lo <= val[1] <= hi

    return lo <= val <= hi


def all_valid(**inputs) -> bool:
    return all(is_valid_value(name, val) for name, val in inputs.items())
