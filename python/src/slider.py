import numpy as np
from dataclasses import dataclass
from config import SETTINGS


@dataclass(frozen=True)
class StrikeSliderConfig:
    min: float
    max: float
    value: list[float]
    marks: dict[float, dict]


def compute_strike_slider(spot: float) -> StrikeSliderConfig:
    # Define the stike slider default values and permitted range of values based on the spot price
    slider_value: list[float] = [spot * pct for pct in SETTINGS.STRIKE_VAL_PCTS]
    strike_range: list[float] = [spot * pct for pct in SETTINGS.STRIKE_RANGE_PCTS]
    strike_min: float = strike_range[0]
    strike_max: float = strike_range[1]
    range_width: float = strike_max - strike_min
    raw_interval: float = range_width / 10
    magnitude: np.float64 = 10 ** np.floor(np.log10(raw_interval))

    # Determine the cleanest interval (1, 2, 5, 10, etc. times the magnitude)
    if raw_interval / magnitude <= 1.5:
        mark_interval: np.float64 = magnitude
    elif raw_interval / magnitude <= 3.5:
        mark_interval = magnitude * 2.0
    elif raw_interval / magnitude <= 7.5:
        mark_interval = magnitude * 5.0
    else:
        mark_interval = magnitude * 10.0

    # Generate the actual marks dictionary
    mark_interval: np.float64 = max(np.float64(1.0), mark_interval)
    start_mark: float = np.ceil(strike_min / mark_interval) * mark_interval  # first clean mark >= strike_min
    marks: dict[float, dict[str, str | dict[str, str]]] = {
        float(i): {"label": f"{i:,.2f}", "style": {"color": SETTINGS.TICK_COLOR}}
        for i in np.arange(start_mark, strike_max + 0.001, mark_interval)
    }
    return StrikeSliderConfig(strike_min, strike_max, slider_value, marks)
