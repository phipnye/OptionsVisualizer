# OptionsVisualizer: High-Performance Financial Option Modeling

This project is a tool designed for the **valuation and visualization of financial options**. It combines the speed of compiled **C++** for computational finance models with the flexibility and user-friendly capabilities of **Python** for visualization.

## Core Features and Structure

| Component | Description | Technologies |
| :--- | :--- | :--- |
| **Computational Core** | High-performance implementation of option pricing models (Black-Scholes-Merton, Trinomial Tree) for both European and American style options. | C++ (C++20), CMake |
| **Frontend/Server** | A Python application serving as the visualization layer, designed to consume data from the C++ core to render data (e.g., option Greeks, volatility surfaces, heatmaps). | Python, `app.py` |
| **Build Environment** | The entire development environment is containerized to ensure consistent dependencies and easy setup for anyone wanting to build the application themselves. | Docker, VS Code Dev Containers |

## Project Layout Summary

* `include/`: **C++ Header Files** (`.hpp`) for financial models, math utilities, option classes, and server controllers.
* `src/`: **C++ Source Files** (`.cpp`) for core logic implementation.
* `app/`: **Python Application** and its dependencies (`app.py`, `requirements.txt`).
* `CMakeLists.txt`: Defines the C++ build process.
* `.devcontainer/`: Configuration files for the reproducible **Dev Container** environment.