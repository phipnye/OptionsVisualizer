import dash_bootstrap_components as dbc
import numpy as np
import plotly.express as px
from dash import dcc, html
from constants import (
    greek_enum,
    GREEK_SYMBOLS,
    GREEK_TYPES,
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
                        options=[
                            {"label": GREEK_SYMBOLS[name], "value": str(val)} for val, name in GREEK_TYPES.items()
                        ],
                        value=str(greek_enum.Price.value)  # dbc select gives str values only
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
                                        id="input_spot",
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
        className="h-100",  # fill parent column height

        children=[
            dbc.CardBody(
                className="d-flex flex-column h-100",
                style={"minHeight": 0},  # allow flex children to shrink

                children=[
                    # fixed-height header
                    html.Div(
                        "Black-Scholes & Trinomial Option Pricing Engine Dashboard",
                        className="text-center p-2"
                    ),

                    # main plot area
                    html.Div(
                        className="flex-grow-1",     # take remaining space
                        style={"minHeight": 0},      # avoid Plotly overflow bug

                        children=[
                            dcc.Loading(
                                className="h-100",  # spinner fills plot area

                                children=[
                                    dbc.Row(
                                        className="h-50",  # top half

                                        children=[
                                            dbc.Col(
                                                dcc.Graph(
                                                    id="heatmap_ac",
                                                    style={"height": "100%"}  # let Plotly fill container
                                                ),
                                                md=6,
                                                className="h-100"
                                            ),
                                            dbc.Col(
                                                dcc.Graph(
                                                    id="heatmap_ap",
                                                    style={"height": "100%"}
                                                ),
                                                md=6,
                                                className="h-100"
                                            ),
                                        ],
                                    ),

                                    dbc.Row(
                                        className="h-50",  # bottom half

                                        children=[
                                            dbc.Col(
                                                dcc.Graph(
                                                    id="heatmap_ec",
                                                    style={"height": "100%"}
                                                ),
                                                md=6,
                                                className="h-100"
                                            ),
                                            dbc.Col(
                                                dcc.Graph(
                                                    id="heatmap_ep",
                                                    style={"height": "100%"}
                                                ),
                                                md=6,
                                                className="h-100"
                                            ),
                                        ],
                                    ),
                                ],
                            )
                        ],
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
    font_size: int = 14,
    color_range: Optional[tuple[np.float64]] = None
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
        height=None,  # let CSS control height (required for flex layouts)
        coloraxis_colorbar=dict(
            title=None,
            thickness=10  # colorbar thickness
        ),
        title=dict(x=0.5, xanchor="center")
    )
    return fig
