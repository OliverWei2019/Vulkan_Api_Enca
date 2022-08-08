#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>
#include <chrono>
#include "Application.h"
VKApp* app = nullptr;
Pipeline* pipeline = nullptr;

//uniform 
struct UniformBufferObeject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

uint32_t updateUniformBufferData(char*& data, uint32_t size) {
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
    UniformBufferObeject ubo{};
    //rotate矩阵需要提供，初始矩阵，旋转角，旋转轴
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    //lookat矩阵View变换矩阵需要提供，相机位置，相机朝向，快门方向
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    //perspective投影变换矩阵需要提供 视角，宽高比，近远平面位置
    auto extent = app->getSwapChain()->getSwapChainExtent();
    ubo.proj = glm::perspective(glm::radians(45.0f), (float)extent.width / (float)extent.height, 0.1f, 10.0f);
    //glm中Y轴倒置
    ubo.proj[1][1] *= -1;
    //std::cout << static_cast<void*>(&ubo) << std::endl;
    //std::cout << static_cast<void*>(&ubo.model[0][0]) << std::endl;
    memcpy(data, &ubo.model[0][0], size);
    //std::cout << sizeof(ubo) << std::endl;
    return sizeof(ubo);
}

int main() {
    VKAppConfig appConfig;
    appConfig.name = "test1";
    appConfig.debug = true;
    Config config;

    app = creatApp(appConfig, config);
    app->initWindow();

    app->initVulkanRHI(config);

    auto shaders = app->createShaderSet();
    shaders->addShader("D:/VulKan/Vulkanstart/shaders/vert1.spv", VK_SHADER_STAGE_VERTEX_BIT);
    shaders->addShader("D:/VulKan/Vulkanstart/shaders/frag1.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    //pos
    shaders->appendVertexAttributeDescription(0, sizeof(float) * 3, VK_FORMAT_R32G32B32_SFLOAT, 0);
    //color
    shaders->appendVertexAttributeDescription(1, sizeof(float) * 3, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3);
    //textColor
    shaders->appendVertexAttributeDescription(2, sizeof(float) * 2, VK_FORMAT_R32G32_SFLOAT,sizeof(float) * 6);
    //binding
    shaders->appendVertexInputBindingDescription(sizeof(float) * 8, 0, VK_VERTEX_INPUT_RATE_VERTEX);

    //shaders中的统一缓冲区和采样器布局
    auto uniformBinding = shaders->createDescriptorSetLayoutBinding(0,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 
        VK_SHADER_STAGE_VERTEX_BIT);
    shaders->addDescriptorSetLayoutBinding(uniformBinding);
    auto samplerBinding = shaders->createDescriptorSetLayoutBinding(1,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 
        VK_SHADER_STAGE_FRAGMENT_BIT);

    //auto samplerInfo = initializers::createSamplerCreateInfo(0);
    //auto sampler = app->createSampler(samplerInfo);
    //samplerBinding.pImmutableSamplers = sampler->getSampler();

    shaders->addDescriptorSetLayoutBinding(samplerBinding);

    if (!shaders->isValid()) {
        std::cerr << "shader set is invalid!" << std::endl;
        shaders->release();
        app->release();
        return -1;
    }
    auto ubo = shaders->addUniformBuffer(0, sizeof(float) * 16 * 3);
    ubo->setWriteDataCallback(updateUniformBufferData);

    auto texture = app->createImage("D:/VulKan/Vulkanstart/textures/viking_room.png");
    auto samplerInfo = initializers::createSamplerCreateInfo(texture->getMipLevel());
    auto sampler = app->createSampler(samplerInfo);

    auto textureViewInfo = initializers::createImageViewCreateInfo();
    textureViewInfo.image = texture->getImage();
    textureViewInfo.format = texture->getImageFormat();
    textureViewInfo.subresourceRange.levelCount = texture->getMipLevel();
    auto textureView = app->createImageView(textureViewInfo);
    textureView->setLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    textureView->setSampler(*(sampler->getSampler()));
    shaders->addImageView(textureView);


    app->initVulkanRenderFrame();

    pipeline = app->createPipeline(shaders);
    //pipeline->prepare();
    pipeline->getDynamicState()->addDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
    pipeline->getDynamicState()->addDynamicState(VK_DYNAMIC_STATE_SCISSOR);
    pipeline->create();
    pipeline->getDynamicState()->applyDynamicViewport({ 0, 0, 480, 480, 0, 1 });
    pipeline->getDynamicState()->applyDynamicScissor({ {0,0},{480,480} });
    auto vertexbuffer = app->createVertexBuffer("D:/VulKan/Vulkanstart/models/viking_room.obj", true, false);
    pipeline->addRenderBuffer(vertexbuffer);
    app->recordCommandBuffers();

    try {
        app->run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        app->release();
        return EXIT_FAILURE;
    }
    app->release();
    std::cout << "iiii" << std::endl;
    return EXIT_SUCCESS;
}