import numpy as np
import plotly.express as px
from config import SETTINGS
from mappings import GreekTypeEntry, OptionTypeEntry, GREEK_TYPES, OPTION_TYPES
from plotly.graph_objects import Figure
from typing import Optional


def generate_heatmap_figure(
    grid: np.ndarray,
    sigmas: np.ndarray,
    strikes: np.ndarray,
    opt_idx: int,
    greek_idx: int,
    color_range: Optional[tuple[float, float]] = None,
) -> Figure:
    opt_entry: OptionTypeEntry = OPTION_TYPES[opt_idx]
    greek_entry: GreekTypeEntry = GREEK_TYPES[greek_idx]

    # Gamma typically requires higher precision
    precision: str = SETTINGS.PREC_GAMMA if greek_entry.label.lower() == "gamma" else SETTINGS.PREC_STANDARD

    # Main plot definition
    fig: Figure = px.imshow(
        grid,
        x=strikes,
        y=sigmas,
        origin="lower",  # puts low strike/vol at bottom-left
        aspect="auto",  # allows heatmap to stretch/fill the flex container
        title=opt_entry.label,
        labels={"x": "Strike (K)", "y": "Volatility (\u03c3)", "color": greek_entry.display_name},
        text_auto=precision,  # text within cells of grid (round to the given precision)
        color_continuous_scale=SETTINGS.PLOT_COLOR_SCALE,
        zmin=color_range[0] if color_range else None,
        zmax=color_range[1] if color_range else None,
    )

    # Define cell hover behavior
    fig.update_traces(
        hovertemplate=(
            f"Strike: %{{x:.2f}}<br>"
            f"Volatility: %{{y:.3f}}<br>"
            f"{greek_entry.label}: %{{z:{precision}}}"
            "<extra></extra>"  # <extra></extra> removes the secondary "trace name" box from the tooltip
        )
    )

    # Update axes
    fig.update_xaxes(
        side="bottom",  # make sure the volatilities are ascending along the y-axis
        tickformat=SETTINGS.X_AXIS_FORMAT,
        title_font=dict(size=SETTINGS.PLOT_FONT_SIZE),
    )
    fig.update_yaxes(tickformat=SETTINGS.Y_AXIS_FORMAT, title_font=dict(size=SETTINGS.PLOT_FONT_SIZE))

    # Confgure the plot's visuals and layout
    fig.update_layout(
        margin=SETTINGS.PLOT_MARGINS,
        autosize=True,
        height=None,  # allows Dash 'flex-grow-1' CSS to manage vertical sizing
        font=dict(family=SETTINGS.PLOT_FONT_FAMILY),
        coloraxis_colorbar=dict(title=None, thickness=SETTINGS.PLOT_COLORBAR_THICKNESS),
        title=dict(x=SETTINGS.PLOT_TITLE_XPOS, xanchor="center", font=dict(size=SETTINGS.PLOT_TITLE_SIZE)),
    )

    return fig
