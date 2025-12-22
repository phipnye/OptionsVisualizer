import dash_bootstrap_components as dbc
import numpy as np
from config import SETTINGS
from dash import dcc, html
from mappings import GREEK_ENUM, GREEK_TYPES, OPTION_TYPES


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
                            {"label": entry.display_name, "value": str(idx)} for idx, entry in GREEK_TYPES.items()
                        ],
                        value=str(GREEK_ENUM.Price.value),  # dbc select gives str values only (default to price)
                    ),
                    html.Hr(),
                    # --- Variable ranges
                    dbc.Label("Strike Range"),
                    dcc.RangeSlider(
                        id="strike_range",
                        min=SETTINGS.SPOT_DEFAULT * SETTINGS.STRIKE_RANGE_PCTS[0],  # bounds driven by spot value
                        max=SETTINGS.SPOT_DEFAULT * SETTINGS.STRIKE_RANGE_PCTS[1],
                        step=SETTINGS.STRIKE_STEP,
                        value=[SETTINGS.SPOT_DEFAULT * pct for pct in SETTINGS.STRIKE_VAL_PCTS],
                        tooltip={"placement": "bottom", "always_visible": True},
                    ),
                    dbc.Label("Volatility Range (\u03c3)"),
                    dcc.RangeSlider(
                        id="sigma_range",
                        min=SETTINGS.SIGMA_MIN,
                        max=SETTINGS.SIGMA_MAX,
                        step=SETTINGS.SIGMA_STEP,
                        value=SETTINGS.SIGMA_DEFAULTS,
                        marks={float(x): f"{x:.02f}" for x in np.linspace(SETTINGS.SIGMA_MIN, SETTINGS.SIGMA_MAX, 10)},
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
                                        value=SETTINGS.SPOT_DEFAULT,
                                        min=SETTINGS.SPOT_MIN,
                                        max=SETTINGS.SPOT_MAX,
                                        step=SETTINGS.SPOT_STEP,
                                        debounce=True,
                                    ),
                                ],
                                width=SETTINGS.HEATMAP_WIDTH,
                            ),
                            dbc.Col(
                                [
                                    dbc.Label("Time (Y)"),
                                    dbc.Input(
                                        id="input_tau",
                                        type="number",
                                        value=SETTINGS.TAU_DEFAULT,
                                        min=SETTINGS.TAU_MIN,
                                        max=SETTINGS.TAU_MAX,
                                        step=SETTINGS.TAU_STEP,
                                        debounce=True,
                                    ),
                                ],
                                width=SETTINGS.HEATMAP_WIDTH,
                            ),
                        ],
                    ),
                    dbc.Row(
                        [
                            dbc.Col(
                                [
                                    dbc.Label("Rate (r)"),
                                    dbc.Input(
                                        id="input_r",
                                        type="number",
                                        value=SETTINGS.RATE_DEFAULT,
                                        min=SETTINGS.RATE_MIN,
                                        max=SETTINGS.RATE_MAX,
                                        step=SETTINGS.RATE_STEP,
                                        debounce=True,
                                    ),
                                ],
                                width=SETTINGS.HEATMAP_WIDTH,
                            ),
                            dbc.Col(
                                [
                                    dbc.Label("Div (q)"),
                                    dbc.Input(
                                        id="input_q",
                                        type="number",
                                        value=SETTINGS.DIV_DEFAULT,
                                        min=SETTINGS.DIV_MIN,
                                        max=SETTINGS.DIV_MAX,
                                        step=SETTINGS.DIV_STEP,
                                        debounce=True,
                                    ),
                                ],
                                width=SETTINGS.HEATMAP_WIDTH,
                            ),
                        ]
                    ),
                    html.Br(),
                    # --- Parameter summary block
                    html.Div(
                        style={"border": f"2px solid {SETTINGS.SUMMARY_COLOR}"},  # border around summary box
                        children=[
                            html.Strong(
                                "Active Fixed Parameters:",
                                style={"color": SETTINGS.SUMMARY_COLOR, "display": "block"},
                            ),
                            html.Span(id="param_summary"),
                        ],
                    ),
                ]
            ),
        ]
    )


def create_heatmap_grid() -> dbc.Card:
    # Dynamically generate columns based on OPTION_TYPES metadata
    plot_cols: list[dbc.Col] = [
        dbc.Col(dcc.Graph(id=f"heatmap_{opt.id}", style={"height": "100%"}), md=6, className="h-100")
        for opt in OPTION_TYPES.values()
    ]

    # Split the columns into rows of 2 (ensures a 2x2 for 4 items)
    rows: list[dbc.Row] = [dbc.Row(plot_cols[i : i + 2], className="flex-grow-1") for i in range(0, len(plot_cols), 2)]

    return dbc.Card(
        className="h-100",
        children=[
            dbc.CardBody(
                className="d-flex flex-column h-100",
                style={"minHeight": 0},
                children=[
                    # Fixed-height header
                    html.Div("Black-Scholes & Trinomial Option Pricing Engine Dashboard", className="text-center"),
                    # Main plot area
                    html.Div(
                        className="flex-grow-1",
                        style={"minHeight": 0},
                        children=[dcc.Loading(children=rows, className="h-100")],
                    ),
                ],
            )
        ],
    )
