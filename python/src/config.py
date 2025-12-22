from pydantic import BaseModel, ConfigDict


class Settings(BaseModel):
    # --- Make settings frozen
    model_config: ConfigDict = ConfigDict(frozen=True)

    # --- App and engine settings
    DEBUG: bool = True
    ENGINE_CAPACITY: int = 16
    GRID_RESOLUTION: int = 10

    # --- Plot Layout & Styling
    PLOT_THEME: str = "darkly"
    FONT_FAMILY: str = "Arial"
    PLOT_FONT_SIZE: int = 12
    TITLE_FONT_SIZE: int = 14
    TITLE_X_POS: float = 0.5  # centered title
    COLOR_SCALE: str = "thermal"
    COLORBAR_THICKNESS: int = 15
    PREC_STANDARD: str = ".2f"
    PREC_GAMMA: str = ".4f"
    X_AXIS_FORMAT: str = ".0f"
    Y_AXIS_FORMAT: str = ".2f"
    PLOT_MARGINS: dict[str, int] = dict(l=40, r=10, t=35, b=35)

    # --- UI styling tokens
    HEATMAP_WIDTH: int = 6
    SUMMARY_COLOR: str = "#00BC8C"  # param summary block color
    TICK_COLOR: str = "#9C9C9C"  # light grey for slider ticks

    # --- Domain parameters: (min, max, step, default)
    # Spot price
    SPOT_MIN: float = 0.01
    SPOT_MAX: float = 10000.0
    SPOT_STEP: float = 0.01
    SPOT_DEFAULT: float = 100.0

    # Time to maturity
    TAU_MIN: float = 0.01
    TAU_MAX: float = 5.0
    TAU_STEP: float = 0.01
    TAU_DEFAULT: float = 1.0

    # Risk-free rate
    RATE_MIN: float = -0.05
    RATE_MAX: float = 0.5
    RATE_STEP: float = 0.0001
    RATE_DEFAULT: float = 0.05

    # Dividend yield
    DIV_MIN: float = 0.0
    DIV_MAX: float = 0.5
    DIV_STEP: float = 0.0001
    DIV_DEFAULT: float = 0.02

    # Volatility (slider)
    SIGMA_MIN: float = 0.01
    SIGMA_MAX: float = 1.0
    SIGMA_STEP: float = 0.005
    SIGMA_DEFAULTS: list[float] = [0.1, 0.4]

    # Strike (slider) - set value to += 20% and valid range to += 50%
    STRIKE_VAL_PCTS: list[float] = [0.8, 1.2]
    STRIKE_RANGE_PCTS: list[float] = [0.5, 1.5]
    STRIKE_STEP: float = 0.1


SETTINGS: Settings = Settings()
