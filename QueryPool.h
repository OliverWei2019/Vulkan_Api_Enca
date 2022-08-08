#pragma once
#ifndef QUERYPOOL_H
#define QUERYPOOL_H
#pragma once
//#include <vulkan/vulkan.h>
#include <vector>
#include <functional>
#include "VKApp.h"
#include "VKDevices.h"
//#include <VK_QueryPool.h>

class VKQueryPool
{
public:
    VKQueryPool() = delete;
    VKQueryPool(VKApp* vkApp, uint32_t count, VkQueryPipelineStatisticFlags flag):app(vkApp) {
        VkQueryPoolCreateInfo queryPoolCreateInfo{};
        queryPoolCreateInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
        queryPoolCreateInfo.queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS;
        queryPoolCreateInfo.queryCount = count;
        queryPoolCreateInfo.pipelineStatistics = flag;
        auto status = vkCreateQueryPool(app->getDevice()->getLogicalDevice(), &queryPoolCreateInfo,nullptr,&queryPool);
        assert(status == VK_SUCCESS);
        queryData.push_back(count);

    }
public:
    void release() {
        if (queryPool) {
            vkDestroyQueryPool(app->getDevice()->getLogicalDevice(), queryPool, nullptr);
        }
        queryPool = nullptr;
    }
    void reset(VkCommandBuffer commandBuffer) {
        vkCmdResetQueryPool(commandBuffer, queryPool, 0, queryData.size());
    }
    void startQeury(VkCommandBuffer commandBuffer) {
        vkCmdBeginQuery(commandBuffer, queryPool, 0, 0);
    }
    void endQuery(VkCommandBuffer commandBuffer) {
        vkCmdEndQuery(commandBuffer, queryPool, 0);
    }
    void setQueryCallback(std::function<void(const std::vector<uint64_t>& data)> fn) {
        if (fn)
            queryCallback = fn;
    }

    void query() {
        queryData.assign(queryData.size(), 0);

        vkGetQueryPoolResults(
            app->getDevice()->getLogicalDevice(),
            queryPool,
            0, 1, sizeof(uint64_t) * queryData.size(), queryData.data(), sizeof(uint64_t),
            VK_QUERY_RESULT_64_BIT
        );

        if (queryCallback)
            queryCallback(queryData);
    }
private:
    VKApp* app = nullptr;
    VkQueryPool queryPool = nullptr;
    std::vector<uint64_t> queryData;
    std::function<void(const std::vector<uint64_t>& data)> queryCallback;
};
#endif // !QUERYPOOL_H
