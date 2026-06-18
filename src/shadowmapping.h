#pragma once

struct ShadowMapping {
	VkImage shadowDepthImage;
	VkDeviceMemory shadowDepthImageMemory;
	VkImageView shadowDepthImageView;
	VkRenderPass shadowRenderPass;
	VkSampler shadowSampler;
	VkFramebuffer shadowFramebuffer;
};