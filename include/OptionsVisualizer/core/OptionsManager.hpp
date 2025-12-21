#pragma once

#include "OptionsVisualizer/core/globals.hpp"
#include "OptionsVisualizer/lru/LRUCache.hpp"
#include "OptionsVisualizer/lru/Params.hpp"
#include <Eigen/Dense>
#include <array>
#include <cstddef>

class OptionsManager : public LRUCache {
public:
    explicit OptionsManager(std::size_t capacity);

    const std::array<Eigen::MatrixXd, globals::nGrids>& get(Eigen::DenseIndex nSigma, Eigen::DenseIndex nStrike,
                                                            double spot, double r, double q, double sigmaLo,
                                                            double sigmaHi, double strikeLo, double strikeHi,
                                                            double tau) override;

private:
    // Set values
    void set(const Params& params, std::array<Eigen::MatrixXd, globals::nGrids>&& value) override;
};
