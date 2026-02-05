import CppPricingEngine
import enum
from typing import NamedTuple

# Enums exported from C++ to match indexing logic
GREEK_ENUM: enum.Enum = CppPricingEngine.OptionsManager.GreekType
OPT_ENUM: enum.Enum = CppPricingEngine.OptionsManager.OptionType

# Make sure we're not missing enum options
assert GREEK_ENUM.COUNT.value == GREEK_ENUM.Rho.value + 1, "Missing greek type enums value(s)"
assert OPT_ENUM.COUNT.value == OPT_ENUM.EuroPut.value + 1, "Missing option type enums value(s)"

class OptionTypeEntry(NamedTuple):
    label: str
    id: str


class GreekTypeEntry(NamedTuple):
    label: str
    display_name: str


# Mapping of C++ Enum integer values to human-readable labels and html ids
OPTION_TYPES: dict[int, OptionTypeEntry] = {
    OPT_ENUM.AmerCall.value: OptionTypeEntry("American Call", "ac"),
    OPT_ENUM.AmerPut.value: OptionTypeEntry("American Put", "ap"),
    OPT_ENUM.EuroCall.value: OptionTypeEntry("European Call", "ec"),
    OPT_ENUM.EuroPut.value: OptionTypeEntry("European Put", "ep"),
}

# Use the Enum values as keys for the greek_selector
GREEK_TYPES: dict[int, GreekTypeEntry] = {
    GREEK_ENUM.Price.value: GreekTypeEntry("Price", "Price"),
    GREEK_ENUM.Delta.value: GreekTypeEntry("Delta", "\u0394 (Delta)"),
    GREEK_ENUM.Gamma.value: GreekTypeEntry("Gamma", "\u0393 (Gamma)"),
    GREEK_ENUM.Vega.value: GreekTypeEntry("Vega", "\u03bd (Vega)"),
    GREEK_ENUM.Theta.value: GreekTypeEntry("Theta", "\u0398 (Theta)"),
    GREEK_ENUM.Rho.value: GreekTypeEntry("Rho", "\u03c1 (Rho)"),
}
