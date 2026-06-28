#pragma once

struct ShadowMapping {
	VkImage shadowDepthImage;
	VkDeviceMemory shadowDepthImageMemory;
	VkImageView shadowDepthImageView;
	VkRenderPass shadowRenderPass;
	VkSampler shadowSampler;
	VkFramebuffer shadowFramebuffer;

	const uint32_t shadowMapWidth = 1024;
	const uint32_t shadowMapHeight = 1024;

	VkPipelineLayout shadowPipelineLayout;
	VkPipeline shadowPipeline;
};