#pragma once

#include "OptionsVisualizer/greeks/GreeksResult.hpp"
#include "OptionsVisualizer/greeks/bsm/calculate_greeks.hpp"
#include "OptionsVisualizer/greeks/trinomial/calculate_greeks.hpp"
#include "OptionsVisualizer/grid/index.hpp"
#include <cstdint>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <torch/torch.h>
namespace py = pybind11;

namespace grid {

template <typename T>
py::array_t<T> calculateGrids(T spot_, py::array_t<T, py::array::c_style> strikes_, T r_, T q_,
                              py::array_t<T, py::array::c_style> sigmas_, T tau_) {
    // --- Setup

    assert(strikes_.ndim() == 1 && sigmas_.ndim() == 1 && "Unexpected number of dimensions.");
    const std::int64_t nStrike{strikes_.shape(0)};
    const std::int64_t nSigma{sigmas_.shape(0)};

    // Convert inputs to torch tensors
    torch::Tensor spot{torch::tensor(spot_)};
    torch::Tensor strikes{
        torch::from_blob(strikes_.mutable_data(), {nStrike}, torch::dtype(torch::CppTypeToScalarType<T>::value))
            .clone()};
    torch::Tensor r{torch::tensor(r_)};
    torch::Tensor q{torch::tensor(q_)};
    torch::Tensor sigmas{
        torch::from_blob(sigmas_.mutable_data(), {nSigma}, torch::dtype(torch::CppTypeToScalarType<T>::value)).clone()};
    torch::Tensor tau{torch::tensor(tau_)};

    // Release gill (no longer need access to python objects)
    py::gil_scoped_release noGil{};

    greeks::GreeksResult amerCall{greeks::trinomial::callGreeks(spot, strikes, r, q, sigmas, tau)};
    greeks::GreeksResult amerPut{greeks::trinomial::putGreeks(spot, strikes, r, q, sigmas, tau)};
    greeks::GreeksResult euroCall{greeks::bsm::callGreeks(spot, strikes, r, q, sigmas, tau)};
    greeks::GreeksResult euroPut{greeks::bsm::putGreeks(spot, strikes, r, q, sigmas, tau)};

    // Write results to output
    torch::Tensor output{torch::empty({nSigma, nStrike, 4, 5}, strikes.options())};
    index::writeOptionGreeks(output, index::OptionType::AmerCall, amerCall);
    index::writeOptionGreeks(output, index::OptionType::AmerPut, amerPut);
    index::writeOptionGreeks(output, index::OptionType::EuroCall, euroCall);
    index::writeOptionGreeks(output, index::OptionType::EuroPut, euroPut);
    output = output.contiguous();

    // https://github.com/pybind/pybind11/issues/1042#issuecomment-325941022
    // Link the lifetime of the underlying data to the lifetime of the python object (numpy array)
    py::gil_scoped_acquire acquireGil{};
    torch::Tensor* outputPtr{new torch::Tensor{std::move(output)}};
    py::capsule capsule{outputPtr, [](void* p) { delete static_cast<torch::Tensor*>(p); }};
    py::array::ShapeContainer shape{static_cast<long>(nSigma), static_cast<long>(nStrike), 4L, 5L};
    return py::array_t<T>{shape, static_cast<T*>(outputPtr->data_ptr()), capsule};
}

} // namespace grid
