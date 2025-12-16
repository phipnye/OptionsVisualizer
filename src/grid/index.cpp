#include "OptionsVisualizer/grid/index.hpp"
#include "OptionsVisualizer/greeks/GreeksResult.hpp"
#include "OptionsVisualizer/grid/Dims.hpp"
#include <Eigen/Dense>
#include <unsupported/Eigen/CXX11/Tensor>

namespace grid::index {

void writeOptionGreeks(Eigen::VectorXd& output, index::OptionType option, const greeks::GreeksResult& g, Dims dims) {
    static constexpr Eigen::DenseIndex priceIdx{index::idx(index::GreekType::Price)};
    static constexpr Eigen::DenseIndex deltaIdx{index::idx(index::GreekType::Delta)};
    static constexpr Eigen::DenseIndex gammaIdx{index::idx(index::GreekType::Gamma)};
    static constexpr Eigen::DenseIndex vegaIdx{index::idx(index::GreekType::Vega)};
    static constexpr Eigen::DenseIndex thetaIdx{index::idx(index::GreekType::Theta)};
    const Eigen::DenseIndex optionIdx{idx(option)};

    for (Eigen::DenseIndex sigmaIdx{0}; sigmaIdx < dims.nSigma; ++sigmaIdx) {
        for (Eigen::DenseIndex strikeIdx{0}; strikeIdx < dims.nStrike; ++strikeIdx) {
            output(sigmaIdx * dims.nStrike * 4 * 5 + strikeIdx * 4 * 5 + optionIdx * 5 + priceIdx) =
                g.price(sigmaIdx, strikeIdx);
            output(sigmaIdx * dims.nStrike * 4 * 5 + strikeIdx * 4 * 5 + optionIdx * 5 + deltaIdx) =
                g.delta(sigmaIdx, strikeIdx);
            output(sigmaIdx * dims.nStrike * 4 * 5 + strikeIdx * 4 * 5 + optionIdx * 5 + gammaIdx) =
                g.gamma(sigmaIdx, strikeIdx);
            output(sigmaIdx * dims.nStrike * 4 * 5 + strikeIdx * 4 * 5 + optionIdx * 5 + vegaIdx) =
                g.vega(sigmaIdx, strikeIdx);
            output(sigmaIdx * dims.nStrike * 4 * 5 + strikeIdx * 4 * 5 + optionIdx * 5 + thetaIdx) =
                g.theta(sigmaIdx, strikeIdx);
        }
    }
}

} // namespace grid::index
