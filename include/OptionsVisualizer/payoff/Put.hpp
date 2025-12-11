#pragma once

#include "OptionsVisualizer/math/generic_math.hpp"
#include "OptionsVisualizer/payoff/Payoff.hpp"
#include "OptionsVisualizer/utils/typing.hpp"

namespace Payoff {

template <typename T>
struct Put : public Payoff<T> {
    static constexpr T zero{0};

    T operator()(utils::type::ParamT<T> spot, utils::type::ParamT<T> strike) const override {
        return generic::max<T, T>(strike - spot, zero);
    }
};

} // namespace Payoff