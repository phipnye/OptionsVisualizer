#include <pybind11/eigen.h>
#include <pybind11/native_enum.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

#include <Eigen/Dense>
#include <array>

#include "OptionsVisualizer/core/Enums.hpp"
#include "OptionsVisualizer/core/OptionsManager.hpp"
#include "OptionsVisualizer/core/globals.hpp"
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
        const std::array<Eigen::ArrayXXd, globals::nGrids>& grids{
            manager.get(nSigma, nStrike, spot, r, q, sigmaLo, sigmaHi, strikeLo,
                        strikeHi, tau)};

        constexpr std::size_t nGreeks{Enums::idx(Enums::GreekType::COUNT)};
        constexpr std::size_t nOptTypes{Enums::idx(Enums::OptionType::COUNT)};
        const std::size_t greekIdx{Enums::idx(greekType)};

        // Re-acquire the GIL
        py::gil_scoped_acquire gil{};
        py::tuple output(nOptTypes);

        for (std::size_t optIdx{0}; optIdx < nOptTypes; ++optIdx) {
          // Pass immuatable views of data to python
          const Eigen::ArrayXXd& grid{grids[optIdx * nGreeks + greekIdx]};
          output[optIdx] = py::array{
              {grid.rows(), grid.cols()}, // shape
              {sizeof(double),
               sizeof(double) *
               static_cast<std::size_t>(grid.outerStride())}, // strides
              grid.data(), // data pointer
              py::cast(&manager) // owner
          };
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

  // Generate coordina
  m.def("linspace", &linspace, py::arg("size"), py::arg("lo"), py::arg("hi"),
        "Create a linearly spaced vector");
}