#pragma once
#ifndef VK_RENDERPASS_H
#define VK_RENDERPASS_H
#include <vulkan/vulkan.h>
#include "VKApp.h"
#include "VKUtil.h"
#include "Attachments.h"
#include "subpass.h"

class VKRenderPass {
public:
    VKRenderPass() = delete;
    VKRenderPass(VKApp* vkApp) :app(vkApp) {

    };
public:
    void create() {
        attachsSet = new AttachmentSet(app);
        attachsSet->generateAttachmentsDescriptionSet();
        attachsSet->createAttachments();

        subpass = new subpassSet(app);
        attachmentsRef attachsRef{};
        //颜色附件引用 + 深度附件引用
        attachsRef.colorAttachRef.push_back({ 0,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
        attachsRef.depthAttachRef.push_back({ 1,VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL });
        //attachsRef.inputAttachRef.push_back({});
        subpass->addAttachRef(attachsRef);
        subpass->generateSubpassDescription();
        //子通道依赖
        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        //渲染通道
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = attachsSet->getAttachmentsCount();
        renderPassInfo.pAttachments = attachsSet->getAttachmentsDescriptionData();
        renderPassInfo.subpassCount = subpass->getSubpassCount();
        renderPassInfo.pSubpasses = subpass->getSubpassDescriptionData();
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(app->getDevice()->getLogicalDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
    }
    VkRenderPass getRenderPass()const
    {
        return renderPass;
    }
    void createFrameBuffers() {
        if (!frameBuffers.empty()) {
            frameBuffers.clear();
        }
        frameBuffers.resize(app->getSwapChain()->getSwapChainSize());
        for (size_t i = 0; i < frameBuffers.size(); i++) {
            std::vector<VkImageView> Attachs = {
            app->getSwapChain()->getSwapChainImageView(i)->getImageView(),
            attachsSet->getDepthAttachs(i)->imageVIew->getImageView(),
            };
            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(Attachs.size());
            framebufferInfo.pAttachments = Attachs.data();
            framebufferInfo.width = app->getSwapChain()->getSwapChainExtent().width;
            framebufferInfo.height = app->getSwapChain()->getSwapChainExtent().height;
            framebufferInfo.layers = 1;
            auto device = app->getDevice()->getLogicalDevice();
            if (vkCreateFramebuffer(device, &framebufferInfo, nullptr,
                &frameBuffers[i]) != VK_SUCCESS) {
                std::cerr << "failed to create framebuffer!" << std::endl;
            }
        }
    }
    void release() {
        if (attachsSet) {
            attachsSet->release();
            attachsSet = nullptr;
        }
        if (subpass) {
            subpass->release();
            subpass = nullptr;
        }
        if (renderPass) {
            vkDestroyRenderPass(app->getDevice()->getLogicalDevice(), renderPass, nullptr);
            renderPass = nullptr;
        }
        if (!frameBuffers.empty()) {
            for (auto buffer:frameBuffers)
            {
                vkDestroyFramebuffer(app->getDevice()->getLogicalDevice(), buffer, nullptr);
            }
            frameBuffers.clear();
        }
        delete this;
    }
    VkFramebuffer getFrameBuffer(uint32_t index) {
        if (frameBuffers.empty()) {
            std::cerr << " frame buffers is empty!" << std::endl;
            return nullptr;
        }
        if (index >= frameBuffers.size()) {
            std::cerr << " index over frame buffer vector size!" << std::endl;
            return nullptr;
        }
        return frameBuffers[index];
    }
    uint32_t getFrameBufferSize() {
        return static_cast<uint32_t>(frameBuffers.size());
    }
    void clearAttachSet() {
        if (attachsSet) {
            attachsSet->release();
            attachsSet = nullptr;
        }
    }
private:
    VKApp* app = nullptr;
    VkRenderPass renderPass = nullptr;
    AttachmentSet* attachsSet = nullptr;
    subpassSet* subpass = nullptr;
    std::vector<VkFramebuffer> frameBuffers;
};

#endif // VK_RENDERPASS_H