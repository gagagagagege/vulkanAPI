#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan_raii.hpp>

#include "vkContext.h"
#include "Buffer.h"
#include "SwapChain.h"
#include "Pipeline.h"

#include <cstdint>
#include <vector>

namespace Fish {
	class CommandPool
	{
	public:
		CommandPool() = default;
		explicit CommandPool(vkContext* context);

		CommandPool(const CommandPool&) = delete;
		CommandPool& operator=(const CommandPool&) = delete;
		CommandPool(CommandPool&&) = default;
		CommandPool& operator=(CommandPool&&) = default;

		vk::raii::CommandBuffer allocateBuffer() const;

		vk::raii::CommandPool& handle() { return m_pool; }

	private:
		vk::raii::CommandPool m_pool = nullptr;
		vkContext*            m_context = nullptr;
	};

	vk::raii::CommandBuffer beginSingleTimeCommands(vkContext* context, CommandPool& transientPool);
	void endSingleTimeCommands(vkContext* context, vk::raii::CommandBuffer& commandBuffer);

	void recordCommandBuffer(vk::raii::CommandBuffer& commandBuffer,
		SwapChain& swapChain, uint32_t imageIndex, Pipeline& pipeline,
		const Buffer& vertexBuffer, const std::vector<Vertex>& vertices,
		const Buffer& indexBuffer, const std::vector<uint16_t>& indices,
		vk::raii::DescriptorSet& descriptorSet);
}
