import dash
import options_surface
import dash_bootstrap_components as dbc
import numpy as np
from constants import (
    greek_enum,
    GREEK_TYPES,
    GRID_RESOLUTION,
    OPTION_TYPES
)
from dash import html
from dash.dependencies import Input, Output, State
from dash_bootstrap_templates import load_figure_template
from helpers import (
    create_control_panel,
    create_heatmap_grid,
    generate_heatmap_figure
)
from plotly.graph_objects import Figure

# --- Setup

# Initialize the Dash application using a dark mode theme
APP: dash.Dash = dash.Dash(__name__, external_stylesheets=[dbc.themes.DARKLY])

# C++ cache object to retrieving greeks results (capacity dictates number of arrays we can store: capacity x number of
# sigmas x number of strikes x number of greeks * number of option types)
OPTIONS_MANAGER: options_surface.OptionsManager = options_surface.OptionsManager(capacity=16)

# Loads the "darkly" template and sets it as the default
load_figure_template("darkly")

# Define app layout (left column for control panel, right column for heatmaps)
APP.layout = dbc.Container(
    fluid=True,
    className="vh-100 d-flex flex-column",  # full viewport height + enable vertical flex sizing
    children=[
        dbc.Row(
            className="flex-grow-1",  # allow row to expand vertically
            children=[
                # Control panel (left column)
                dbc.Col(
                    create_control_panel(),
                    md=3,  # 25% width on md+
                    className="h-100"
                ),
                # Heatmaps (right column)
                dbc.Col(
                    create_heatmap_grid(),
                    md=9,  # 75% width on md+
                    className="h-100"
                ),
            ],
        )
    ]
)


# Dynamically adjusts strike range based on spot price input
@APP.callback(
    Output("strike_range", "min"),
    Output("strike_range", "max"),
    Output("strike_range", "step"),
    Output("strike_range", "value"),
    Output("strike_range", "marks"),
    Input("input_spot", "value")
)
def update_strike_and_spot(stock_price: float) -> tuple[float, float, float, list[float], dict]:
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
        mark_interval: np.float64 = magnitude
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

# Updates the text summary of fixed parameters
@APP.callback(
    Output("param_summary", "children"),
    Input("input_spot", "value"),
    Input("input_T", "value"),
    Input("input_r", "value"),
    Input("input_q", "value"),
)
def update_param_summary(S: float, T: float, r: float, q: float) -> html.Pre:
    # Input validation: Checks if Dash returned None (due to input being out of min/max range)
    if S is None or T is None or r is None or q is None:
        return html.Pre("S = ERROR\nT = ERROR\nr = ERROR\nq = ERROR\n\n(Input out of configured range)")

    # Format and return the parameters using pre-formatted text
    return html.Pre(f"S = ${S:,.2f}\nT = {T:.2f} years\nr = {r:.2%}\nq = {q:.2%}")

# Update heatmap plots
@APP.callback(
    Output("heatmap_ac", "figure"),
    Output("heatmap_ap", "figure"),
    Output("heatmap_ec", "figure"),
    Output("heatmap_ep", "figure"),
    Input("greek_selector", "value"),
    State("strike_range", "value"),
    State("sigma_range", "value"),
    Input("input_spot", "value"),
    Input("input_T", "value"),
    Input("input_r", "value"),
    Input("input_q", "value"),
)
def update_heatmaps(
    greek_idx_str: str,
    strike_range: list[float],
    sigma_range: list[float],
    spot: float,
    tau: float,
    r: float,
    q: float,
) -> tuple[Figure, Figure, Figure, Figure]:
    # Convert the string from Dash Select to an int
    greek_idx: int = int(greek_idx_str)

    # Map the Greek name for the UI
    greek_name: str = GREEK_TYPES.get(greek_idx, "Price")

    # Define the range of values for varying pricing parameters (needed for axis labels/error plotting)
    sigmas_arr: np.ndarray[np.float64] = options_surface.linspace(GRID_RESOLUTION, sigma_range[0], sigma_range[1])
    strikes_arr: np.ndarray[np.float64] = options_surface.linspace(GRID_RESOLUTION, strike_range[0], strike_range[1])

    try:
        # Call C++ pricing engine (returns 4 matrices)
        grids: tuple[np.ndarray[np.float64], ...] = OPTIONS_MANAGER.get_greek(
            greek_enum(greek_idx),
            GRID_RESOLUTION,
            GRID_RESOLUTION,
            spot,
            r,
            q,
            sigma_range[0],
            sigma_range[1],
            strike_range[0],
            strike_range[1],
            tau
        )

    except Exception as e:
        print(f"Pricing error: {e}")
        error_fig: Figure = generate_heatmap_figure(
            np.zeros((GRID_RESOLUTION, GRID_RESOLUTION)),
            strikes_arr,
            sigmas_arr,
            "ERROR",
            "Price",
        )
        return (error_fig,) * len(OPTION_TYPES)

    # Determine global color scale
    z_min: np.float64 = min(grid.min() for grid in grids)
    z_max: np.float64 = max(grid.max() for grid in grids)
    color_range: tuple[np.float64, np.float64] = (z_min, z_max)

    return tuple(
        generate_heatmap_figure(
            grids[i],
            strikes_arr,
            sigmas_arr,
            OPTION_TYPES[i],
            greek_name,
            color_range=color_range
        )
        for i in range(len(OPTION_TYPES))
    )


if __name__ == "__main__":
    APP.run()
