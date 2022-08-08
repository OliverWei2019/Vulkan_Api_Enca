#pragma once
#ifndef VK_VERTEXBUFFER_H
#define VK_VERTEXBUFFER_H
#include <cstring>
#include <array>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include "VKApp.h"
#include "Buffer.h"
class Vertex {
    glm::vec3 position;
    glm::vec4 color;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }
    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, position);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);
        return attributeDescriptions;
    }
};

class VertexBuffer : public BufferBase
{
public:
    VertexBuffer() = delete;
    VertexBuffer(VKApp* vkApp) :BufferBase(vkApp) {

    }
    virtual ~VertexBuffer() {

    }
public:
    virtual void release() {
        if(indexBuffer)
            indexBuffer->release();
        indexBuffer = nullptr;
        BufferBase::release();
        //delete this;
    }

    void create(const std::vector<float>& vertices, int32_t size,
        const std::vector<uint32_t>& indices, bool indirect = false) {
        buffer = createBufferData(vertices, true);
        count = vertices.size() / size;
        if (indices.empty()) {
            indexedVertex = false;
        }
        else {
            indexBuffer = createBufferData(indices, false);
            indexedVertex = true;
            count = indices.size() / size;
        }
        indirectDraw = indirect;
    }
    void create(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, bool indirect = false) {
        buffer = createBufferData(vertices, true);
        if (indices.empty()) {
            indexedVertex = false;
        }
        else {
            indexBuffer = createBufferData(indices, false);
            indexedVertex = true;
        }
        indirectDraw = indirect;
    }

    size_t getDataCount() {
        return count;
    }
    void render(VkCommandBuffer command) {
        VkBuffer vertexBuffers[] = { buffer->getBuffer() };
        //VkBuffer indexedBuffer = indexBuffer->getBuffer();
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(command, 0, 1, vertexBuffers, offsets);
        if (!indexedVertex) {
            if (!indirectDraw)
                vkCmdDraw(command, count, 1, 0, 0);
        }
        else {
            VkBuffer indexedBuffer = indexBuffer->getBuffer();
            vkCmdBindIndexBuffer(command, indexedBuffer, 0, VK_INDEX_TYPE_UINT32);
            if (!indirectDraw)
                vkCmdDrawIndexed(command, static_cast<uint32_t>(count), 1, 0, 0, 0);
        }
    }
private:
    template<class T>
    VKBuffer* createBufferData(const std::vector<T>& input,bool vertex)
    {
        auto bufferSize = sizeof(T) * input.size();

        VKBuffer* stagingBuffer = new VKBuffer(app, 
            bufferSize, 
            true, 
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VMA_MEMORY_USAGE_AUTO);

        VmaAllocator allocator = app->getAllocator();
        void* gpuData;
        if (vmaMapMemory(allocator, stagingBuffer->getAllocation(), &gpuData) != VK_SUCCESS) {
            throw std::runtime_error("failed to map staging buffer memory!");
        }
        memcpy(gpuData, input.data(), (size_t)bufferSize);
        vmaUnmapMemory(allocator, stagingBuffer->getAllocation());
        
        VKBuffer* gpuBuffer = new VKBuffer(app,
            bufferSize,
            false,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | (vertex ? VK_BUFFER_USAGE_VERTEX_BUFFER_BIT : VK_BUFFER_USAGE_INDEX_BUFFER_BIT),
            VMA_MEMORY_USAGE_AUTO);
        auto tempBuffer = stagingBuffer->getBuffer();
        gpuBuffer->copyBufferFrom(tempBuffer, bufferSize);
        stagingBuffer->release();
        return gpuBuffer;
    }
private:
    int count = 0;
    int indexedVertex = false;
    VKBuffer* indexBuffer = nullptr;
    bool indirectDraw = false;
};

#endif // VK_VERTEXBUFFER_H