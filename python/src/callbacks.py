import dash
import numpy as np
from config import SETTINGS
from dash import exceptions, html, Input, Output
from mappings import GREEK_ENUM, OPTION_TYPES
from plotly.graph_objects import Figure
from plotting import generate_heatmap_figure
from services import PricingService
from slider import compute_strike_slider, StrikeSliderConfig


def register_callbacks(app: dash.Dash) -> None:
    # --- Dynamically adjusts strike range based on spot price input
    @app.callback(
        [Output("strike_range", prop) for prop in ["min", "max", "value", "marks"]],
        Input("input_spot", "value"),
    )
    def update_strike_and_spot(spot: float) -> tuple[float, float, list[float], dict]:
        if spot is None:
            raise exceptions.PreventUpdate

        # Re-calculates slider bounds and tick marks relative to new spot price
        cfg: StrikeSliderConfig = compute_strike_slider(spot)
        return cfg.min, cfg.max, cfg.value, cfg.marks

    # --- Updates the text summary of fixed parameters
    @app.callback(
        Output("param_summary", "children"),
        [Input(f"input_{param}", "value") for param in ["spot", "tau", "r", "q"]],
    )
    def update_param_summary(spot: float, tau: float, r: float, q: float) -> html.Pre:
        # Prevent update if numeric inputs are cleared or fall out of min/max range
        if any(v is None for v in (spot, tau, r, q)):
            raise exceptions.PreventUpdate

        return html.Pre(f"S = ${spot:,.2f}\nT = {tau:.2f} years\nr = {r:.2%}\nq = {q:.2%}")

    # Update heatmap plots
    @app.callback(
        [Output(f"heatmap_{option.id}", "figure") for option in OPTION_TYPES.values()],
        [Input(f"{param}_range", "value") for param in ["sigma", "strike"]],
        [Input(param, "value") for param in ["greek_selector", "input_spot", "input_tau", "input_r", "input_q"]],
    )
    def update_heatmaps(
        sigma_range: list[float],
        strike_range: list[float],
        greek_selector: str,
        spot: float,
        tau: float,
        r: float,
        q: float,
    ) -> tuple[Figure, ...]:
        # Prevent update if numeric inputs are cleared or fall out of min/max range
        if any(v is None for v in (greek_selector, strike_range, sigma_range, spot, tau, r, q)):
            raise exceptions.PreventUpdate

        try:
            greek_idx: int = int(greek_selector)
            grids: tuple[np.ndarray[np.float64], ...]
            strikes: np.ndarray[np.float64]
            sigmas: np.ndarray[np.float64]

            # C++ engine call returns a grid for each option type (American and Europena put and call)
            grids, strikes, sigmas = PricingService.calculate_greeks(
                greek_idx, spot, r, q, sigma_range, strike_range, tau
            )

            # Calculate global color scale across all 4 heatmaps
            z_min: np.float64 = min(grid.min() for grid in grids)
            z_max: np.float64 = max(grid.max() for grid in grids)

            return tuple(
                generate_heatmap_figure(
                    grid=grids[i],
                    sigmas=sigmas,
                    strikes=strikes,
                    opt_idx=opt_idx,  # integer enum value
                    greek_idx=greek_idx,
                    color_range=(z_min, z_max),
                )
                for i, opt_idx in enumerate(OPTION_TYPES.keys())
            )

        except Exception:
            # Fallback to empty zero-grids if engine fails
            return tuple(
                generate_heatmap_figure(
                    grid=np.zeros((SETTINGS.GRID_RESOLUTION, SETTINGS.GRID_RESOLUTION)),
                    sigmas=np.zeros(SETTINGS.GRID_RESOLUTION),
                    strikes=np.zeros(SETTINGS.GRID_RESOLUTION),
                    opt_idx=idx,
                    greek_idx=int(GREEK_ENUM.Price.value),
                )
                for idx in OPTION_TYPES.keys()
            )
