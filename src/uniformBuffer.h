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

	// 只管 UBO 本身和它那一份描述符集
	// 份数(每帧飞行一份)不在这里管
	// 这里的函数都只处理单个对象
	class uniformBuffer
	{
	public:
		static void createDescriptorSetLayout(vk::raii::DescriptorSetLayout& descriptorSetLayout, vk::raii::Device& device);

		static vk::raii::DescriptorPool createDescriptorPool(uint32_t frameCount, vk::raii::Device& device);

		// 分配并写好一个描述符集,绑的是调用方传进来的那一份 buffer。
		// TODO: 顺带绑贴图不属于本类职责,见 BUFFER_REFACTOR_TODO.md 第五节第 2 条
		static vk::raii::DescriptorSet createDescriptorSet(vk::raii::DescriptorPool& descriptorPool,
			vk::raii::DescriptorSetLayout& descriptorSetLayout, vk::raii::Device& device,
			Buffer& buffer, vk::raii::ImageView& textureImageView, vk::raii::Sampler& textureSampler);

		static void updateUniformBuffer(vk::Extent2D& swapChainExtent, Buffer& buffer);
	};

}
