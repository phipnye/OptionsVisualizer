#pragma once

#include "OptionsVisualizer/utils/typing.hpp"

namespace Option {

template <typename T>
class Option {
protected:
    T S_, K_, r_, q_, sigma_, T_exp_, p_;

    Option(utils::type::ParamT<T> S, utils::type::ParamT<T> K, utils::type::ParamT<T> r, utils::type::ParamT<T> q,
           utils::type::ParamT<T> sigma, utils::type::ParamT<T> T_exp) noexcept
        : S_{S}, K_{K}, r_{r}, q_{q}, sigma_{sigma}, T_exp_{T_exp}, p_{0.0} {}

public:
    virtual ~Option() = default;
    virtual void calcPrice() = 0;
    virtual utils::type::ParamT<T> getPrice() const final {
        return p_;
    }
};

} // namespace Option
