import dash_bootstrap_components as dbc
import numpy as np
import plotly.express as px
from dash import dcc, html
from constants import (
    GREEK_SYMBOLS,
    GREEK_TYPES,
    GRID_RESOLUTION
)
from plotly.graph_objects import Figure
from typing import Optional

def create_control_panel() -> dbc.Card:
    return dbc.Card(
        [
            dbc.CardHeader("Settings"),

            dbc.CardBody(
                [
                    # --- Greek selector
                    dbc.Label("Output Metric"),
                    dbc.Select(
                        id="greek_selector",
                        options=[{"label": GREEK_SYMBOLS[name], "value": idx} for idx, name in GREEK_TYPES.items()],
                        value='0'  # dbc select gives str values only
                    ),

                    html.Hr(),

                    # --- Variable ranges
                    dbc.Label("Strike Range"),
                    dcc.RangeSlider(
                        id="strike_range",
                        min=50,
                        max=150,
                        step=0.1,
                        value=[80, 120],
                        tooltip={"placement": "bottom", "always_visible": True},
                    ),

                    dbc.Label("Volatility Range (\u03c3)"),
                    dcc.RangeSlider(
                        id="sigma_range",
                        min=0.01,
                        max=1.0,
                        step=0.005,
                        value=[0.1, 0.4],
                        marks={x: f"{x:.02f}" for x in np.arange(0.0, 1.0, 0.1)},
                        tooltip={"placement": "bottom", "always_visible": True},
                    ),

                    html.Hr(),

                    # --- Fixed parameters (2x2 grid)
                    dbc.Label("Fixed Parameters"),

                    dbc.Row(
                        [
                            dbc.Col(
                                [
                                    dbc.Label("Spot ($)"),
                                    dbc.Input(
                                        id="input_S",
                                        type="number",
                                        value=100.0,
                                        min=0.01,
                                        max=10000,
                                        step=0.01,
                                        debounce=True
                                    ),
                                ],
                                width=6,
                            ),

                            dbc.Col(
                                [
                                    dbc.Label("Time (Y)"),
                                    dbc.Input(
                                        id="input_T",
                                        type="number",
                                        value=1.0,
                                        min=0.01,
                                        max=5.0,
                                        step=0.01,
                                        debounce=True
                                    ),
                                ],
                                width=6,
                            ),
                        ],
                        className="mb-2",
                    ),

                    dbc.Row(
                        [
                            dbc.Col(
                                [
                                    dbc.Label("Rate (r)"),
                                    dbc.Input(
                                        id="input_r",
                                        type="number",
                                        value=0.05,
                                        min=-0.05,
                                        max=0.5,
                                        step=0.0001,
                                        debounce=True
                                    ),
                                ],
                                width=6,
                            ),

                            dbc.Col(
                                [
                                    dbc.Label("Div (q)"),
                                    dbc.Input(
                                        id="input_q",
                                        type="number",
                                        value=0.02,
                                        min=0.0,
                                        max=0.5,
                                        step=0.0001,
                                        debounce=True
                                    ),
                                ],
                                width=6,
                            ),
                        ]
                    ),

                    html.Br(),

                    # --- Parameter summary block
                    html.Div(
                        style={
                            "padding": "15px",
                            "borderRadius": "8px",
                            "border": "2px solid " + "#00BC8C"
                        },
                        children=[
                            html.Strong(
                                "Active Fixed Parameters:",
                                style={
                                    "color": "#00BC8C",
                                    "display": "block",
                                    "marginBottom": "10px",
                                    "fontSize": "1.05rem"
                                }
                            ),
                            html.Span(id="param_summary")
                        ]
                    )
                ]
            ),
        ]
    )


def create_heatmap_grid() -> dbc.Card:
    return dbc.Card(
        className="h-100",  # allow the card to stretch to fill its parent column
        children=[
            dbc.CardBody(
                className="d-flex flex-column",  # stack contents vertically and enable flex sizing
                style={"height": "100%", "minHeight": 0},
                children=[
                    html.Div(
                        html.H4(
                            "Black-Scholes & Trinomial Option Pricing Engine Dashboard",
                            id="plot_header_title",
                            className="text-center mt-1 mb-3",  # apply vertical margins
                        ),
                        style={"flex": "0 0 auto"},  # title takes only the space it needs
                    ),

                    dcc.Loading(
                        type="circle",
                        style={"flex": "1 1 auto", "minHeight": 0},  # loading spinner fills remaining vertical space

                        # --- Top row of heatmaps grid
                        children=html.Div(
                            className="d-flex flex-column",
                            style={"height": "100%"},  # inherit full height from CardBody
                            children=[
                                html.Div(
                                    className="d-flex",
                                    style={"flex": 1},  # top row takes half of available height
                                    children=[
                                        dcc.Graph(
                                            id="heatmap_ac",
                                            style={"flex": 1},  # fill half the row width
                                            config={'displaylogo': False}  # remove created by plotly logo
                                        ),
                                        dcc.Graph(
                                            id="heatmap_ap",
                                            style={"flex": 1},
                                            config={'displaylogo': False}
                                        ),
                                    ],
                                ),

                                # --- Bottom row of heatmaps grid
                                html.Div(
                                    className="d-flex",
                                    style={"flex": 1},  # bottom row takes remaining half
                                    children=[
                                        dcc.Graph(
                                            id="heatmap_ec",
                                            style={"flex": 1},
                                            config={'displaylogo': False}
                                        ),
                                        dcc.Graph(
                                            id="heatmap_ep",
                                            style={"flex": 1},
                                            config={'displaylogo': False}
                                        ),
                                    ],
                                ),
                            ],
                        ),
                    )
                ],
            ),
        ],
    )


def generate_heatmap_figure(
    options_grid: np.ndarray[np.float64],
    strikes_arr: np.ndarray[np.float64],
    sigmas_arr: np.ndarray[np.float64],
    title_label: str,
    greek_label: str,
    color_range: Optional[list[float]] = None,
    font_size: int = 14
) -> Figure:
    """Generates a standardized heatmap figure with custom styling and text overlay."""
    color_label: str = GREEK_SYMBOLS.get(greek_label, greek_label)

    fig: Figure = px.imshow(
        options_grid,
        x=strikes_arr,
        y=sigmas_arr,
        origin="lower",
        text_auto=False,
        labels=dict(x="Strike (K)", y="Volatility (\u03C3)", color=color_label),
        aspect="auto",
        title=f"{title_label} {color_label}",
        color_continuous_scale="thermal",
        zmin=color_range[0] if color_range else None,
        zmax=color_range[1] if color_range else None,
    )

    fig.update_traces(
        text=[
            [f"{val:.2f}" if greek_label.lower() != "gamma" else f"{val:.4f}" for val in row] for row in options_grid
        ],
        texttemplate="%{text}",
        hovertemplate=f"Strike: %{{x:.2f}}<br>Volatility: %{{y:.3f}}<br>{color_label}: %{{z:.4f}}<extra></extra>",
        textfont=dict(size=font_size, family="Arial")
    )

    fig.update_xaxes(side="bottom", tickformat=".0f", title_font=dict(size=font_size), fixedrange=True)
    fig.update_yaxes(tickformat=".2f", title_font=dict(size=font_size), fixedrange=True)
    fig.update_layout(
        margin=dict(l=40, r=10, t=35, b=35),  # tighten internal spacing
        autosize=True,
        coloraxis_colorbar=dict(
            title=None,
            thickness=10  # colorbar thickness
        ),
        title=dict(x=0.5, xanchor="center")
    )
    return fig

def generate_ranges(
    sigma_range: list[float],
    strike_range: list[float]
) -> tuple[np.ndarray[np.float64], np.ndarray[np.float64]]:
    sigmas_arr: np.ndarray[np.float64] = np.linspace(sigma_range[0], sigma_range[1], GRID_RESOLUTION)
    strikes_arr: np.ndarray[np.float64] = np.linspace(strike_range[0], strike_range[1], GRID_RESOLUTION)
    return np.asfortranarray(sigmas_arr), np.asfortranarray(strikes_arr)  # column major order (for eigen computations)
