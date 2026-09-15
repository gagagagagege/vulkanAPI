#pragma once

#define VK_USE_PLATFORM_WIN32_KHR

//#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include<glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include"vulkan/vulkan.h"
#include <vulkan/vulkan_raii.hpp>

#include <chrono>

#include"buffer.h"

namespace Fish {
	struct UniformBufferObject {
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

	class uniformBuffer
	{
	public:
		static void createDescriptorSetLayout(vk::raii::DescriptorSetLayout& descriptorSetLayout, vk::raii::Device& device);
		static void createUniformBuffers(const int& MAX_FRAMES_IN_FLIGHT, std::vector<Buffer>& uniformBuffers, vkContext* context);
		static void updateUniformBuffer(uint32_t currentImage, vk::Extent2D& swapChainExtent, std::vector<Buffer>& uniformBuffers);
		static vk::raii::DescriptorPool createDescriptorPool(uint32_t MAX_FRAMES_IN_FLIGHT, vk::raii::Device& device);
		static void createDescriptorSets(uint32_t MAX_FRAMES_IN_FLIGHT, std::vector<Buffer>& uniformBuffers,
			vk::raii::DescriptorSetLayout& descriptorSetLayout, vk::raii::DescriptorPool& descriptorPool, vk::raii::Device& device,
			std::vector<vk::raii::DescriptorSet>& descriptorSets, vk::raii::ImageView& textureImageView,
			vk::raii::Sampler& textureSampler);
	public:
		UniformBufferObject ubo;
	};

}