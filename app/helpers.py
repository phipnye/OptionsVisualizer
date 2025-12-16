import numpy as np
import plotly.express as px
from dash import dcc, html
from constants import (
    GREEK_SYMBOLS,
    GREEK_TYPES,
    GRID_RESOLUTION
)
from plotly.graph_objects import Figure
from styles import (
    BORDER_STYLE,
    COLOR_ACCENT,
    COLOR_BG_PRIMARY,
    COLOR_BG_SECONDARY,
    COLOR_SLIDER_MARK,
    COLOR_TEXT_DEFAULT,
    CONTROL_PANEL_WIDTH,
    FONT_PLOT_BAR_THICKNESS,
    FONT_PLOT_BAR_TITLE,
    FONT_PLOT_LBL,
    FONT_PLOT_TICK,
    FONT_SIZE_LG,
    FONT_SIZE_MD,
    FONT_SIZE_SM,
    FONT_SIZE_XL,
    FONT_SIZE_XS,
    GAP_LG,
    GRID_ITEM_SIZE,
    MARGIN_PLOT,
    PADDING_MD,
)
from typing import Optional

def create_control_panel() -> html.Div:
    """Creates the entire left control panel containing all inputs and summaries."""
    return html.Div(
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
                        children=html.H4(
                            "Pricing Model Inputs",
                            style={"padding": "15px " + PADDING_MD, "margin": 0, "fontSize": FONT_SIZE_XL}
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
                            # --- Greek selector dropdown
                            html.H5(
                                "Output Metric",
                                style={
                                    "marginTop": "10px",
                                    "marginBottom": PADDING_MD,
                                    "color": COLOR_ACCENT,
                                    "fontSize": FONT_SIZE_MD
                                }
                            ),
                            dcc.Dropdown(
                                id="greek_selector",
                                options=[
                                    {'label': GREEK_SYMBOLS[name], 'value': idx} for idx, name in GREEK_TYPES.items()
                                ],
                                value=0,  # Default to Price
                                searchable=False,
                                clearable=False,
                                style={
                                    "marginBottom": GAP_LG,
                                    "backgroundColor": COLOR_BG_SECONDARY,
                                    "color": COLOR_TEXT_DEFAULT,
                                    "border": BORDER_STYLE,
                                    "fontSize": FONT_SIZE_MD
                                }
                            ),

                            html.Hr(style={"margin": GAP_LG + " 0"}),

                            # --- Variable range inputs
                            html.H5(
                                "Variable Ranges (Heatmap Axes)",
                                style={
                                    "marginTop": "10px",
                                    "marginBottom": PADDING_MD,
                                    "color": COLOR_ACCENT,
                                    "fontSize": FONT_SIZE_MD
                                }
                            ),
                            html.Label(
                                "Strike Range (K)",
                                style={"display": "block", "fontWeight": "bold"}
                            ),
                            dcc.RangeSlider(
                                id="strike_range", min=50, max=150, step=0.1, value=[80, 120],
                                marks={i: {
                                    "label": str(i),
                                    "style": {"color": COLOR_SLIDER_MARK, "fontSize": FONT_SIZE_XS}
                                }
                                    for i in range(50, 151, 20)},
                                tooltip={"placement": "bottom", "always_visible": True}
                            ),
                            html.Br(),
                            html.Label(
                                "Volatility Range (\u03C3)",
                                style={"display": "block", "fontWeight": "bold"}
                            ),
                            dcc.RangeSlider(
                                id="sigma_range", min=0.01, max=1.0, step=0.005, value=[0.1, 0.4],
                                marks={i / 100: {
                                    "label": f"{i / 100:.2f}",
                                    "style": {"color": COLOR_SLIDER_MARK, "fontSize": FONT_SIZE_XS}
                                }
                                    for i in range(10, 101, 20)},
                                tooltip={"placement": "bottom", "always_visible": True}
                            ),
                            html.Br(),
                            html.Hr(style={"margin": GAP_LG + " 0"}),

                            # --- Fixed parameters inputs
                            html.H5(
                                "Fixed Parameters",
                                style={
                                    "color": COLOR_ACCENT,
                                    "marginBottom": PADDING_MD,
                                    "fontSize": FONT_SIZE_MD
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

                            # --- Parameter summary block
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
                                            "marginBottom": "10px",
                                            "fontSize": "1.05rem"
                                        }
                                    ),
                                    html.Span(
                                        id="param_summary",
                                        style={"fontFamily": "monospace", "fontSize": FONT_SIZE_SM}
                                    )
                                ]
                            )
                        ]
                    )
                ]
            )
        ]
    )

def create_heatmap_grid() -> html.Div:
    """Creates the right-hand container holding the 2x2 grid of heatmaps."""
    return html.Div(
        style={"flex": "1"},
        children=dcc.Loading(
            id="loading-heatmap",
            type="cube",
            style={"height": "100%", "display": "flex", "alignItems": "center", "justifyContent": "center"},
            children=html.Div(
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
                    html.Div(
                        children=html.H4(
                            id="plot_header_title",
                            children=f"{GREEK_SYMBOLS[GREEK_TYPES[0]]} Heatmap Comparison (K vs. \u03C3)",
                            style={"padding": "15px " + PADDING_MD, "margin": 0, "fontSize": FONT_SIZE_XL}
                        ),
                        style={
                            "backgroundColor": COLOR_BG_PRIMARY,
                            "borderBottom": BORDER_STYLE
                        }
                    ),
                    html.Div(
                        style={"display": "flex", "flexWrap": "wrap", "width": "100%", "flexGrow": "1"},
                        children=[
                            dcc.Graph(
                                id="heatmap_ac",
                                style={
                                    "width": GRID_ITEM_SIZE,
                                    "height": GRID_ITEM_SIZE,
                                    "borderBottom": BORDER_STYLE,
                                    "borderRight": BORDER_STYLE
                                },
                                config={'displaylogo': False}
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

def create_input_group(
    label: str,
    id_name: str,
    value: float | int,
    min_val: float | int,
    max_val: float | int,
    step: float | int
) -> html.Div:
    return html.Div(
        style={"display": "flex", "alignItems": "center", "marginBottom": "15px"},
        children=[
            html.Label(
                label,
                style={"flex": "0 0 " + GRID_ITEM_SIZE, "paddingRight": "15px", "fontWeight": "bold"}
            ),
            html.Div(
                style={"flex": "0 0 " + GRID_ITEM_SIZE},
                children=dcc.Input(
                    id=id_name,
                    type="number",
                    value=value,
                    min=min_val,
                    max=max_val,
                    step=step,
                    debounce=True,
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

def generate_heatmap_figure(
    options_grid: np.ndarray[np.float64],
    strikes_arr: np.ndarray[np.float64],
    sigmas_arr: np.ndarray[np.float64],
    title_label: str,
    greek_label: str,
    color_range: Optional[list[float]] = None
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

    fig.update_layout(
        title_font_size=FONT_SIZE_LG,
        plot_bgcolor=COLOR_BG_SECONDARY,
        paper_bgcolor=COLOR_BG_SECONDARY,
        font=dict(color=COLOR_TEXT_DEFAULT, size=FONT_PLOT_TICK),
        margin=MARGIN_PLOT
    )

    fig.update_traces(
        text=[[f"{val:.2f}" for val in row] for row in options_grid],
        texttemplate="%{text}",
        hovertemplate=f"Strike: %{{x:.2f}}<br>Volatility: %{{y:.3f}}<br>{color_label}: %{{z:.4f}}<extra></extra>",
        textfont=dict(size=FONT_PLOT_LBL, family="Arial")
    )

    fig.update_xaxes(side="bottom", tickformat=".0f", title_font=dict(size=FONT_PLOT_LBL), fixedrange=True)
    fig.update_yaxes(tickformat=".2f", title_font=dict(size=FONT_PLOT_LBL), fixedrange=True)
    fig.update_layout(
        coloraxis_colorbar=dict(
            title=color_label,
            thickness=FONT_PLOT_BAR_THICKNESS,
            len=0.9,
            title_font=dict(size=FONT_PLOT_BAR_TITLE),
            tickfont=dict(size=FONT_PLOT_TICK)
        )
    )
    return fig

def generate_ranges(
    sigma_range: list[float],
    strike_range: list[float]
) -> tuple[np.ndarray[np.float64], np.ndarray[np.float64]]:
    sigmas_arr: np.ndarray[np.float64] = np.linspace(sigma_range[0], sigma_range[1], GRID_RESOLUTION)
    strikes_arr: np.ndarray[np.float64] = np.linspace(strike_range[0], strike_range[1], GRID_RESOLUTION)
    return np.asfortranarray(sigmas_arr), np.asfortranarray(strikes_arr)  # column major order (for eigen computations)
