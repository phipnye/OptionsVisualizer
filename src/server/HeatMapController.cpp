#include "OptionsVisualizer/server/HeatMapController.hpp"
#include "OptionsVisualizer/grid/OptionGrid.hpp"
#include "OptionsVisualizer/option/AmericanCall.hpp"
#include "OptionsVisualizer/option/AmericanPut.hpp"
#include "OptionsVisualizer/option/EuropeanCall.hpp"
#include "OptionsVisualizer/option/EuropeanPut.hpp"
#include <cstddef>
#include <cstring>
#include <drogon/HttpResponse.h>
#include <functional>
#include <jsoncpp/json/json.h>

namespace server {

void HeatMapController::asyncHandleHttpRequest(const drogon::HttpRequestPtr& req,
                                               std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    // 1. Retrieve parameters from frontend app
    const auto json{*(req->getJsonObject())};

    // User-defined ranges
    const double kMin{json["k_min"].asDouble()};
    const double kMax{json["k_max"].asDouble()};
    const double sigmaMin{json["sigma_min"].asDouble()};
    const double sigmaMax{json["sigma_max"].asDouble()};
    static const std::size_t N{static_cast<std::size_t>(json["N"].asInt())};

    // User-defined fixed parameters
    const double S{json["fixed"]["S"].asDouble()};
    const double r{json["fixed"]["r"].asDouble()};
    const double q{json["fixed"]["q"].asDouble()};
    const double T_exp{json["fixed"]["T"].asDouble()};

    // 2. Build the grid and calculate option prices
    Option::Grid::OptionGrid<double, Option::AmericanCall<double>> gridAmerCalls{N, N};
    Option::Grid::OptionGrid<double, Option::AmericanPut<double>> gridAmerPuts{N, N};
    Option::Grid::OptionGrid<double, Option::EuropeanCall<double>> gridEuroCalls{N, N};
    Option::Grid::OptionGrid<double, Option::EuropeanPut<double>> gridEuroPuts{N, N};
    static const double den{static_cast<double>(N - 1)};

    for (std::size_t i{0}; i < N; ++i) {
        for (std::size_t j{0}; j < N; ++j) {
            const double strike{kMin + ((kMax - kMin) * (static_cast<double>(i) / den))};
            const double vol{sigmaMin + ((sigmaMax - sigmaMin) * (static_cast<double>(j) / den))};
            gridAmerCalls.emplace(i, j, S, strike, r, q, vol, T_exp);
            gridAmerPuts.emplace(i, j, S, strike, r, q, vol, T_exp);
            gridEuroCalls.emplace(i, j, S, strike, r, q, vol, T_exp);
            gridEuroPuts.emplace(i, j, S, strike, r, q, vol, T_exp);
        }
    }

    gridAmerCalls.calcPricesParallel();
    gridAmerPuts.calcPricesParallel();
    gridEuroCalls.calcPricesParallel();
    gridEuroPuts.calcPricesParallel();

    // 3. Return results back to frontend
    const auto httpResp{drogon::HttpResponse::newHttpResponse()};
    httpResp->setStatusCode(drogon::k200OK);
    httpResp->setContentTypeCode(drogon::CT_APPLICATION_OCTET_STREAM);
    std::string body{};
    body.resize(4 * N * N * sizeof(double));
    std::size_t idx{0};

    for (std::size_t i{0}; i < N; ++i) {
        for (std::size_t j{0}; j < N; ++j) {
            copyToBuffer(idx, body, gridAmerCalls(i, j).getPrice());
            copyToBuffer(idx, body, gridAmerPuts(i, j).getPrice());
            copyToBuffer(idx, body, gridEuroCalls(i, j).getPrice());
            copyToBuffer(idx, body, gridEuroPuts(i, j).getPrice());
        }
    }

    httpResp->setBody(std::move(body));
    callback(httpResp);
}

void HeatMapController::copyToBuffer(std::size_t& idx, std::string& body, double optionPrice) {
    std::memcpy(&body[idx], &optionPrice, sizeof(optionPrice));
    idx += sizeof(optionPrice);
}

} // namespace server
