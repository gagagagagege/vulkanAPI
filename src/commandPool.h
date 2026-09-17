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

		// 单个命令缓冲
		static vk::raii::CommandBuffer createCommandBuffer(vk::raii::CommandPool& commandPool, vk::raii::Device& device);

		// 只收"要画上去的那一张图 + 它的 view"和"要绑的那一个描述符集",
		// 不收下标、也不收整套 vector。签名里一个下标都不剩 ——
		// image 和 view 类型不同,传反了编译不过。
		static void recordCommandBuffer(vk::raii::CommandBuffer& commandBuffer,
			vk::Image targetImage, vk::raii::ImageView& targetView,
			vk::Extent2D& swapChainExtent, vk::raii::Pipeline& graphicsPipeline, vk::raii::Buffer& vertexBuffer,
			const std::vector<Vertex>& vertices, vk::raii::Buffer& indexBuffer, const std::vector<uint16_t>& indices,
			vk::raii::PipelineLayout& pipelineLayout, vk::raii::DescriptorSet& descriptorSet);

		static vk::raii::CommandBuffer beginSingleTimeCommands(vk::raii::CommandPool& commandPool, vk::raii::Device& device);
		static void endSingleTimeCommands(vk::raii::CommandBuffer& commandBuffer, vk::raii::Queue& graphicsQueue);
	};

}
