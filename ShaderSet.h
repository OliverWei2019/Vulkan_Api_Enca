#pragma once
#ifndef VK_SHADERSET_H
#define VK_SHADERSET_H
#include <vector>
#include <map>
#include <list>
#include "VKApp.h"
#include "VKUtil.h"
#include "VKUniformBuffer.h"
#include "VKImageView.h"
#include "VKDescriptorSet.h"

//class VKImageView;
//class VKDescriptorSet;
//class VKUniformBuffer;
//class VKApp;

class VKShaderSet
{
public:
    VKShaderSet() = delete;
    VKShaderSet(VKApp* vkApp):app(vkApp) {

    }
    ~VKShaderSet(){}
public:
    void release() {
        for (auto itr = shaderStageCreateInfos.begin(); itr != shaderStageCreateInfos.end(); itr++)
            vkDestroyShaderModule(app->getDevice()->getLogicalDevice(), itr->module, nullptr);

        for (auto uniform : uniformBuffers)
            uniform->release();
        uniformBuffers.clear();
        delete this;
    }

    void appendVertexAttributeDescription(uint32_t index, uint32_t size, VkFormat format,
        uint32_t offset, uint32_t binding = 0) {
        VkVertexInputAttributeDescription attributeDescription{};
        attributeDescription.binding = binding;
        attributeDescription.format = format;
        attributeDescription.location = index;
        attributeDescription.offset = offset;
        vertexInputAttributeDescriptions.push_back(attributeDescription);
    }
    void appendVertexInputBindingDescription(uint32_t stride, uint32_t binding = 0,
        VkVertexInputRate inputRate = VK_VERTEX_INPUT_RATE_VERTEX) {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = binding;
        bindingDescription.stride = stride;
        bindingDescription.inputRate = inputRate;
        vertexInputBindingDescriptions.push_back(bindingDescription);
    }

    size_t getVertexAttributeDescriptionCount() {
        return vertexInputAttributeDescriptions.size();
    }
    const VkVertexInputAttributeDescription* getVertexAttributeDescriptionData() {
        return vertexInputAttributeDescriptions.data();
    }

    size_t getVertexInputBindingDescriptionCount() {
        return vertexInputBindingDescriptions.size();
    }
    VkVertexInputBindingDescription* getVertexInputBindingDescriptionData() {
        return vertexInputBindingDescriptions.data();
    }
    VkDescriptorSetLayoutBinding createDescriptorSetLayoutBinding(uint32_t bindindex, 
        VkDescriptorType type, 
        VkShaderStageFlagBits flag) {
        VkDescriptorSetLayoutBinding bindingInfo{};
        bindingInfo.binding = bindindex;
        bindingInfo.descriptorCount = 1;
        bindingInfo.descriptorType = type;
        bindingInfo.stageFlags = flag;

        return bindingInfo;
    }
    void addDescriptorSetLayoutBinding(const VkDescriptorSetLayoutBinding& binding) {
        descriptorSetLayoutBindings.push_back(binding);
        VkDescriptorPoolSize poolSize;
        poolSize.descriptorCount = 0;
        poolSize.type = binding.descriptorType;
        descriptorPoolSizes.push_back(poolSize);
    }
    size_t getDescriptorSetLayoutBindingCount() {
        return descriptorSetLayoutBindings.size();
    }
    const VkDescriptorSetLayoutBinding* getDescriptorSetLayoutBindingData() {
        return descriptorSetLayoutBindings.data();
    }

    size_t getDescriptorPoolSizeCount() {
        return descriptorPoolSizes.size();
    }
    const VkDescriptorPoolSize* getDescriptorPoolSizeData() {
        return descriptorPoolSizes.data();
    }
    void updateDescriptorPoolSize(int32_t size) {
        if (size > 1) {
            for (size_t i = 0; i < descriptorPoolSizes.size(); i++) {
                descriptorPoolSizes[i].descriptorCount = static_cast<uint32_t>(size);
            }
        }
        else {
            throw std::runtime_error("poolSize descriptor count is not greater than 0!");
        }
    }

    bool addShader(const std::string& spvFile, VkShaderStageFlagBits shaderStage,
        const char* entryPoint = "main") {
        auto Shader = createShaderModule(spvFile);
        for (size_t i = 0; i < shaderStageCreateInfos.size(); i++) {
            if (shaderStageCreateInfos[i].stage == shaderStage) {
                return false;
            }
        }
        
        VkPipelineShaderStageCreateInfo shaderStageInfo = {};
        shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageInfo.stage = shaderStage;//可编程阶段内
        //着色器类别和入口，一个shader module可以集合多个shaders
        shaderStageInfo.module = Shader;
        shaderStageInfo.pName = entryPoint;

        shaderStageCreateInfos.push_back(shaderStageInfo);
        return true;
    }
public:
    VKUniformBuffer* addUniformBuffer(uint32_t binding, uint32_t bufferSize) {
        auto buffer = new VKUniformBuffer(app, binding, bufferSize);
        buffer->initBuffer(app->getSwapChain()->getSwapChainSize());
        uniformBuffers.push_back(buffer);
        return buffer;
    }
    void initUniformBuffer() {
        for (auto uniform : uniformBuffers) {
            uniform->initBuffer(app->getSwapChain()->getSwapChainSize());
        }
    }
    void clearUniformBuffer() {
        for (auto uniform : uniformBuffers) {
            uniform->clearBuffer();
        }
    }

    void addImageView(VKImageView* imageView, uint32_t binding = 1) {
        if (imageView) {
            auto find = imageViews.find(imageView);
            if (find != imageViews.end()) {
                return;
            }
            imageViews.insert(std::make_pair(imageView, binding));
        }
    }

    //determine whelther has vertex shader and fragment shader or not?
    bool isValid() {
        bool hasVert = false;
        bool hasFrag = false;
        for (size_t i = 0; i < shaderStageCreateInfos.size(); i++) {
            if (shaderStageCreateInfos[i].stage == VK_SHADER_STAGE_VERTEX_BIT) {
                hasVert = true;
            }
            if (shaderStageCreateInfos[i].stage == VK_SHADER_STAGE_FRAGMENT_BIT) {
                hasFrag = true;
            }
        }
        return hasVert && hasFrag;
    }
    VkPipelineShaderStageCreateInfo* getCreateInfoData() {
        return shaderStageCreateInfos.data();
    }
    size_t getCreateInfoCount() {
        return shaderStageCreateInfos.size();
    }

    void updateDescriptorSet(std::shared_ptr<DescriptorSets> descriptorSet) {
        descriptorSet->update(uniformBuffers, imageViews);
    }
    //update uniformbuffer and change shader 
    void update(uint32_t index) {
        for (auto uniform : uniformBuffers) {
            uniform->update(index);
        }
    }
private:
    VkShaderModule createShaderModule(const std::string& spvFile) {
        VkShaderModule shaderModule;
        auto shaderCode = readDataFromFile(spvFile);
        
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = shaderCode.size();
        //字节码的指针是uint32_t 类型，而不是char 类型。
        createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

        if (vkCreateShaderModule(app->getDevice()->getLogicalDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }
        return shaderModule;
    }
private:
    VKApp* app = nullptr;

    std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions;
    std::vector<VkVertexInputBindingDescription> vertexInputBindingDescriptions;

    std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos;

    std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;
    std::vector<VkDescriptorPoolSize> descriptorPoolSizes;

    std::list<VKUniformBuffer*> uniformBuffers;
    std::map<VKImageView*, uint32_t> imageViews;
};

#endif // VK_SHADERSET_H