#include "OptionsVisualizer/lru/ParamsHash.hpp"
#include "OptionsVisualizer/lru/Params.hpp"
#include <cstddef>
#include <cstdint>
#include <functional>

std::size_t ParamsHash::operator()(const Params& params) const noexcept {
    std::size_t h{0};
    static std::hash<std::int64_t> hasher{};

    auto hashCombine{[&h](std::size_t v) {
        // 64-bit version of boost::hash_combine
        h ^= v + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
    }};

    hashCombine(hasher(params.nSigma));
    hashCombine(hasher(params.nStrike));
    hashCombine(hasher(params.spot));
    hashCombine(hasher(params.r));
    hashCombine(hasher(params.q));
    hashCombine(hasher(params.sigmaLo));
    hashCombine(hasher(params.sigmaHi));
    hashCombine(hasher(params.strikeLo));
    hashCombine(hasher(params.strikeHi));
    hashCombine(hasher(params.tau));
    return h;
}
