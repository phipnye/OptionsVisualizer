#include "OptionsVisualizer/Grid/Grid.hpp"
#include <Eigen/Dense>
#include <pybind11/eigen.h>
#include <pybind11/pybind11.h>
namespace py = pybind11;

PYBIND11_MODULE(pricing, m) {
    m.doc() = "Produce grid of option Greeks (Price, Delta, Gamma, Vega, Theta, Rho) across a series of strike and "
              "volatility values.";

    py::class_<Grid>(m, "Grid")
        .def(py::init<double, Eigen::Ref<Eigen::VectorXd>, double, double, Eigen::Ref<Eigen::VectorXd>, double>(),
             py::arg("spot"), py::arg("strikes_arr").noconvert(), py::arg("r"), py::arg("q"),
             py::arg("sigmas_arr").noconvert(), py::arg("tau"))
        .def("calculate_grids", &Grid::calculateGrids, "Compute option Greeks on the grid");
}