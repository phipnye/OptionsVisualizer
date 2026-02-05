import CppPricingEngine
import logging
import numpy as np
from config import SETTINGS
from CppPricingEngine import linspace
from mappings import GREEK_ENUM


class PricingService:
    # Class handles all c++ pricing interactions
    if SETTINGS.ENGINE_THREADS is None:
        manager: CppPricingEngine.OptionsManager = CppPricingEngine.OptionsManager(capacity=SETTINGS.ENGINE_CAPACITY)
    else:
        manager = CppPricingEngine.OptionsManager(capacity=SETTINGS.ENGINE_CAPACITY, n_threads=SETTINGS.ENGINE_THREADS)

    engine_logger: logging.Logger = logging.getLogger(__name__)

    @staticmethod
    def calculate_greeks(
        greek_idx: int,
        spot: float,
        r: float,
        q: float,
        sigma_range: list[float],
        strike_range: list[float],
        tau: float,
    ) -> tuple[tuple[np.ndarray, ...], np.ndarray, np.ndarray]:
        try:
            # Generate linear axis arrays for the heatmap grid coordinates (CppPricingEngine.linspace is used for
            # consistency with the C++ engine)
            sigmas: np.ndarray = linspace(SETTINGS.GRID_RESOLUTION, sigma_range[0], sigma_range[1])
            strikes: np.ndarray = linspace(SETTINGS.GRID_RESOLUTION, strike_range[0], strike_range[1])

            # Retrieve pricing grids from the underlying C++ OptionsManager (either retrieves cached results or
            # generates new ones)
            grids: tuple[np.ndarray, ...] = PricingService.manager.get_greek(
                GREEK_ENUM(greek_idx),
                SETTINGS.GRID_RESOLUTION,
                SETTINGS.GRID_RESOLUTION,
                spot,
                r,
                q,
                sigma_range[0],
                sigma_range[1],
                strike_range[0],
                strike_range[1],
                tau,
            )

        except Exception as e:
            PricingService.engine_logger.error(f"Engine failure for Greek {greek_idx}: {e}", exc_info=True)
            raise e

        return grids, strikes, sigmas
