#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan_raii.hpp>
#include <GLFW/glfw3.h>
#include <vector>

#include "shader.h"
#include "buffer.h"

namespace Fish {
	class commandPool
	{
	public:
		static void createCommandPool(vk::raii::PhysicalDevice& physicalDevice, vk::raii::SurfaceKHR& surface, vk::raii::Device& device, vk::raii::CommandPool& commandPool);
		static void createCommandBuffers(vk::raii::CommandPool& commandPool, vk::raii::Device& device, std::vector<vk::raii::CommandBuffer>& commandBuffers, int count);
		static void recordCommandBuffer(vk::raii::CommandBuffer& commandBuffer, uint32_t imageIndex, uint32_t frameIndex,
			const std::vector<vk::Image>& swapChainImages, const std::vector<vk::raii::ImageView>& swapChainImageViews,
			vk::Extent2D& swapChainExtent, vk::raii::Pipeline& graphicsPipeline, vk::raii::Buffer& vertexBuffer,
			const std::vector<Vertex>& vertices, vk::raii::Buffer& indexBuffer, const std::vector<uint16_t>& indices,
			vk::raii::PipelineLayout& pipelineLayout, std::vector<vk::raii::DescriptorSet>& descriptorSets);

		static vk::raii::CommandBuffer beginSingleTimeCommands(vk::raii::CommandPool& commandPool, vk::raii::Device& device);
		static void endSingleTimeCommands(vk::raii::CommandBuffer& commandBuffer, vk::raii::Queue& graphicsQueue);
	};

}
