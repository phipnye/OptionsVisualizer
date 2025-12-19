#pragma once

#include "OptionsVisualizer/core/Enums.hpp"
#include "OptionsVisualizer/core/globals.hpp"
#include "OptionsVisualizer/lru/Params.hpp"
#include "OptionsVisualizer/lru/ParamsHash.hpp"
#include <Eigen/Dense>
#include <array>
#include <cstddef>
#include <list>
#include <unordered_map>
#include <utility>

class LRUCache {
protected:
    // --- Data members
    std::size_t capacity_;
    std::list<Params> keys_{};
    std::unordered_map<Params, std::pair<std::array<Eigen::MatrixXd, globals::nGrids>, std::list<Params>::iterator>,
                       ParamsHash>
        cache_{};

    // Set values
    virtual void set(const Params& params, std::array<Eigen::MatrixXd, globals::nGrids>&& value) = 0;

public:
    // Ctor and dtor
    explicit LRUCache(std::size_t capacity);
    virtual ~LRUCache() = default;

    // Get values
    virtual const std::array<Eigen::MatrixXd, globals::nGrids>& get(Eigen::DenseIndex nSigma, Eigen::DenseIndex nStrike,
                                                                    double spot, double r, double q, double sigmaLo,
                                                                    double sigmaHi, double strikeLo, double strikeHi,
                                                                    double tau) = 0;
};
