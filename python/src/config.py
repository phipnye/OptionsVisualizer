from pydantic import BaseModel, ConfigDict
from typing import Optional


class Settings(BaseModel):
    model_config: ConfigDict = ConfigDict(frozen=True)

    # --- System and environment
    DEBUG: bool = False
    ENGINE_CAPACITY: int = 16
    ENGINE_THREADS: Optional[int] = None
    PLOT_THEME: str = "darkly"

    # --- Core app parameters
    GRID_RESOLUTION: int = 12  # text labels become unreadable at higher resolutions
    NUM_TICKS: int = 6  # number of visual marks on sliders
    HEATMAP_WIDTH: int = 6  # column width for individual heatmap panels

    # --- Spot and strike
    SPOT_MIN: float = 1.0
    SPOT_MAX: float = 10000.0
    SPOT_STEP: float = 0.01
    SPOT_DEFAULT: float = 100.0

    STRIKE_VAL_PCTS: list[float] = [0.8, 1.2]  # default window: +/- 20%
    STRIKE_RANGE_PCTS: list[float] = [0.5, 1.5]  # permitted range: +/- 50%
    STRIKE_STEP: float = 0.1

    # --- Volatility
    SIGMA_MIN: float = 0.01
    SIGMA_MAX: float = 2.0
    SIGMA_STEP: float = 0.005
    SIGMA_DEFAULTS: list[float] = [0.1, 0.4]

    # --- Time to maturity
    TAU_MIN: float = 0.001
    TAU_MAX: float = 3.0
    TAU_STEP: float = 0.001
    TAU_DEFAULT: float = 1.0

    # --- Risk-free rate
    RATE_MIN: float = -0.05
    RATE_MAX: float = 0.15
    RATE_STEP: float = 0.0001
    RATE_DEFAULT: float = 0.05

    # --- Dividend
    DIV_MIN: float = 0.0
    DIV_MAX: float = 0.15
    DIV_STEP: float = 0.0001
    DIV_DEFAULT: float = 0.02

    # --- App colors and styling
    SUMMARY_COLOR: str = "#00BC8C"
    APP_TITLE_FONT_SIZE: int = 20
    TICK_COLOR: str = "#9C9C9C"  # used for slider marks and modebar icons

    # --- Plotly styling
    PLOT_COLOR_SCALE: str = "thermal"
    PLOT_FONT_SIZE: int = 14
    PLOT_FONT_FAMILY: str = "Arial"
    PLOT_TITLE_SIZE: int = 14
    PLOT_TITLE_XPOS: float = 0.5
    PLOT_MARGINS: dict[str, int] = dict(l=40, r=10, t=35, b=35)
    PLOT_COLORBAR_THICKNESS: int = 15

    # --- String formatting
    PREC_STANDARD: str = ".2f"
    PREC_GAMMA: str = ".4f"  # higher precision required for gamma sensitivity
    X_AXIS_FORMAT: str = ".2f"
    Y_AXIS_FORMAT: str = ".1%"


SETTINGS: Settings = Settings()
