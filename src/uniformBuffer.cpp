#include "uniformBuffer.h"

namespace Fish {
	void uniformBuffer::createDescriptorSetLayout(vk::raii::DescriptorSetLayout& descriptorSetLayout, vk::raii::Device& device)
	{
		std::array<vk::DescriptorSetLayoutBinding, 2> bindings{
	        {{.binding = 0, .descriptorType = vk::DescriptorType::eUniformBuffer, .descriptorCount = 1, .stageFlags = vk::ShaderStageFlagBits::eVertex},
	         {.binding = 1, .descriptorType = vk::DescriptorType::eCombinedImageSampler, .descriptorCount = 1, .stageFlags = vk::ShaderStageFlagBits::eFragment}} };

		vk::DescriptorSetLayoutCreateInfo layoutInfo{ .bindingCount = static_cast<uint32_t>(bindings.size()), .pBindings = bindings.data() };
		descriptorSetLayout = vk::raii::DescriptorSetLayout(device, layoutInfo);
	}

	void uniformBuffer::createUniformBuffers(const int& MAX_FRAMES_IN_FLIGHT, std::vector<Buffer>& uniformBuffers, vkContext* context)
	{
		vk::DeviceSize bufferSize = sizeof(UniformBufferObject);

		uniformBuffers.reserve(MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			uniformBuffers.emplace_back(
				bufferSize, vk::BufferUsageFlagBits::eUniformBuffer,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
				context);
		}
	}

	void uniformBuffer::updateUniformBuffer(uint32_t frameIndex, vk::Extent2D& swapChainExtent, std::vector<Buffer>& uniformBuffers)
	{
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		UniformBufferObject ubo{};
		ubo.model = rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.view =  lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.proj =  glm::perspective(glm::radians(45.0f), static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height), 0.1f, 10.0f);
		ubo.proj[1][1] *= -1;
		memcpy(uniformBuffers[frameIndex].map(0, sizeof(ubo)), &ubo, sizeof(ubo));
	}

	vk::raii::DescriptorPool uniformBuffer::createDescriptorPool(uint32_t MAX_FRAMES_IN_FLIGHT, vk::raii::Device& device)
	{
		std::array<vk::DescriptorPoolSize, 2> poolSize{ {{.type = vk::DescriptorType::eUniformBuffer, .descriptorCount = MAX_FRAMES_IN_FLIGHT},
												{.type = vk::DescriptorType::eCombinedImageSampler, .descriptorCount = MAX_FRAMES_IN_FLIGHT}} };
		vk::DescriptorPoolCreateInfo          poolInfo{ .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
													   .maxSets = MAX_FRAMES_IN_FLIGHT,
													   .poolSizeCount = static_cast<uint32_t>(poolSize.size()),
													   .pPoolSizes = poolSize.data() };
		vk::raii::DescriptorPool descriptorPool(device, poolInfo);
		return descriptorPool;
	}

	void uniformBuffer::createDescriptorSets(uint32_t MAX_FRAMES_IN_FLIGHT, std::vector<Buffer>& uniformBuffers,
		vk::raii::DescriptorSetLayout& descriptorSetLayout, vk::raii::DescriptorPool& descriptorPool, vk::raii::Device& device,
		std::vector<vk::raii::DescriptorSet>& descriptorSets, vk::raii::ImageView& textureImageView,
	    vk::raii::Sampler& textureSampler)
	{
		std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, *descriptorSetLayout);
		vk::DescriptorSetAllocateInfo        allocInfo{ .descriptorPool = descriptorPool,
													   .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
													   .pSetLayouts = layouts.data() };
		descriptorSets = device.allocateDescriptorSets(allocInfo);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vk::DescriptorBufferInfo bufferInfo{ .buffer = uniformBuffers[i].getHandle(), .offset = 0, .range = sizeof(UniformBufferObject) };
			vk::DescriptorImageInfo  imageInfo{ .sampler = textureSampler, .imageView = textureImageView, .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal };
			std::array<vk::WriteDescriptorSet, 2> descriptorWrites{ {{.dstSet = descriptorSets[i],
															 .dstBinding = 0,
															 .dstArrayElement = 0,
															 .descriptorCount = 1,
															 .descriptorType = vk::DescriptorType::eUniformBuffer,
															 .pBufferInfo = &bufferInfo},
															{.dstSet = descriptorSets[i],
															 .dstBinding = 1,
															 .dstArrayElement = 0,
															 .descriptorCount = 1,
															 .descriptorType = vk::DescriptorType::eCombinedImageSampler,
															 .pImageInfo = &imageInfo}} };
			device.updateDescriptorSets(descriptorWrites, {});
		}
	}
}