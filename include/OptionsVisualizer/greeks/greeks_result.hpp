#pragma once

template <typename T>
struct GreeksResult {
    T price;
    T delta;
    T gamma;
    T vega;
    T theta;
};