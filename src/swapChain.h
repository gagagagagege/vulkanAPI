#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan_raii.hpp>
#include <GLFW/glfw3.h>

#include "PhysicalDevice.h"

#include <vector>

#include <cstdint>
#include <limits>
#include <algorithm>

namespace Fish {
	class swapChain
	{
	public:
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR> presentModes;
		vk::SurfaceCapabilitiesKHR capabilities;

	public:
		static swapChain querySwapChainSupport(const vk::raii::PhysicalDevice& device, const vk::raii::SurfaceKHR& surface);
		static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
		static vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
		static vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

		static void createSwapChain(vk::raii::PhysicalDevice& physicalDevice, vk::raii::SurfaceKHR& surface,
			GLFWwindow* window, vk::raii::Device& device, vk::raii::SwapchainKHR& swapChainHandle,
			std::vector<vk::Image>& swapChainImages, vk::Format& swapChainImageFormat,
			vk::Extent2D& swapChainExtent, vk::SwapchainKHR oldSwapchain = nullptr);

		static void recreateSwapChain(vk::raii::PhysicalDevice& physicalDevice, vk::raii::SurfaceKHR& surface,
			GLFWwindow* window, vk::raii::Device& device, vk::raii::SwapchainKHR& swapChainHandle,
			std::vector<vk::Image>& swapChainImages, vk::Format& swapChainImageFormat,
			vk::Extent2D& swapChainExtent, std::vector<vk::raii::ImageView>& swapChainImageViews);

		static void cleanupSwapChain(std::vector<vk::raii::ImageView>& swapChainImageViews, vk::raii::SwapchainKHR& swapChainHandle);

		// 交换链 image 归呈现引擎所有,构造不出 Image 对象,
		// 所以这边只借 Image::createView 包一层,view 自身由调用方持有。
		static void createImageViews(const std::vector<vk::Image>& swapChainImages, std::vector<vk::raii::ImageView>& swapChainImageViews,
			vk::Format swapChainImageFormat, vk::raii::Device& device);
	};

}
