#include "OptionsVisualizer/greeks/bsm/calculate_greeks.hpp"
#include "OptionsVisualizer/greeks/trinomial/calculate_greeks.hpp"
#include <gtest/gtest.h>
#include <torch/torch.h>

TEST(PricingModuleTest, TrinomialCallPriceIsPositive) {
    double S{100.0};
    double K{100.0};
    double R{0.05};
    double Q{0.02};
    double Sigma{0.20};
    double Tau{1.0};

    torch::Tensor spot{torch::tensor(S)};
    torch::Tensor strikes{torch::tensor(K).unsqueeze(0)};
    torch::Tensor r{torch::tensor(R)};
    torch::Tensor q{torch::tensor(Q)};
    torch::Tensor sigmas{torch::tensor(Sigma).unsqueeze(0)};
    torch::Tensor tau{torch::tensor(Tau)};
    greeks::GreeksResult result{greeks::trinomial::callGreeks(spot, strikes, r, q, sigmas, tau)};

    // Assertions
    double price{result.price.index({0, 0, 0}).item<double>()};
    ASSERT_GT(price, 0.0) << "American Call price should be positive.";
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}