#include "OptionsVisualizer/grid/index.hpp"
#include "OptionsVisualizer/greeks/GreeksResult.hpp"
#include <cstdint>
#include <pybind11/pybind11.h>
#include <torch/torch.h>
namespace py = pybind11;

namespace grid::index {

constexpr std::int64_t idx(OptionType t) noexcept {
    return static_cast<std::int64_t>(t);
}

constexpr std::int64_t idx(GreekType t) noexcept {
    return static_cast<std::int64_t>(t);
}

void writeOptionGreeks(torch::Tensor& output, index::OptionType option, const greeks::GreeksResult& g) {
    using torch::indexing::Slice;
    output.index_put_({Slice(), Slice(), index::idx(option), index::idx(index::GreekType::Price)}, g.price);
    output.index_put_({Slice(), Slice(), index::idx(option), index::idx(index::GreekType::Delta)}, g.delta);
    output.index_put_({Slice(), Slice(), index::idx(option), index::idx(index::GreekType::Gamma)}, g.gamma);
    output.index_put_({Slice(), Slice(), index::idx(option), index::idx(index::GreekType::Vega)}, g.vega);
    output.index_put_({Slice(), Slice(), index::idx(option), index::idx(index::GreekType::Theta)}, g.theta);
}

} // namespace grid::index
