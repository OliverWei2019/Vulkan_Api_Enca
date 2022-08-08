#pragma once
#ifndef VK_SUBPASS_H
#define VK_SUBPASS_H
#include <vector>
#include <vulkan/vulkan.h>
#include "VKApp.h"
#include "VMAllocator.h"
#include "VKUtil.h"

struct attachmentsRef {
	std::vector< VkAttachmentReference> colorAttachRef;
	std::vector< VkAttachmentReference> depthAttachRef;
	std::vector< VkAttachmentReference> inputAttachRef;
};
class subpassSet {
public:
	subpassSet(VKApp* vkApp) :app(vkApp) {

	}
	~subpassSet() {

	}
public:
	void addAttachRef(attachmentsRef &attachsRef) {
		attachsRefSet.push_back(attachsRef);
	}
	void generateSubpassDescription() {
		subpassDescriptions.resize(attachsRefSet.size());
		for (size_t i = 0; i < attachsRefSet.size(); i++) {
			subpassDescriptions[i].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			if (!attachsRefSet[i].colorAttachRef.empty())
			{
				subpassDescriptions[i].colorAttachmentCount = static_cast<uint32_t>(attachsRefSet[i].colorAttachRef.size());
				subpassDescriptions[i].pColorAttachments = attachsRefSet[i].colorAttachRef.data();
			}
			if (!attachsRefSet[i].depthAttachRef.empty())
			{
				subpassDescriptions[i].pDepthStencilAttachment = attachsRefSet[i].depthAttachRef.data();
			}
			if (!attachsRefSet[i].inputAttachRef.empty()) {
				subpassDescriptions[i].inputAttachmentCount = attachsRefSet[i].inputAttachRef.size();
				subpassDescriptions[i].pInputAttachments = attachsRefSet[i].inputAttachRef.data();
			}
		}
	}
	void release() {
		if (!attachsRefSet.empty()) {
			for (auto attachRef : attachsRefSet) {
				if(!attachRef.colorAttachRef.empty())
					attachRef.colorAttachRef.clear();
				if (!attachRef.depthAttachRef.empty())
					attachRef.depthAttachRef.clear();
				if (!attachRef.inputAttachRef.empty())
					attachRef.inputAttachRef.clear();
			}
			attachsRefSet.clear();
		}
		if (!subpassDescriptions.empty()) {
			subpassDescriptions.clear();
		}
		delete this;
	}
	uint32_t getSubpassCount() {
		return static_cast<uint32_t>(subpassDescriptions.size());
	}
	VkSubpassDescription* getSubpassDescriptionData() {
		return subpassDescriptions.data();
	}
private:
	VKApp* app = nullptr;
	std::vector<attachmentsRef> attachsRefSet;
	std::vector<VkSubpassDescription> subpassDescriptions;
};
#endif // !VK_SUBPASS_H
