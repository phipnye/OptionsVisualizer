import dash
import hashlib
import pricing
import dash_bootstrap_components as dbc
import numpy as np
from constants import (
    GREEK_TYPES,
    GRID_RESOLUTION,
    OPTION_TYPES,
)
from dash import dcc, html
from dash.dependencies import Input, Output, State
from dash_bootstrap_templates import load_figure_template
from flask_caching import Cache
from helpers import (
    create_control_panel,
    create_heatmap_grid,
    generate_heatmap_figure,
    generate_ranges
)
from plotly.graph_objects import Figure
from typing import Optional

# Initialize the Dash application using a dark mode theme
APP: dash.Dash = dash.Dash(
    __name__,
    external_stylesheets=[dbc.themes.DARKLY]
)

CACHE: Cache = Cache(APP.server, config={"CACHE_TYPE": "simple", "CACHE_DEFAULT_TIMEOUT": 3600})

# Loads the "darkly" template and sets it as the default
load_figure_template("darkly")

# --- Layout definition
APP.layout = dbc.Container(
    fluid=True,
    style={"height": "100vh"},  # set height equal to 100% of viewport height
    children=[
        dbc.Row(
            className="flex-grow-1 mt-1",  # make the flex item grow to fill available space within the flex container
            style={"minHeight": 0},
            children=[
                # Left control panel
                dbc.Col(
                    create_control_panel(),
                    md=3,  # 25% width on medium+ screens
                    style={"height": "100%"}
                ),

                # Right heatmap grid
                dbc.Col(
                    create_heatmap_grid(),
                    md=9,  # 75% width on medium+ screens
                    style={"height": "100%"}
                ),
            ],
        ),
        # Cache greek results (prevent re-computing when switiching greek type for the same model parameters)
        dcc.Store(id="greeks_cache", storage_type="memory"),
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
    magnitude: np.float64 = 10 ** np.floor(np.log10(raw_interval))

    # Determine the cleanest interval (1, 2, 5, 10, etc. times the magnitude)
    if raw_interval / magnitude <= 1.5:
        mark_interval: np.float64 = magnitude * 1.0
    elif raw_interval / magnitude <= 3.5:
        mark_interval = magnitude * 2.0
    elif raw_interval / magnitude <= 7.5:
        mark_interval = magnitude * 5.0
    else:
        mark_interval = magnitude * 10.0

    mark_interval: np.float64 = max(np.float64(1.0), mark_interval)
    marks: dict[float, dict[str, str | dict[str, str]]] = {}
    start_mark: float = np.ceil(strike_min / mark_interval) * mark_interval  # first clean mark >= strike_min

    # Generate the actual marks dictionary
    for i in np.arange(start_mark, strike_max + 0.001, mark_interval):
        label: str = f"{i:,.0f}" if i == int(i) else f"{i:,.2f}"
        marks[float(i)] = {"label": label, "style": {"color": "#9C9C9C", "fontSize": "0.75rem"}}

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

    if CACHE.get(cache_key) is not None:
        return cache_key

    # Call the C++ pricing engine
    try:
        # Define the range of values for varying pricing parameters
        sigmas_arr: np.ndarray[np.float64]
        strikes_arr: np.ndarray[np.float64]
        sigmas_arr, strikes_arr = generate_ranges(sigma_range, strike_range)

        # Construct C++ grid object
        grid: pricing.Grid = pricing.Grid(
            spot=S,
            strikes_arr=strikes_arr,
            r=r,
            q=q,
            sigmas_arr=sigmas_arr,
            tau=T
        )

        # Calculate greeks across our grid of volatilities and strikes (output is a flat array)
        flat_greeks_array: np.ndarray[np.float64] = grid.calculate_grids()

        # Reshape flat output array sigma x strike x option type x greek type
        full_greeks_array: np.ndarray[np.float64] = flat_greeks_array.reshape(
            GRID_RESOLUTION,
            GRID_RESOLUTION,
            len(OPTION_TYPES),
            len(GREEK_TYPES),
            order="F",  # retain the column-major memory layout of the returned object
            copy=False  # prevent copying
        )

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
    greek_idx: str,
    full_greeks_cache_key: str | None,
    strike_range: list[float],
    sigma_range: list[float]
) -> tuple[Figure, Figure, Figure, Figure]:
    # Define the range of values for varying pricing parameters (needed for axis labels/error plotting)
    sigmas_arr: np.ndarray[np.float64]
    strikes_arr: np.ndarray[np.float64]
    sigmas_arr, strikes_arr = generate_ranges(sigma_range, strike_range)
    full_greeks_grid: Optional[np.ndarray[np.float64]] = None

    if full_greeks_cache_key is not None:
        full_greeks_grid = CACHE.get(full_greeks_cache_key)

    # --- Check if the cached data is available
    if full_greeks_grid is None:
        # Return four error figures
        error_figure: Figure = generate_heatmap_figure(
            np.zeros((GRID_RESOLUTION, GRID_RESOLUTION)),
            strikes_arr,
            sigmas_arr,
            "ERROR: Calculation Pending or Failed",
            "Price",
        )
        return tuple(error_figure for _ in range(len(OPTION_TYPES)))

    # Slice the 4D grid to get the selected greek for all options (Grid shape: [sigma, strike, option type, greek type])
    selected_greek_grid: np.ndarray[np.float64] = full_greeks_grid[:, :, :, int(greek_idx)]

    # Determine the min/max value globally across the selected Greek to unify the color scale
    z_min: np.float64 = selected_greek_grid.min()
    z_max: np.float64 = selected_greek_grid.max()
    color_range: list[np.float64] = [z_min, z_max]

    # Get the descriptive name for the selected Greek
    greek_name: str = GREEK_TYPES.get(greek_idx, "Price")

    # Slice the 3D Greek grid and generate the four figures
    return tuple(
        generate_heatmap_figure(
            selected_greek_grid[:, :, option_type_idx],
            strikes_arr,
            sigmas_arr,
            OPTION_TYPES[option_type_idx],
            greek_name,
            color_range
        )
        for option_type_idx in range(len(OPTION_TYPES))
    )


if __name__ == "__main__":
    APP.run()
