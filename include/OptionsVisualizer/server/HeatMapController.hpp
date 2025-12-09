#pragma once

#include <drogon/HttpResponse.h>
#include <drogon/HttpSimpleController.h>
#include <functional>

namespace server {

class HeatMapController : public drogon::HttpSimpleController<HeatMapController> {
public:
    virtual void asyncHandleHttpRequest(const drogon::HttpRequestPtr& req,
                                        std::function<void(const drogon::HttpResponsePtr&)>&& callback) override;

private:
    static void copyToBuffer(std::size_t& idx, std::string& body, double optionPrice);

    PATH_LIST_BEGIN
    PATH_ADD("/heatmap", drogon::Post);
    PATH_LIST_END
};

} // namespace server
