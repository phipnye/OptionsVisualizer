#pragma once

#include <cassert>
#include <cstddef>
#include <execution>
#include <type_traits>
#include <vector>

namespace Option::Grid {

template <typename T, typename OptionType>
class OptionGrid {
    // Raw memory that's suited for alignment
    using Cell = std::aligned_storage_t<sizeof(OptionType), alignof(OptionType)>;

    // Data members
    std::size_t nrow_, ncol_;
    std::vector<Cell> storage_;
    std::vector<unsigned char> constructed_;

public:
    // Constructor
    OptionGrid(std::size_t nrow, std::size_t ncol) noexcept
        : nrow_{nrow}, ncol_{ncol}, storage_(nrow * ncol_), constructed_(storage_.size(), 0) {}

    // Destructor
    ~OptionGrid() noexcept {
        OptionType* const endIt{end()};
        auto constructedIt{constructed_.cbegin()};

        for (OptionType* it{begin()}; it != endIt; ++it, ++constructedIt) {
            if (*constructedIt) {
                it->~OptionType();
            }
        }
    }

    // Rule of 5
    OptionGrid(const OptionGrid&) = delete;
    OptionGrid(OptionGrid&&) = delete;
    OptionGrid& operator=(const OptionGrid&) = delete;

    // Construct option objects in-place
    template <typename... Args>
    void emplace(std::size_t row, std::size_t col, Args&&... args) {
        const std::size_t idx{index(row, col)};
        assert(!constructed_[idx] && "Cell already constructed!");
        new (&storage_[idx]) OptionType{std::forward<Args>(args)...};
        constructed_[idx] = 1;
    }

    // Access dimensions
    std::size_t nrow() const noexcept {
        return nrow_;
    }

    std::size_t ncol() const noexcept {
        return ncol_;
    }

    // Iterators
    OptionType* begin() noexcept {
        return reinterpret_cast<OptionType*>(storage_.data());
    }

    const OptionType* cbegin() const noexcept {
        return reinterpret_cast<const OptionType*>(storage_.data());
    }

    OptionType* end() noexcept {
        return begin() + (nrow_ * ncol_);
    }

    const OptionType* cend() const noexcept {
        return cbegin() + (nrow_ * ncol_);
    }

    // Data access
    OptionType& operator()(std::size_t row, std::size_t col) {
        const std::size_t idx{index(row, col)};
        assert(constructed_[idx] && "Accessing uninitialized cell!");
        return *(begin() + idx);
    }

    const OptionType& operator()(std::size_t row, std::size_t col) const {
        const std::size_t idx{index(row, col)};
        assert(constructed_[idx] && "Accessing uninitialized cell!");
        return *(cbegin() + idx);
    }

    // Bulk parallel pricing
    void calcPricesParallel() {
#ifndef NDEBUG
        for (unsigned char b : constructed_) {
            assert(b && "Uninitialized cell before calcPricesParallel()");
        }
#endif

        std::for_each(std::execution::par_unseq, begin(), end(), [](OptionType& opt) { opt.calcPrice(); });
    }

private:
    std::size_t index(std::size_t row, std::size_t col) const {
        assert(row < nrow_ && col < ncol_ && "Out-of-range access!");
        return (row * ncol_) + col;
    }
};

} // namespace Option::Grid
