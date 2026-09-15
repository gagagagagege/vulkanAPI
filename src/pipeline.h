#pragma once
#include "shader.h"
#include "buffer.h"

namespace Fish {
	class pipeline
	{
	public:
		static void createGraphicsPipeline(vk::raii::Device& device, std::vector<vk::DynamicState>& dynamicStates,
			vk::Extent2D& swapChainExtent, vk::raii::PipelineLayout& pipelineLayout, vk::Format swapChainImageFormat,
			vk::raii::Pipeline& graphicsPipeline, vk::raii::DescriptorSetLayout& descriptorSetLayout);
	};

}
