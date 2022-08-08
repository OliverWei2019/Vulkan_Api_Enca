#pragma once
#ifndef VK_BUFFER_H
#define VK_BUFFER_H
#include <vector>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "VKApp.h"
#include "PoolBase.h"

class VKBuffer {
public:
	VKBuffer() = delete;
	VKBuffer(VKApp* vkApp, VkDeviceSize size, bool needupload,
		VkBufferUsageFlags usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VmaMemoryUsage memusage = VMA_MEMORY_USAGE_AUTO) :app(vkApp) {

		VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		bufferInfo.size = size;
		bufferInfo.usage = usage;

		VmaAllocationCreateInfo allocCreateInfo = {};
		allocCreateInfo.usage = memusage;
		if (needupload) {
			allocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT ;
		}
		//VkBuffer buffer;
		VmaAllocationInfo allocInfo;
		if (vmaCreateBuffer(app->getAllocator(), &bufferInfo, &allocCreateInfo, &buffer, &allocation, &allocInfo) != VK_SUCCESS) {
			throw std::runtime_error("Faild to create buffer!");
		}
	}

	~VKBuffer() {

	}
public:
	void release() {
		if (buffer) {
			vmaDestroyBuffer(app->getAllocator(), buffer, allocation);
			buffer = nullptr;
		}
		delete this;
	}
	VKApp* getApp() {
		return app;
	}
	VkBuffer getBuffer() {
		return buffer;
	}
	VmaAllocation getAllocation() {
		return allocation;
	}
	void setAllocation(VmaAllocation& alloc) {
		if (alloc) {
			allocation = alloc;
		}
	}
	void copyBufferFrom(VkBuffer& srcBuffer, VkDeviceSize size) {
		VkCommandBuffer commandBuffer = app->getCmdPool()->beginSingleTimeCommands();

		VkBufferCopy copyRegion{};
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, buffer, 1, &copyRegion);

		app->getCmdPool()->endSingleTimeCommands(commandBuffer, app->getDevice()->getGraphicsQueue());
	}
	
private:
	VKApp* app = nullptr;
	VkBuffer buffer;
	VmaAllocation allocation;
};



class BufferBase {
public:
	BufferBase(VKApp* vkApp) :app(vkApp) {

	}
	virtual ~BufferBase() {

	}
public:
	virtual void release() {
		if (buffer) {
			buffer->release();
			buffer = nullptr;
		}
		delete  this;
	}
	VkBuffer getBuffer() {
		if(buffer)
			return buffer->getBuffer();
		return VK_NULL_HANDLE;
	}
	VmaAllocation getVmaAllocation() {
		if (buffer)
			return buffer->getAllocation();
		return VK_NULL_HANDLE;
	}
	virtual void render(VkCommandBuffer command) = 0;
public:
	VKApp* app = nullptr;
	VKBuffer* buffer = nullptr;
};

#endif // VK_BUFFER_H