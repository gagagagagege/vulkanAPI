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
	// 一个命令池。池是批量的单位 —— 分配任意多个缓冲,销毁时一起回收。
	// 工程里有两个:commandPool 管每帧的命令缓冲,transientPool 管一次性拷贝。
	class CommandPool
	{
	public:
		CommandPool() = default;
		explicit CommandPool(vkContext* context);

		CommandPool(const CommandPool&) = delete;
		CommandPool& operator=(const CommandPool&) = delete;
		CommandPool(CommandPool&&) = default;
		CommandPool& operator=(CommandPool&&) = default;

		// 单个命令缓冲。分配个数不限,池自己不持有它们 ——
		// 谁拿到谁负责在池之前放掉。
		vk::raii::CommandBuffer allocateBuffer() const;

		vk::raii::CommandPool& handle() { return m_pool; }

	private:
		vk::raii::CommandPool m_pool = nullptr;
		vkContext*            m_context = nullptr;
	};

	// ---- 一次性同步拷贝。整个应用共用 transientPool 和 graphicsQueue,
	//      没什么状态可持有,所以是自由函数而不是方法 ----
	vk::raii::CommandBuffer beginSingleTimeCommands(vkContext* context, CommandPool& transientPool);
	void endSingleTimeCommands(vkContext* context, vk::raii::CommandBuffer& commandBuffer);

	// 录制一帧。这里不碰池 —— 缓冲是调用方已经分配好的,所以也是自由函数。
	// 只收"要画上去的那一张图 + 它那个交换链"和"要绑的那一个描述符集",
	// 不收整套 vector。收下标而不是两个句柄 —— image 和 view 都由 SwapChain 出,
	// 必然同代,下标错不了。早先"传两个句柄防传反"的理由已经不成立。
	void recordCommandBuffer(vk::raii::CommandBuffer& commandBuffer,
		SwapChain& swapChain, uint32_t imageIndex, Pipeline& pipeline,
		const Buffer& vertexBuffer, const std::vector<Vertex>& vertices,
		const Buffer& indexBuffer, const std::vector<uint16_t>& indices,
		vk::raii::DescriptorSet& descriptorSet);
}
