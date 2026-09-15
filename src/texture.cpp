#define STB_IMAGE_IMPLEMENTATION
#include "texture.h"

#include "buffer.h"
#include "commandPool.h"
#include "PhysicalDevice.h"

#include <stdexcept>
#include <cstring>
#include <tuple>

namespace Fish {
	void texture::createTextureImage(vkContext* context, vk::raii::Image& textureImage,
		vk::raii::DeviceMemory& textureImageMemory, vk::raii::CommandPool& commandPool)
	{
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load("textures/texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		vk::DeviceSize imageSize = static_cast<vk::DeviceSize>(texWidth) * texHeight * 4;

		if (!pixels)
		{
			throw std::runtime_error("failed to load texture image!");
		}

		Buffer stagingBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, context);

		memcpy(stagingBuffer.map(0, imageSize), pixels, imageSize);
		stagingBuffer.unmap();
		stbi_image_free(pixels);

		std::tie(textureImage, textureImageMemory) = texture::createImage(context->device, context->physicalDevice,
			vk::Format::eR8G8B8A8Srgb, texWidth, texHeight,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
			vk::MemoryPropertyFlagBits::eDeviceLocal);

		vk::raii::CommandBuffer commandBuffer = commandPool::beginSingleTimeCommands(commandPool, context->device);
		transitionImageLayout(commandBuffer, textureImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
		copyBufferToImage(commandBuffer, stagingBuffer.getHandle(), textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
		transitionImageLayout(commandBuffer, textureImage, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
		commandPool::endSingleTimeCommands(commandBuffer, context->graphicsQueue);
	}

	std::pair<vk::raii::Image, vk::raii::DeviceMemory> texture::createImage(vk::raii::Device& device, vk::raii::PhysicalDevice& physicalDevice,
		const vk::Format& format, uint32_t width, uint32_t height, const vk::ImageTiling& tiling,
		const vk::Flags<vk::ImageUsageFlagBits>& usage, const vk::MemoryPropertyFlags& properties)
	{
		vk::ImageCreateInfo imageInfo{
			.imageType = vk::ImageType::e2D,
			.format = format,
			.extent = { width, height, 1 },
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = vk::SampleCountFlagBits::e1,
			.tiling = tiling,
			.usage = usage,
			.sharingMode = vk::SharingMode::eExclusive };
		vk::raii::Image image(device, imageInfo);

		vk::MemoryRequirements memRequirements = image.getMemoryRequirements();
		vk::MemoryAllocateInfo allocInfo{ .allocationSize = memRequirements.size,
										 .memoryTypeIndex = Buffer::findMemoryType(memRequirements.memoryTypeBits, properties, physicalDevice) };
		vk::raii::DeviceMemory imageMemory(device, allocInfo);
		image.bindMemory(*imageMemory, 0);

		return { std::move(image), std::move(imageMemory) };
	}

	void texture::transitionImageLayout(vk::raii::CommandBuffer& commandBuffer, const vk::raii::Image& image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
	{
		vk::PipelineStageFlags sourceStage{};
		vk::PipelineStageFlags destinationStage{};

		vk::ImageMemoryBarrier barrier{ .oldLayout = oldLayout,
							   .newLayout = newLayout,
							   .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
							   .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
							   .image = image,
							   .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor, .levelCount = 1, .layerCount = 1} };
		if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
		{
			barrier.srcAccessMask = {};
			barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

			sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStage = vk::PipelineStageFlagBits::eTransfer;
		}
		else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
		{
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

			sourceStage = vk::PipelineStageFlagBits::eTransfer;
			destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
		}
		else
		{
			throw std::invalid_argument("unsupported layout transition!");
		}
		commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, {}, nullptr, barrier);
	}

	void texture::copyBufferToImage(vk::raii::CommandBuffer& commandBuffer, const vk::raii::Buffer& buffer, vk::raii::Image& image, uint32_t width, uint32_t height)
	{
		vk::BufferImageCopy region{ .bufferOffset = 0,
						   .bufferRowLength = 0,
						   .bufferImageHeight = 0,
						   .imageSubresource = {.aspectMask = vk::ImageAspectFlagBits::eColor, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1},
						   .imageOffset = {0, 0, 0},
						   .imageExtent = {width, height, 1} };
		commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, region);
	}
	void texture::createTextureImageView(vk::raii::Image& textureImage, vk::raii::ImageView& textureImageView, vk::raii::Device& device)
	{
		textureImageView = imageView::createImageView(*textureImage, vk::Format::eR8G8B8A8Srgb, device);
	}
	void texture::createTextureSampler(vk::raii::PhysicalDevice& physicalDevice, vk::raii::Sampler& textureSampler, vk::raii::Device& device)
	{
		vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();
		vk::SamplerCreateInfo        samplerInfo{ .magFilter = vk::Filter::eLinear,
												 .minFilter = vk::Filter::eLinear,
												 .mipmapMode = vk::SamplerMipmapMode::eLinear,
												 .addressModeU = vk::SamplerAddressMode::eRepeat,
												 .addressModeV = vk::SamplerAddressMode::eRepeat,
												 .addressModeW = vk::SamplerAddressMode::eRepeat,
												 .anisotropyEnable = vk::True,
												 .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
												 .compareEnable = vk::False,
												 .compareOp = vk::CompareOp::eAlways };
		samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
		samplerInfo.unnormalizedCoordinates = vk::False;
		samplerInfo.compareEnable = vk::False;
		samplerInfo.compareOp = vk::CompareOp::eAlways;
		samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		textureSampler = vk::raii::Sampler(device, samplerInfo);
	}
}
