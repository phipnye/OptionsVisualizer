#include <pybind11/eigen.h>
#include <pybind11/native_enum.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

#include <Eigen/Dense>
#include <type_traits>

#include "OptionsVisualizer/core/Enums.hpp"
#include "OptionsVisualizer/core/OptionsManager.hpp"
#include "OptionsVisualizer/core/linspace.hpp"
namespace py = pybind11;

PYBIND11_MODULE(CppPricingEngine, m) {
  // --- OptionsManager class
  py::class_<OptionsManager> pyOptionsManager{m, "OptionsManager"};

  // Exposed Constructors (first uses all available threads)
  pyOptionsManager.def(py::init<std::size_t>(), py::arg("capacity"));
  pyOptionsManager.def(py::init<std::size_t, std::size_t>(),
                       py::arg("capacity"), py::arg("n_threads"));

  // Method to retrieve greeks values
  pyOptionsManager.def(
      "get_greek",
      [](OptionsManager& manager, const Enums::GreekType greekType,
         const Eigen::Index nSigma, const Eigen::Index nStrike,
         const double spot, const double r, const double q,
         const double sigmaLo, const double sigmaHi, const double strikeLo,
         const double strikeHi, const double tau) {
        // Release GIL for multithreaded evaluation
        py::gil_scoped_release noGil{};

        // Get the reference to the array in the cache
        const auto& grids{manager.get(nSigma, nStrike, spot, r, q, sigmaLo,
                                      sigmaHi, strikeLo, strikeHi, tau)};

        // Re-acquire the GIL
        py::gil_scoped_acquire gil{};

        // Pass immuatable views of data to python
        static constexpr std::size_t nGreeks{
            Enums::idx(Enums::GreekType::COUNT)};
        static constexpr std::size_t nOptTypes{
            Enums::idx(Enums::OptionType::COUNT)};
        const std::size_t greekIdx{Enums::idx(greekType)};
        py::tuple output{nOptTypes};

        for (std::size_t optIdx{0}; optIdx < nOptTypes; ++optIdx) {
          const auto& grid{grids[optIdx * nGreeks + greekIdx]};

          // Strides are defined for column-major order
          static_assert(!std::decay_t<decltype(grid)>::IsRowMajor,
                        "Strides are defined for column-major storage order.");
          constexpr py::ssize_t szDbl{sizeof(double)};

          // Map an Eigen array to a numpy array without copying the underlying
          // data
          output[optIdx] = py::array{
              // Shape
              {grid.rows(), grid.cols()},
              // Strides (defined for column major order)
              {
                  szDbl,                      // distance to next row
                  szDbl * grid.outerStride()  // distance to next column
              },
              // Data pointer
              grid.data(),
              // Owner/handle (tells python not to delete this memory when the
              // array goes out of scope since the manager object owns it)
              py::cast(&manager)};

          // Don't allow the object to be writeable in python
          output[optIdx].attr("flags").attr("writeable") = false;
        }

        return output;
      });

  // --- Enums

  // GreekType Enum
  py::native_enum<Enums::GreekType>(pyOptionsManager, "GreekType", "enum.Enum")
      .value("Price", Enums::GreekType::Price)
      .value("Delta", Enums::GreekType::Delta)
      .value("Gamma", Enums::GreekType::Gamma)
      .value("Vega", Enums::GreekType::Vega)
      .value("Theta", Enums::GreekType::Theta)
      .value("Rho", Enums::GreekType::Rho)
      .value("COUNT", Enums::GreekType::COUNT)
      .finalize();

  // OptionType Enum
  py::native_enum<Enums::OptionType>(pyOptionsManager, "OptionType",
                                     "enum.Enum")
      .value("AmerCall", Enums::OptionType::AmerCall)
      .value("AmerPut", Enums::OptionType::AmerPut)
      .value("EuroCall", Enums::OptionType::EuroCall)
      .value("EuroPut", Enums::OptionType::EuroPut)
      .value("COUNT", Enums::OptionType::COUNT)
      .finalize();

  // --- Helper functions

  // Generate coordinates
  m.def("linspace", &linspace, py::arg("size"), py::arg("lo"), py::arg("hi"),
        "Create a linearly spaced vector");
}