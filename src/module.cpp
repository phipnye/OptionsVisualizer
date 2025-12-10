#include "OptionsVisualizer/grid/price_grid.hpp"
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
namespace py = pybind11;

PYBIND11_MODULE(pricing, m) {
    m.doc() = "Produce grid of option values across a series of strike and volatility values.";

    // Bind priceGrids<double>
    m.def("price_grids", &priceGrids<double>, py::arg("S"), py::arg("K_arr"), py::arg("r"), py::arg("q"),
          py::arg("sigma_arr"), py::arg("T_exp"),
          "Produce a 3D array of option prices (trinomial + BSM, calls and puts) across strikes and volatilities");
}
