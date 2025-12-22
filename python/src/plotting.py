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
    precision: float = SETTINGS.PREC_GAMMA if greek_entry.label.lower() == "gamma" else SETTINGS.PREC_STANDARD

    fig: Figure = px.imshow(
        grid,
        x=strikes,
        y=sigmas,
        origin="lower",  # puts low strike/vol at bottom-left
        text_auto=False,  # text formatting is done manually in update_traces
        labels={"x": "Strike (K)", "y": "Volatility (\u03c3)", "color": greek_entry.display_name},
        aspect="auto",
        title=f"{opt_entry.label}: {greek_entry.display_name}",
        color_continuous_scale=SETTINGS.COLOR_SCALE,
        zmin=color_range[0] if color_range else None,
        zmax=color_range[1] if color_range else None,
    )

    # Fine-tune data labels and tooltips
    fig.update_traces(
        text=[[f"{val:{precision}}" for val in row] for row in grid],  # text annotations for the heatmap cells
        texttemplate="%{text}",
        hovertemplate=(
            f"Strike: %{{x:{SETTINGS.X_AXIS_FORMAT}}}"
            f"<br>Volatility: %{{y:{SETTINGS.Y_AXIS_FORMAT}}}"
            f"<br>{greek_entry.label}: %{{z:{precision}}}"
            "<extra></extra>"
        ),
        textfont=dict(size=SETTINGS.PLOT_FONT_SIZE, family=SETTINGS.FONT_FAMILY),
    )

    # Configure axis formatting and disable scrolling out-of-scope
    fig.update_xaxes(
        side="bottom",
        tickformat=SETTINGS.X_AXIS_FORMAT,
        title_font=dict(size=SETTINGS.PLOT_FONT_SIZE),
        fixedrange=True,
    )

    fig.update_yaxes(tickformat=SETTINGS.Y_AXIS_FORMAT, title_font=dict(size=SETTINGS.PLOT_FONT_SIZE), fixedrange=True)

    fig.update_layout(
        margin=SETTINGS.PLOT_MARGINS,
        autosize=True,
        height=None,  # allows 'flex-grow-1' in panels.py to work correctly
        coloraxis_colorbar=dict(title=None, thickness=SETTINGS.COLORBAR_THICKNESS),
        title=dict(x=SETTINGS.TITLE_X_POS, xanchor="center", font=dict(size=SETTINGS.TITLE_FONT_SIZE)),
        font=dict(family=SETTINGS.FONT_FAMILY),
    )

    return fig
