import dash
import pricing
import numpy as np
import hashlib
from flask_caching import Cache
from constants import (
    GREEK_SYMBOLS,
    GREEK_TYPES,
    GRID_RESOLUTION,
    OPTION_TYPES,
)
from dash import html
from dash.dependencies import Input, Output, State
from helpers import (
    create_control_panel,
    create_heatmap_grid,
    generate_heatmap_figure,
)
from plotly.graph_objects import Figure
from styles import (
    COLOR_TEXT_DEFAULT,
    COLOR_SLIDER_MARK,
    FONT_SIZE_XS,
    GAP_LG,
    PADDING_MD,
)
from typing import Optional

# Initialize the Dash application using a dark mode theme
APP: dash.Dash = dash.Dash(
    __name__,
    external_stylesheets=["https://cdn.jsdelivr.net/npm/bootswatch@5.3.3/dist/darkly/bootstrap.min.css"]
)

CACHE: Cache = Cache(APP.server, config={"CACHE_TYPE": "simple", "CACHE_DEFAULT_TIMEOUT": 3600})

# --- Layout definition
APP.layout: html.Div = html.Div(  # type: ignore
    style={"margin": "0 auto", "padding": PADDING_MD},
    children=[
        html.H1(
            "Black-Scholes & Trinomial Option Pricing Engine Dashboard",
            style={"textAlign": "center", "color": COLOR_TEXT_DEFAULT, "marginBottom": "40px"}
        ),

        # Main 1x2 flexbox container
        html.Div(
            style={"display": "flex", "gap": GAP_LG},
            children=[
                create_control_panel(),  # Left column
                create_heatmap_grid()    # Right column
            ]
        ),

        # Cache greek results (prevent re-computing when switiching greek type for the same model parameters)
        dash.dcc.Store(id="greeks_cache", storage_type="memory"),
    ]
)

# Callback: Dynamically adjusts strike range based on spot price input
@APP.callback(
    Output("strike_range", "min"),
    Output("strike_range", "max"),
    Output("strike_range", "step"),
    Output("strike_range", "value"),
    Output("strike_range", "marks"),
    Input("input_S", "value")
)
def update_strike_and_spot(stock_price: float) -> tuple[float, float, float, list[float], dict]:
    # Use a fallback price if input is None or invalid
    if stock_price is None or stock_price <= 0:
        stock_price = 100.0

    # Define the slider range (50% to 150% of spot price)
    strike_min: float = stock_price * 0.5
    strike_max: float = stock_price * 1.5
    strike_step: float = 0.1
    slider_value: list[float] = [stock_price * 0.8, stock_price * 1.2]

    # Logic to create clean, well-spaced tick marks for the slider
    range_width: float = strike_max - strike_min
    raw_interval: float = range_width / 10
    magnitude: float = 10 ** np.floor(np.log10(raw_interval))

    # Determine the cleanest interval (1, 2, 5, 10, etc. times the magnitude)
    if raw_interval / magnitude <= 1.5:
        mark_interval: float = magnitude * 1.0
    elif raw_interval / magnitude <= 3.5:
        mark_interval = magnitude * 2.0
    elif raw_interval / magnitude <= 7.5:
        mark_interval = magnitude * 5.0
    else:
        mark_interval = magnitude * 10.0

    mark_interval = max(1.0, mark_interval)
    marks: dict[float, dict[str, str | dict[str, str]]] = {}
    start_mark: float = np.ceil(strike_min / mark_interval) * mark_interval  # first clean mark >= to strike_min

    # Generate the actual marks dictionary
    for i in np.arange(start_mark, strike_max + 0.001, mark_interval):
        label: str = f"{i:,.0f}" if i == int(i) else f"{i:,.2f}"
        marks[i] = {"label": label, "style": {"color": COLOR_SLIDER_MARK, "fontSize": FONT_SIZE_XS}}

    return strike_min, strike_max, strike_step, slider_value, marks

# Callback: Updates the text summary of fixed parameters
@APP.callback(
    Output("param_summary", "children"),
    [
        Input("input_S", "value"),
        Input("input_T", "value"),
        Input("input_r", "value"),
        Input("input_q", "value"),
    ]
)
def update_param_summary(S: float, T: float, r: float, q: float) -> html.Pre:
    # Input validation: Checks if Dash returned None (due to input being out of min/max range)
    if S is None or T is None or r is None or q is None:
        return html.Pre("S = ERROR\nT = ERROR\nr = ERROR\nq = ERROR\n\n(Input out of configured range)")

    # Format and return the parameters using pre-formatted text
    return html.Pre(f"S = ${S:,.2f}\nT = {T:.2f} years\nr = {r:.2%}\nq = {q:.2%}")

# Callback: Updates the main plot title when the Greek selector changes
@APP.callback(
    Output("plot_header_title", "children"),
    Input("greek_selector", "value")
)
def update_plot_header_title(greek_idx: int) -> str:
    greek_name: str = GREEK_TYPES.get(greek_idx, "Price")
    symbol: str = GREEK_SYMBOLS.get(greek_name, "Metric")
    return f"{symbol} Heatmap Comparison (K vs. \u03C3)"

@APP.callback(
    Output("greeks_cache", "data"),
    [
        Input("strike_range", "value"),
        Input("sigma_range", "value"),
        Input("input_S", "value"),
        Input("input_T", "value"),
        Input("input_r", "value"),
        Input("input_q", "value"),
    ]
)
def compute_greeks_and_cache(
    strike_range: list[float],
    sigma_range: list[float],
    S: float,
    T: float,
    r: float,
    q: float,
) -> Optional[str]:
    inputs_tuple: tuple = (tuple(strike_range), tuple(sigma_range), S, T, r, q)
    cache_key: str = hashlib.sha256(str(inputs_tuple).encode("utf-8")).hexdigest()
    cached_greeks: Optional[np.ndarray[np.float64]] = CACHE.get(cache_key)

    if cached_greeks is not None:
        return cache_key

    # Define the range of values for varying pricing parameters
    strike_arr: np.ndarray[np.float64] = np.linspace(strike_range[0], strike_range[1], GRID_RESOLUTION)
    sigma_arr: np.ndarray[np.float64] = np.linspace(sigma_range[0], sigma_range[1], GRID_RESOLUTION)

    # Call the C++ pricing engine
    try:
        # pricing.calculate_greeks_grid returns a flat Python list of doubles
        full_greeks_array: np.ndarray[np.float64] = pricing.calculate_greeks_grid(S, strike_arr, r, q, sigma_arr, T)

        if full_greeks_array.dtype != np.float64:
            full_greeks_array = full_greeks_array.astype(np.float64)

        CACHE.set(cache_key, full_greeks_array)
        return cache_key

    except Exception as e:
        print(f"Error communicating with pricing engine: {e}")
        return None

@APP.callback(
    Output("heatmap_ac", "figure"),
    Output("heatmap_ap", "figure"),
    Output("heatmap_ec", "figure"),
    Output("heatmap_ep", "figure"),
    [
        Input("greek_selector", "value"),
        Input("greeks_cache", "data"),
    ],
    [
        State("strike_range", "value"),
        State("sigma_range", "value"),
    ]
)
def update_heatmaps(
    greek_idx: int,
    full_greeks_cache_key: str | None,
    strike_range: list[float],
    sigma_range: list[float],
) -> tuple[Figure, Figure, Figure, Figure]:
    # Define the range of values for varying pricing parameters (needed for axis labels/error plotting)
    strike_arr: np.ndarray[np.float64] = np.linspace(strike_range[0], strike_range[1], GRID_RESOLUTION)
    sigma_arr: np.ndarray[np.float64] = np.linspace(sigma_range[0], sigma_range[1], GRID_RESOLUTION)
    full_greeks_array: Optional[np.ndarray[np.float64]] = None

    if full_greeks_cache_key is not None:
        full_greeks_array = CACHE.get(full_greeks_cache_key)

    # --- Check if the cached data is available
    if full_greeks_array is None:
        # Return four error figures
        error_figure: Figure = generate_heatmap_figure(
            np.zeros((GRID_RESOLUTION, GRID_RESOLUTION)),
            strike_arr,
            sigma_arr,
            "ERROR: Calculation Pending or Failed",
            "Price",
        )
        return tuple(error_figure for _ in range(len(OPTION_TYPES)))

    # --- Convert the flat list back into a 4D numpy array (sigma x strike x option type x greek type)
    try:
        data_size: int = GRID_RESOLUTION * GRID_RESOLUTION * len(OPTION_TYPES) * len(GREEK_TYPES)

        if full_greeks_array.size != data_size:
            raise ValueError("Cached list size does not match expected grid dimensions.")

        full_greeks_grid: np.ndarray[np.float64] = (
            full_greeks_array.reshape(GRID_RESOLUTION, GRID_RESOLUTION, len(OPTION_TYPES), len(GREEK_TYPES))
        )

    except Exception as e:
        print(f"Error reshaping cached data: {e}")
        error_figure: Figure = generate_heatmap_figure(
            np.zeros((GRID_RESOLUTION, GRID_RESOLUTION)),
            strike_arr,
            sigma_arr,
            "ERROR: Data Conversion Failed",
            "Price",
        )
        return tuple(error_figure for _ in range(len(OPTION_TYPES)))

    # Slice the 4D grid to get the selected greek for all options (Grid shape: [sigma, strike, option type, greek type])
    selected_greek_grid: np.ndarray[np.float64] = full_greeks_grid[:, :, :, greek_idx]

    # Determine the min/max value globally across the selected Greek to unify the color scale
    z_min: float = selected_greek_grid.min()
    z_max: float = selected_greek_grid.max()
    color_range: list[float] = [z_min, z_max]

    # Get the descriptive name for the selected Greek
    greek_name: str = GREEK_TYPES.get(greek_idx, "Price")

    # Slice the 3D Greek grid and generate the four figures
    return tuple(
        generate_heatmap_figure(
            selected_greek_grid[:, :, option_type_idx],
            strike_arr,
            sigma_arr,
            OPTION_TYPES[option_type_idx],
            greek_name,
            color_range
        )
        for option_type_idx in range(len(OPTION_TYPES))
    )


if __name__ == "__main__":
    APP.run()
