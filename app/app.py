import dash
import pricing
import numpy as np
import plotly.express as px
from dash import dcc, html
from dash.dependencies import Input, Output
from plotly.graph_objects import Figure
from styles import (
    COLOR_ACCENT,
    COLOR_BG_PRIMARY,
    COLOR_BG_SECONDARY,
    COLOR_TEXT_DEFAULT,
    COLOR_SLIDER_MARK,
    PADDING_MD,
    GAP_LG,
    BORDER_STYLE,
    CONTROL_PANEL_WIDTH,
    GRID_ITEM_SIZE,
    FONT_SIZE_XL,
    FONT_SIZE_LG,
    FONT_SIZE_MD,
    FONT_SIZE_SM,
    FONT_SIZE_XS,
    MARGIN_PLOT,
    FONT_PLOT_LBL,
    FONT_PLOT_TICK,
    FONT_PLOT_BAR_TITLE,
    FONT_PLOT_BAR_THICKNESS,
)
from typing import Optional

# Defines the resolution of the strike/volatility grid (e.g., 10x10)
GRID_RESOLUTION: int = 10

# Initialize the Dash application using a dark mode theme
APP: dash.Dash = dash.Dash(
    __name__,
    external_stylesheets=["https://cdn.jsdelivr.net/npm/bootswatch@5.3.3/dist/darkly/bootstrap.min.css"]
)

# Helper function to create a standardized input field for fixed parameters (S, r, T, q)
def create_input_group(label: str, id_name: str, value: float | int, min_val: float | int, max_val: float | int,
                       step: float | int) -> html.Div:
    return html.Div(
        # Flexbox ensures the label and input are aligned horizontally
        style={"display": "flex", "align-items": "center", "margin-bottom": "15px"},
        children=[
            html.Label(
                label,
                style={"flex": GRID_ITEM_SIZE + " 0 50%", "padding-right": "15px", "font-weight": "bold"}
            ),
            html.Div(
                style={"flex": GRID_ITEM_SIZE + " 0 50%"},
                children=dcc.Input(
                    id=id_name,
                    type="number",
                    value=value,
                    min=min_val,
                    max=max_val,
                    step=step,
                    debounce=True,  # Critical: Prevents heavy callbacks until the user stops typing
                    style={
                        "width": "100%",
                        "backgroundColor": COLOR_BG_SECONDARY,
                        "color": COLOR_TEXT_DEFAULT,
                        "border": BORDER_STYLE
                    }
                )
            )
        ]
    )

# Function to generate a Plotly heatmap figure
def generate_heatmap_figure(options_grid: np.ndarray, K_arr: np.ndarray, sigma_arr: np.ndarray,
                            title_label: str, color_range: Optional[list[float]] = None) -> Figure:
    """Generates a standardized heatmap figure with custom styling and text overlay."""
    # Create the Plotly figure
    fig: Figure = px.imshow(
        options_grid,
        x=K_arr,
        y=sigma_arr,
        origin="lower",  # Sets the origin for correct display of the grid data (volatilty increases along y-axis)
        text_auto=False,
        labels=dict(x="Strike (K)", y="Volatility (\u03C3)", color="Price"),
        aspect="auto",
        title=title_label,
        color_continuous_scale="thermal",
        # Use global color range if provided for consistent coloring across plots
        zmin=color_range[0] if color_range else None,
        zmax=color_range[1] if color_range else None,
    )

    # Set plot theme/layout configurations
    fig.update_layout(
        title_font_size=FONT_SIZE_LG,
        plot_bgcolor=COLOR_BG_SECONDARY,
        paper_bgcolor=COLOR_BG_SECONDARY,
        font=dict(color=COLOR_TEXT_DEFAULT, size=FONT_PLOT_TICK),
        margin=MARGIN_PLOT
    )

    # Add price labels directly onto the heatmap cells and define hover text format
    fig.update_traces(
        text=[[f"{val:.2f}" for val in row] for row in options_grid],
        texttemplate="%{text}",
        hovertemplate="Strike: %{x:.2f}<br>Volatility: %{y:.3f}<br>Price: %{z:.2f}<extra></extra>",
        textfont=dict(size=FONT_PLOT_LBL, family="Arial")
    )

    # Add axes and prevent panning off the heatmap
    fig.update_xaxes(side="bottom", tickformat=".0f", title_font=dict(size=FONT_PLOT_LBL), fixedrange=True)
    fig.update_yaxes(tickformat=".2f", title_font=dict(size=FONT_PLOT_LBL), fixedrange=True)
    fig.update_layout(
        coloraxis_colorbar=dict(
            title="Price",
            thickness=FONT_PLOT_BAR_THICKNESS,
            len=0.9,
            title_font=dict(size=FONT_PLOT_BAR_TITLE),
            tickfont=dict(size=FONT_PLOT_TICK)
        )
    )
    return fig


# Layout Definition
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
                # Input Controls (left column)
                html.Div(
                    style={"flex": "0 0 " + CONTROL_PANEL_WIDTH},
                    children=[
                        # Styled container for the inputs
                        html.Div(
                            style={
                                "border": BORDER_STYLE,
                                "borderRadius": "4px",
                                "overflow": "hidden", "position": "sticky", "top": PADDING_MD
                            },
                            children=[
                                # Card (input menu container) header
                                html.Div(
                                    html.H4(
                                        "Pricing Model Inputs",
                                        style={"padding": "15px " + PADDING_MD, "margin": 0, "font-size": FONT_SIZE_XL}
                                    ),
                                    style={
                                        "backgroundColor": COLOR_BG_PRIMARY,
                                        "borderBottom": BORDER_STYLE
                                    }
                                ),
                                # Card body with all the inputs
                                html.Div(
                                    style={"padding": "25px"},
                                    children=[
                                        html.H5(
                                            "Variable Ranges (Heatmap Axes)",
                                            style={
                                                "marginTop": "10px",
                                                "marginBottom": PADDING_MD,
                                                "color": COLOR_ACCENT,
                                                "font-size": FONT_SIZE_MD
                                            }
                                        ),
                                        html.Label(
                                            "Strike Range (K)",
                                            style={"display": "block", "font-weight": "bold"}
                                        ),
                                        dcc.RangeSlider(
                                            id="strike_range", min=50, max=150, step=0.1, value=[80, 120],
                                            # Custom marks for the slider
                                            marks={i: {
                                                "label": str(i),
                                                "style": {"color": COLOR_SLIDER_MARK, "font-size": FONT_SIZE_XS}
                                            }
                                                for i in range(50, 151, 20)},
                                            tooltip={"placement": "bottom", "always_visible": True}
                                        ),
                                        html.Br(),
                                        html.Label(
                                            "Volatility Range (\u03C3)",
                                            style={"display": "block", "font-weight": "bold"}
                                        ),
                                        dcc.RangeSlider(
                                            id="vol_range", min=0.01, max=1.0, step=0.005, value=[0.1, 0.4],
                                            # Custom marks for the volatility slider
                                            marks={i / 100: {
                                                "label": f"{i / 100:.2f}",
                                                "style": {"color": COLOR_SLIDER_MARK, "font-size": FONT_SIZE_XS}
                                            }
                                                for i in range(10, 101, 20)},
                                            tooltip={"placement": "bottom", "always_visible": True}
                                        ),
                                        html.Br(),
                                        html.Hr(style={"margin": GAP_LG + " 0"}),

                                        # Fixed parameters inputs
                                        html.H5(
                                            "Fixed Parameters",
                                            style={
                                                "color": COLOR_ACCENT,
                                                "margin-bottom": PADDING_MD,
                                                "font-size": FONT_SIZE_MD
                                            }
                                        ),
                                        create_input_group(
                                            label="Spot Price (S):",
                                            id_name="input_S",
                                            value=100.0,
                                            min_val=0.01,
                                            max_val=10000.0,
                                            step=0.01
                                        ),
                                        create_input_group(
                                            label="Maturity (T) (years):",
                                            id_name="input_T",
                                            value=1.0,
                                            min_val=0.01,
                                            max_val=5.0,
                                            step=0.01
                                        ),
                                        create_input_group(
                                            label="Risk-Free Rate (r):",
                                            id_name="input_r",
                                            value=0.05,
                                            min_val=-0.05,
                                            max_val=0.5,
                                            step=0.0001
                                        ),
                                        create_input_group(
                                            label="Dividend Yield (q):",
                                            id_name="input_q",
                                            value=0.02,
                                            min_val=0.0,
                                            max_val=0.5,
                                            step=0.0001
                                        ),
                                        html.Hr(style={"margin": GAP_LG + " 0"}),
                                        # Display block for current fixed parameters (keep for validation)
                                        html.Div(
                                            style={
                                                "padding": "15px",
                                                "borderRadius": "8px",
                                                "backgroundColor": COLOR_BG_PRIMARY,
                                                "border": "2px solid " + COLOR_ACCENT,
                                                "boxShadow": "0 4px 8px rgba(0, 0, 0, 0.2)",
                                            },
                                            children=[
                                                html.Strong(
                                                    "Active Fixed Parameters:",
                                                    style={
                                                        "color": COLOR_ACCENT,
                                                        "display": "block",
                                                        "margin-bottom": "10px",
                                                        "font-size": "1.05rem"
                                                    }
                                                ),
                                                # Output location for the parameter summary text
                                                html.Span(
                                                    id="param_summary",
                                                    style={"font-family": "monospace", "font-size": FONT_SIZE_SM}
                                                )
                                            ]
                                        )
                                    ]
                                )
                            ]
                        )
                    ]
                ),

                # Grid of heat mpas (right column)
                html.Div(
                    style={"flex": "1"},
                    children=dcc.Loading(
                        id="loading-heatmap",
                        type="cube",
                        style={"height": "100%", "display": "flex", "alignItems": "center", "justifyContent": "center"},
                        children=html.Div(
                            # Container modified for dynamic height using Flexbox
                            style={
                                "border": BORDER_STYLE,
                                "borderRadius": "4px",
                                "overflow": "hidden",
                                "width": "100%",
                                "display": "flex",
                                "flexDirection": "column",
                                "height": "100%"
                            },
                            children=[
                                # Plot header
                                html.Div(
                                    html.H4(
                                        "Option Price Heatmap Comparison (K vs. \u03C3)",
                                        style={"padding": "15px " + PADDING_MD, "margin": 0, "font-size": FONT_SIZE_XL}
                                    ),
                                    style={
                                        "backgroundColor": COLOR_BG_PRIMARY,
                                        "borderBottom": BORDER_STYLE
                                    }
                                ),
                                # 2x2 grid container (flexGrow: 1 ensures it fills the available vertical space)
                                html.Div(
                                    style={"display": "flex", "flex-wrap": "wrap", "width": "100%", "flexGrow": "1"},
                                    children=[
                                        # Each graph takes 50% width and 50% height of the container
                                        dcc.Graph(
                                            id="heatmap_ac",
                                            style={
                                                "width": GRID_ITEM_SIZE,
                                                "height": GRID_ITEM_SIZE,
                                                "borderBottom": BORDER_STYLE,
                                                "borderRight": BORDER_STYLE
                                            },
                                            config={'displaylogo': False}  # disable "Produced by plotly.js"
                                        ),
                                        dcc.Graph(
                                            id="heatmap_ap",
                                            style={
                                                "width": GRID_ITEM_SIZE,
                                                "height": GRID_ITEM_SIZE,
                                                "borderBottom": BORDER_STYLE
                                            },
                                            config={'displaylogo': False}
                                        ),
                                        dcc.Graph(
                                            id="heatmap_ec",
                                            style={
                                                "width": GRID_ITEM_SIZE,
                                                "height": GRID_ITEM_SIZE,
                                                "borderRight": BORDER_STYLE
                                            },
                                            config={'displaylogo': False}
                                        ),
                                        dcc.Graph(
                                            id="heatmap_ep",
                                            style={"width": GRID_ITEM_SIZE, "height": GRID_ITEM_SIZE},
                                            config={'displaylogo': False}
                                        ),
                                    ]
                                )
                            ]
                        )
                    )
                )
            ]
        ),
    ]
)

# Callback: Dynamically adjusts strike range based on spot price (S) input
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
        mark_interval = magnitude * 1.0
    elif raw_interval / magnitude <= 3.5:
        mark_interval = magnitude * 2.0
    elif raw_interval / magnitude <= 7.5:
        mark_interval = magnitude * 5.0
    else:
        mark_interval = magnitude * 10.0

    mark_interval: float = max(1.0, mark_interval)
    marks: dict[float, dict[str, str | dict[str, str]]] = {}
    start_mark: float = np.ceil(strike_min / mark_interval) * mark_interval  # first clean mark >= to strike_min

    # Generate the actual marks dictionary
    for i in np.arange(start_mark, strike_max + 0.001, mark_interval):
        label: str = f"{i:,.0f}" if i == int(i) else f"{i:,.2f}"
        marks[i] = {"label": label, "style": {"color": COLOR_SLIDER_MARK, "font-size": FONT_SIZE_XS}}

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
    """Updates the text summary inside the left control panel."""

    # Input validation: Checks if Dash returned None (due to input being out of min/max range)
    if S is None or T is None or r is None or q is None:
        return html.Pre("S = ERROR\nT = ERROR\nr = ERROR\nq = ERROR\n\n(Input out of configured range)")

    # Format and return the parameters using pre-formatted text
    return html.Pre(f"S = ${S:,.2f}\nT = {T:.2f} years\nr = {r:.2%}\nq = {q:.2%}")

# Callback: Fetches data and updates all four heatmaps
@APP.callback(
    Output("heatmap_ac", "figure"),
    Output("heatmap_ap", "figure"),
    Output("heatmap_ec", "figure"),
    Output("heatmap_ep", "figure"),
    [
        Input("strike_range", "value"),
        Input("vol_range", "value"),
        Input("input_S", "value"),
        Input("input_T", "value"),
        Input("input_r", "value"),
        Input("input_q", "value"),
    ]
)
def update_heatmaps(strike_range: list[float], vol_range: list[float], S: float, T: float, r: float,
                    q: float) -> tuple[Figure, Figure, Figure, Figure]:
    # 1. Define the range of values for varying pricing parameters
    K_arr: np.ndarray = np.linspace(strike_range[0], strike_range[1], GRID_RESOLUTION)
    sigma_arr: np.ndarray = np.linspace(vol_range[0], vol_range[1], GRID_RESOLUTION)

    # 2. Call the C++ pricing engine and get all four option grids
    try:
        full_price_grid: np.ndarray = pricing.price_grids(S, K_arr, r, q, sigma_arr, T)
        # Determine the min/max price globally to unify the color scale across all four charts
        z_min: float = float(full_price_grid.min())
        z_max: float = float(full_price_grid.max())
        color_range: list[float] = [z_min, z_max]

    except Exception as e:
        print(f"Error communicating with pricing engine: {e}")
        # Return four empty/error figures on failure
        error_figure: Figure = generate_heatmap_figure(
            np.zeros((GRID_RESOLUTION, GRID_RESOLUTION)),
            K_arr,
            sigma_arr,
            "ERROR: Could not connect to C++ pricing engine."
        )
        return error_figure, error_figure, error_figure, error_figure

    # 3. Slice the 3D grid and generate the four Plotly figures
    fig_ac: Figure = generate_heatmap_figure(
        full_price_grid[:, :, 0],
        K_arr,
        sigma_arr,
        "American Call Option Price",
        color_range
    )
    fig_ap: Figure = generate_heatmap_figure(
        full_price_grid[:, :, 1],
        K_arr,
        sigma_arr,
        "American Put Option Price",
        color_range
    )
    fig_ec: Figure = generate_heatmap_figure(
        full_price_grid[:, :, 2],
        K_arr,
        sigma_arr,
        "European Call Option Price",
        color_range
    )
    fig_ep: Figure = generate_heatmap_figure(
        full_price_grid[:, :, 3],
        K_arr,
        sigma_arr,
        "European Put Option Price",
        color_range
    )

    return fig_ac, fig_ap, fig_ec, fig_ep


if __name__ == "__main__":
    APP.run()
