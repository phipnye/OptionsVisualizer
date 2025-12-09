#pragma once

#include "OptionsVisualizer/models/black_scholes_merton.hpp"
#include "OptionsVisualizer/option/Option.hpp"
#include "OptionsVisualizer/utils/typing.hpp"

namespace Option {

template <typename T>
class EuropeanCall : public Option<T> {
public:
    EuropeanCall(utils::type::ParamT<T> S, utils::type::ParamT<T> K, utils::type::ParamT<T> r, utils::type::ParamT<T> q,
                 utils::type::ParamT<T> sigma, utils::type::ParamT<T> T_exp) noexcept
        : Option<T>{S, K, r, q, sigma, T_exp} {
        // calcPrice();
    }

    void calcPrice() override {
        this->p_ = pricing::bsmCall<T>(this->S_, this->K_, this->r_, this->q_, this->sigma_, this->T_exp_);
    }
};

} // namespace Option