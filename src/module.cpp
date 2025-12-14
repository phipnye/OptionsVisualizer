#include "OptionsVisualizer/grid/calculate_grids.hpp"
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
namespace py = pybind11;

PYBIND11_MODULE(pricing, m) {
    m.doc() = "Produce grid of option Greeks (Price, Delta, Gamma, Vega, Theta) across a series of strike and "
              "volatility values.";

    m.def("calculate_greeks_grid", &grid::calculateGrids<double>, py::arg("spot"), py::arg("strikes_arr"),
          py::arg("r"), py::arg("q"), py::arg("sigma_arr"), py::arg("tau"),
          "Produce a flat array (volatility x strike x option type x greek type) of option Greeks (Trinomial "
          "American, BSM European) across strikes and volatilities.");
}