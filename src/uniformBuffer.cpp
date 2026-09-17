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

	vk::raii::DescriptorPool uniformBuffer::createDescriptorPool(uint32_t frameCount, vk::raii::Device& device)
	{
		std::array<vk::DescriptorPoolSize, 2> poolSize{ {{.type = vk::DescriptorType::eUniformBuffer, .descriptorCount = frameCount},
												{.type = vk::DescriptorType::eCombinedImageSampler, .descriptorCount = frameCount}} };
		vk::DescriptorPoolCreateInfo          poolInfo{ .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
													   .maxSets = frameCount,
													   .poolSizeCount = static_cast<uint32_t>(poolSize.size()),
													   .pPoolSizes = poolSize.data() };
		vk::raii::DescriptorPool descriptorPool(device, poolInfo);
		return descriptorPool;
	}

	vk::raii::DescriptorSet uniformBuffer::createDescriptorSet(vk::raii::DescriptorPool& descriptorPool,
		vk::raii::DescriptorSetLayout& descriptorSetLayout, vk::raii::Device& device,
		Buffer& buffer, vk::raii::ImageView& textureImageView, vk::raii::Sampler& textureSampler)
	{
		std::vector<vk::DescriptorSetLayout> layouts{ *descriptorSetLayout };
		vk::DescriptorSetAllocateInfo        allocInfo{ .descriptorPool = descriptorPool,
													   .descriptorSetCount = 1,
													   .pSetLayouts = layouts.data() };

		// 分配 1 个再 move 出来。vk::raii::DescriptorSet 的析构是空操作,
		// 真正释放发生在 pool 销毁时,所以单个句柄搬来搬去没有代价。
		vk::raii::DescriptorSet descriptorSet = std::move(device.allocateDescriptorSets(allocInfo).front());

		vk::DescriptorBufferInfo bufferInfo{ .buffer = buffer.getHandle(), .offset = 0, .range = sizeof(UniformBufferObject) };
		vk::DescriptorImageInfo  imageInfo{ .sampler = textureSampler, .imageView = textureImageView, .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal };
		std::array<vk::WriteDescriptorSet, 2> descriptorWrites{ {{.dstSet = descriptorSet,
															 .dstBinding = 0,
															 .dstArrayElement = 0,
															 .descriptorCount = 1,
															 .descriptorType = vk::DescriptorType::eUniformBuffer,
															 .pBufferInfo = &bufferInfo},
															{.dstSet = descriptorSet,
															 .dstBinding = 1,
															 .dstArrayElement = 0,
															 .descriptorCount = 1,
															 .descriptorType = vk::DescriptorType::eCombinedImageSampler,
															 .pImageInfo = &imageInfo}} };
		device.updateDescriptorSets(descriptorWrites, {});

		return descriptorSet;
	}

	void uniformBuffer::updateUniformBuffer(vk::Extent2D& swapChainExtent, Buffer& buffer)
	{
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		UniformBufferObject ubo{};
		ubo.model = rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.view =  lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.proj =  glm::perspective(glm::radians(45.0f), static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height), 0.1f, 10.0f);
		ubo.proj[1][1] *= -1;
		memcpy(buffer.map(0, sizeof(ubo)), &ubo, sizeof(ubo));
	}
}
