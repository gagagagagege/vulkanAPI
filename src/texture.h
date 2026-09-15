#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan_raii.hpp>

#include <stb_image.h>

#include <utility>

#include"swapChain.h"

namespace Fish {
	class vkContext;

	class texture
	{
	public:
		static void createTextureImage(vkContext* context, vk::raii::Image& textureImage,
			vk::raii::DeviceMemory& textureImageMemory, vk::raii::CommandPool& commandPool);

		static std::pair<vk::raii::Image, vk::raii::DeviceMemory> createImage(vk::raii::Device& device, vk::raii::PhysicalDevice& physicalDevice,
			const vk::Format& format, uint32_t width, uint32_t height, const vk::ImageTiling& tiling,
			const vk::Flags<vk::ImageUsageFlagBits>& usage, const vk::MemoryPropertyFlags& properties);

		static void transitionImageLayout(vk::raii::CommandBuffer& commandBuffer, const vk::raii::Image& image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
		static void copyBufferToImage(vk::raii::CommandBuffer& commandBuffer, const vk::raii::Buffer& buffer, vk::raii::Image& image, uint32_t width, uint32_t height);

		static void createTextureImageView(vk::raii::Image& textureImage, vk::raii::ImageView& textureImageView, vk::raii::Device& device);
		static void createTextureSampler(vk::raii::PhysicalDevice& physicalDevice, vk::raii::Sampler& textureSampler, vk::raii::Device& device);
	public:
	
	};

}
