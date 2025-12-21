import enum
import options_surface

# Shortcut for readability
greek_enum: enum.Enum = options_surface.OptionsManager.GreekType
opt_enum: enum.Enum = options_surface.OptionsManager.OptionType

# The number of strikes and sigmas to price across
GRID_RESOLUTION: int = 10

# Now these are synced to C++ memory values
OPTION_TYPES: dict[int, str] = {
    opt_enum.AmerCall.value: "American Call",
    opt_enum.AmerPut.value: "American Put",
    opt_enum.EuroCall.value: "European Call",
    opt_enum.EuroPut.value: "European Put"
}

# Use the Enum values as keys for the Selector
GREEK_TYPES: dict[int, str] = {
    greek_enum.Price.value: "Price",
    greek_enum.Delta.value: "Delta",
    greek_enum.Gamma.value: "Gamma",
    greek_enum.Vega.value: "Vega",
    greek_enum.Theta.value: "Theta",
    greek_enum.Rho.value: "Rho"
}

GREEK_SYMBOLS: dict[str, str] = {
    "Price": "Price",
    "Delta": "\u0394 (Delta)",
    "Gamma": "\u0393 (Gamma)",
    "Vega": "\u03bd (Vega)",
    "Theta": "\u0398 (Theta)",
    "Rho": "\u03c1 (Rho)"
}
