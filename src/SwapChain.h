#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan_raii.hpp>
#include <GLFW/glfw3.h>

#include "vkContext.h"

#include <cstdint>
#include <vector>

namespace Fish {
	// 交换链 + 它派生出来的三样。这四样的边界就是"重建"的边界 ——
	// resize / out-of-date 时一起换,别的一律不动(管线不重建,见 Pipeline)。
	class SwapChain
	{
	public:
		SwapChain() = default;
		// physicalDevice / surface / device 从 context 取;window 只用于取 framebuffer 尺寸
		SwapChain(vkContext* context, GLFWwindow* window);

		SwapChain(const SwapChain&) = delete;
		SwapChain& operator=(const SwapChain&) = delete;
		SwapChain(SwapChain&&) = default;
		SwapChain& operator=(SwapChain&&) = default;

		// 原子重建。失败时保留旧的、下帧重试,不抛出去。
		void recreate();

		vk::raii::SwapchainKHR&       handle()  { return m_swapchain; }
		const std::vector<vk::Image>& images() const { return m_images; }
		vk::raii::ImageView&          imageView(uint32_t i) { return m_imageViews[i]; }
		vk::raii::Semaphore&          renderFinishedSemaphore(uint32_t i) { return m_renderFinishedSemaphores[i]; }
		vk::Format                    imageFormat() const { return m_imageFormat; }
		vk::Extent2D                  extent()      const { return m_extent; }
		uint32_t                      imageCount()  const { return static_cast<uint32_t>(m_images.size()); }

	private:
		void create(vk::raii::SwapchainKHR& out, vk::SwapchainKHR oldSwapchain);
		void createImageViews();
		void createRenderFinishedSemaphores();

		// 声明顺序 = 销毁逆序(同 Image.h):view 引用 swapchain 的 image,
		// 所以要后声明、先销毁。下面三行的销毁顺序是 信号量 → view → swapchain,
		// 和原来 cleanupSwapChain 里 semaphores.clear() / views.clear() / 句柄置空 一致。
		vk::raii::SwapchainKHR           m_swapchain = nullptr;
		std::vector<vk::raii::ImageView> m_imageViews;
		std::vector<vk::raii::Semaphore> m_renderFinishedSemaphores;

		// 非拥有:交换链 image 归呈现引擎,这里只是把句柄取回来,没有析构动作
		std::vector<vk::Image> m_images;
		vk::Format             m_imageFormat = vk::Format::eUndefined;
		vk::Extent2D           m_extent{};

		vkContext*  m_context = nullptr;
		GLFWwindow* m_window  = nullptr;
	};

}
