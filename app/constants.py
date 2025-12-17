# Defines the resolution of the strike/volatility grid (e.g., 10x10)
GRID_RESOLUTION: int = 10

# --- C++ grid mappings

# Mirrors Grid::OptionType
OPTION_TYPES: dict[int, str] = {
    0: "American Call",
    1: "American Put",
    2: "European Call",
    3: "European Put"
}

# Mirrors Grid::GreekType
GREEK_TYPES: dict[int, str] = {
    0: "Price",
    1: "Delta",
    2: "Gamma",
    3: "Vega",
    4: "Theta"
}

GREEK_SYMBOLS: dict[str, str] = {
    "Price": "Price",
    "Delta": "\u0394 (Delta)",
    "Gamma": "\u0393 (Gamma)",
    "Vega": "\u03bd (Vega)",
    "Theta": "\u0398 (Theta)"
}
