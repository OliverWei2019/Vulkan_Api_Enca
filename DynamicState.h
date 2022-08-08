#pragma once
#ifndef VK_DYNAMICSTATE_H
#define VK_DYNAMICSTATE_H
#include <optional>
#include <tuple>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <vulkan/vulkan.h>
#include "PipeLineBase.h"

//#include <VK_DynamicState.h>

class DynamicState
{
public:
    DynamicState() = delete;
    DynamicState(PipeLineBase* pipeLine) :pipeline(pipeLine) {

    }
public:
    void addDynamicState(VkDynamicState dynamicState) {
        dynamicStates.push_back(dynamicState);
    }
    VkPipelineDynamicStateCreateInfo createDynamicStateCreateInfo(VkPipelineDynamicStateCreateFlags flags) {
        VkPipelineDynamicStateCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        createInfo.flags = flags;
        createInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        createInfo.pDynamicStates = dynamicStates.data();
        return createInfo;
    }
    //录制cmdbuffer时，用上dynamic states
    void apply(VkCommandBuffer buffer) {
        if (viewport.has_value())
            vkCmdSetViewport(buffer, 0, 1, &viewport.value());

        if (scissor.has_value())
            vkCmdSetScissor(buffer, 0, 1, &scissor.value());

        if (lineWidth.has_value()) {
            vkCmdSetLineWidth(buffer, lineWidth.value());
        }

        if (depthBias.has_value()) {
            auto value = depthBias.value();
            vkCmdSetDepthBias(buffer, value[0], value[1], value[2]);
        }

        if (blendConstant.has_value()) {
            auto value = blendConstant.value();
            vkCmdSetBlendConstants(buffer, &value[0]);
        }

        if (depthBounds.has_value()) {
            auto value = depthBounds.value();
            vkCmdSetDepthBounds(buffer, value[0], value[1]);
        }

        if (stencilCompareMask.has_value()) {
            auto value = stencilCompareMask.value();
            vkCmdSetStencilCompareMask(buffer, std::get<0>(value), std::get<1>(value));
        }

        if (stencilWriteMask.has_value()) {
            auto value = stencilWriteMask.value();
            vkCmdSetStencilWriteMask(buffer, std::get<0>(value), std::get<1>(value));
        }

        if (stencilReferenceMask.has_value()) {
            auto value = stencilReferenceMask.value();
            vkCmdSetStencilReference(buffer, std::get<0>(value), std::get<1>(value));
        }
    }

    void release() {
        delete this;
    }

    //apply新的states,pipeline准备更新
    void applyDynamicViewport(const VkViewport& inputviewport) {
        viewport = inputviewport;
        pipeline->setNeedRecreate();
    }
    void applyDynamicScissor(const VkRect2D& inputscissor) {
        scissor = inputscissor;
        pipeline->setNeedRecreate();
    }
    void applyDynamicLineWidth(float inputlineWidth) {
        lineWidth = inputlineWidth;
        pipeline->setNeedRecreate();
    }
    void applyDynamicDepthBias(float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor) {
        depthBias = glm::vec3(depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
        pipeline->setNeedRecreate();
    }
    void applyDynamicBlendConstants(const float blendConstants[4]) {
        blendConstant = glm::vec4(blendConstants[0], blendConstants[1], blendConstants[2], blendConstants[3]);
        pipeline->setNeedRecreate();
    }
    void applyDynamicDepthBounds(float minDepthBounds, float maxDepthBounds) {
        depthBounds = glm::vec2(minDepthBounds, maxDepthBounds);
        pipeline->setNeedRecreate();
    }
    void applyDynamicStencilCompareMask(VkStencilFaceFlags faceMask, uint32_t compareMask) {
        stencilCompareMask = { faceMask ,compareMask };
        pipeline->setNeedRecreate();
    }
    void applyDynamicStencilWriteMask(VkStencilFaceFlags faceMask, uint32_t writeMask) {
        stencilWriteMask = { faceMask ,writeMask };
        pipeline->setNeedRecreate();
    }
    void applyDynamicStencilReference(VkStencilFaceFlags faceMask, uint32_t reference) {
        stencilReferenceMask = { faceMask ,reference };
        pipeline->setNeedRecreate();
    }
private:
    PipeLineBase* pipeline = nullptr;

    std::vector<VkDynamicState> dynamicStates;
    VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo{};

    std::optional<VkViewport> viewport;
    std::optional<VkRect2D> scissor;
    std::optional<float> lineWidth;
    std::optional<glm::vec3> depthBias;
    std::optional<glm::vec4> blendConstant;
    std::optional<glm::vec2> depthBounds;
    std::optional<std::tuple<VkStencilFaceFlags, uint32_t>> stencilCompareMask;
    std::optional<std::tuple<VkStencilFaceFlags, uint32_t>> stencilWriteMask;
    std::optional<std::tuple<VkStencilFaceFlags, uint32_t>> stencilReferenceMask;
};

#endif // VK_DYNAMICSTATE_H