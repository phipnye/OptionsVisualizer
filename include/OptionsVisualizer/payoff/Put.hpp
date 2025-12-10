#pragma once

#include "OptionsVisualizer/math/generic_math.hpp"
#include "OptionsVisualizer/payoff/Payoff.hpp"
#include "OptionsVisualizer/utils/typing.hpp"

namespace Payoff {

template <typename T>
struct Put : public Payoff<T> {
    T operator()(utils::type::ParamT<T> S_t, utils::type::ParamT<T> K) const override {
        static const T zero{0};
        return generic::max<T, T>(K - S_t, zero);
    }
};

} // namespace Payoff