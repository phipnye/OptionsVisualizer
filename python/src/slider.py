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
    # Calculate slider boundaries relative to current spot price
    strike_min: float = spot * SETTINGS.STRIKE_RANGE_PCTS[0]
    strike_max: float = spot * SETTINGS.STRIKE_RANGE_PCTS[1]

    # Generate tick values for the visual scale (round the key to match slider resolution; format label for readability)
    marks: dict[int, dict] = {
        int(round(v, 0)): {"label": f"{v:,.0f}", "style": {"color": SETTINGS.TICK_COLOR}}
        for v in np.linspace(strike_min, strike_max, SETTINGS.NUM_TICKS)
    }

    return StrikeSliderConfig(
        min=strike_min, max=strike_max, value=[spot * pct for pct in SETTINGS.STRIKE_VAL_PCTS], marks=marks
    )
