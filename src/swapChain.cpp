#include "swapChain.h"
#include <stdexcept>
#include <iostream>
#include <utility>

namespace Fish {
	swapChain swapChain::querySwapChainSupport(const vk::raii::PhysicalDevice& device, const vk::raii::SurfaceKHR& surface)
	{
		swapChain details;

		details.capabilities = device.getSurfaceCapabilitiesKHR(*surface);
		details.formats = device.getSurfaceFormatsKHR(*surface);
		details.presentModes = device.getSurfacePresentModesKHR(*surface);

		return details;
	}
	vk::SurfaceFormatKHR swapChain::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == vk::Format::eB8G8R8A8Srgb
				&& availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
				return availableFormat;
			}
		}
		return availableFormats[0];
	}
	vk::PresentModeKHR swapChain::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
	{
		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
				return availablePresentMode;
			}
		}

		return vk::PresentModeKHR::eFifo;
	}
	vk::Extent2D swapChain::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, GLFWwindow* window)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;
		}
		else {
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);

			vk::Extent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}
	void swapChain::createSwapChain(vk::raii::PhysicalDevice& physicalDevice, vk::raii::SurfaceKHR& surface,
		GLFWwindow* window, vk::raii::Device& device, vk::raii::SwapchainKHR& swapChainHandle,
		std::vector<vk::Image>& swapChainImages, vk::Format& swapChainImageFormat,
		vk::Extent2D& swapChainExtent, vk::SwapchainKHR oldSwapchain)
	{
		swapChain support = querySwapChainSupport(physicalDevice, surface);

		vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(support.formats);
		vk::PresentModeKHR presentMode = chooseSwapPresentMode(support.presentModes);
		vk::Extent2D extent = chooseSwapExtent(support.capabilities, window);

		uint32_t imageCount = support.capabilities.minImageCount + 1;
		if (support.capabilities.maxImageCount > 0 && imageCount > support.capabilities.maxImageCount) {
			imageCount = support.capabilities.maxImageCount;
		}

		vk::SwapchainCreateInfoKHR createInfo{};
		createInfo.surface = *surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

		QueueFamilyIndices indices = QueueFamilyIndices::findQueueFamilies(physicalDevice, surface);
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		if (indices.graphicsFamily != indices.presentFamily) {
			createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = vk::SharingMode::eExclusive;//独占
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}

		createInfo.preTransform = support.capabilities.currentTransform;
		createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;//忽略a与其他窗口的混合
		createInfo.presentMode = presentMode;
		createInfo.clipped = vk::True;//启用裁剪
		createInfo.oldSwapchain = oldSwapchain;//调整窗口大小时传入旧交换链,实现原子重建

		swapChainHandle = vk::raii::SwapchainKHR(device, createInfo);
		swapChainImages = swapChainHandle.getImages();

		swapChainImageFormat = surfaceFormat.format;
		swapChainExtent = extent;
	}
	void swapChain::recreateSwapChain(vk::raii::PhysicalDevice& physicalDevice, vk::raii::SurfaceKHR& surface,
		GLFWwindow* window, vk::raii::Device& device, vk::raii::SwapchainKHR& swapChainHandle,
		std::vector<vk::Image>& swapChainImages, vk::Format& swapChainImageFormat,
		vk::Extent2D& swapChainExtent, std::vector<vk::raii::ImageView>& swapChainImageViews)
	{
		int width = 0, height = 0;
		glfwGetFramebufferSize(window, &width, &height);
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}

		device.waitIdle();

		// 传入旧交换链作为 oldSwapchain 原子重建;先用临时句柄创建,
		// 创建失败时旧交换链仍保持完整,下帧重试,避免 resize 期间抛异常导致窗口关闭。
		vk::SwapchainKHR oldSwapchain = *swapChainHandle;

		vk::raii::SwapchainKHR newSwapchain = nullptr;
		try {
			createSwapChain(physicalDevice, surface, window, device, newSwapchain, swapChainImages, swapChainImageFormat, swapChainExtent, oldSwapchain);
		}
		catch (const std::exception& e) {
			std::cerr << "swap chain recreation deferred: " << e.what() << '\n';
			return;
		}

		swapChainImageViews.clear();
		swapChainHandle = std::move(newSwapchain);
		imageView::createImageViews(swapChainImages, swapChainImageViews, swapChainImageFormat, device);
	}
	void swapChain::cleanupSwapChain(std::vector<vk::raii::ImageView>& swapChainImageViews, vk::raii::SwapchainKHR& swapChainHandle)
	{
		swapChainImageViews.clear();
		swapChainHandle = nullptr;
	}

	vk::raii::ImageView imageView::createImageView(vk::Image const& image, vk::Format format, vk::raii::Device& device)
	{
		vk::ImageViewCreateInfo viewInfo{
	  .image = image,
	  .viewType = vk::ImageViewType::e2D,
	  .format = format,
	  .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1} };
		return vk::raii::ImageView(device, viewInfo);
	}

	void imageView::createImageViews(const std::vector<vk::Image>& swapChainImages, std::vector<vk::raii::ImageView>& swapChainImageViews,
		vk::Format swapChainImageFormat, vk::raii::Device& device)
	{
		assert(swapChainImageViews.empty());

		swapChainImageViews.reserve(swapChainImages.size());
		for (auto& image : swapChainImages)
		{
			swapChainImageViews.emplace_back(createImageView(image, swapChainImageFormat,device));
		}
	}
}
