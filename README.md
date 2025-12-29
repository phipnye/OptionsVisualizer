# Option Pricing Dashboard (Work in Progress)

> This project is under active development. Features and visualizations may change.

## Overview

This project is an **interactive financial dashboard** for exploring option pricing using the **Black-Scholes** and **trinomial pricing models**. Users can visualize how option values and sensitivities (Greeks) change across different strikes, volatilities, and other parameters in real time.

The dashboard provides:

- Multiple option types: American Call/Put and European Call/Put.
- Heatmap visualization of option prices and Greeks.
- Adjustable ranges for volatility, strike price, and fixed parameters (spot price, interest rate, dividend, time to expiration).
- Dynamic updates based on user input, with safeguards against invalid inputs.


## Key Features

1. **Interactive Heatmaps**
   - Visualize prices and Greeks such as Delta, Gamma, Vega, Theta, and Rho.
   - Automatic color scaling for all option types to allow intuitive comparison.

2. **Dynamic Input Controls**
   - Users can adjust spot price, time to expiration, interest rate, and dividend.
   - Strike and volatility ranges are adjustable via sliders.
   - Input validation ensures parameter values remain within realistic bounds.

3. **Multiple Option Types**
   - American Call and Put
   - European Call and Put
   - Heatmaps are organized in a 2x2 grid for easy comparison.

4. **Real-Time Updates**
   - Changes to inputs immediately refresh the heatmaps and summary.
   - Supports high-resolution grids for detailed visual analysis.


## User Flow

1. **Select Greek / Metric**
   - Choose which output metric (Price, Delta, Gamma, etc.) to visualize.

2. **Adjust Ranges**
   - Use sliders to set strike and volatility ranges for the heatmaps.

3. **Set Fixed Parameters**
   - Input spot price, time to expiration, interest rate, and dividend.
   - The dashboard validates inputs and updates the strike slider accordingly.

4. **View Heatmaps**
   - Heatmaps for each option type update automatically.
   - Hover over the heatmaps to see detailed values for each strike/volatility pair.

5. **Monitor Summary**
   - A summary box shows the current fixed parameters for quick reference.


## Technical Notes

- Uses a **C++ backend** for fast option pricing calculations. This backend takes advanctage of multithreaded and vectorized evaulation along with caching and backward induction to make the computations highly efficient.
- Frontend is built with **Dash** and **Dash Bootstrap Components**.
- Plotly is used for interactive heatmaps with consistent theming.
