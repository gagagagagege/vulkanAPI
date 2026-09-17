#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_raii.hpp>

#include "ValidationLayers.h"
#include "PhysicalDevice.h"
#include "surface.h"
#include "swapChain.h"
#include "pipeline.h"
#include "shader.h"
#include "commandPool.h"
#include "uniformBuffer.h"
#include "texture.h"

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <optional>

#include <vector>
#include <map>
#include <memory>


namespace Fish {

	class TriangleApp {
	public:
		TriangleApp();
		void run();

	private:
		GLFWwindow* window = nullptr;

		//析构时逆序
		vk::raii::Context context;
		vk::raii::Instance instance = nullptr;

		const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
		vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;

		const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME };

		vk::raii::SurfaceKHR surface = nullptr;
		
		//instance 之后、其他 device 资源之前声明
		std::unique_ptr<vkContext> deviceClass;

		vk::raii::SwapchainKHR swapChain = nullptr;
		std::vector<vk::Image> swapChainImages;
		vk::Format swapChainImageFormat;
		vk::Extent2D swapChainExtent;
		std::vector<vk::raii::ImageView> swapChainImageViews;

		std::vector<vk::DynamicState> dynamicStates = {
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor
		};

		vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
		vk::raii::PipelineLayout pipelineLayout = nullptr;
		vk::raii::Pipeline graphicsPipeline = nullptr;

		vk::raii::CommandPool commandPool = nullptr;
		vk::raii::CommandPool transientPool = nullptr;
		std::vector<vk::raii::CommandBuffer> commandBuffers;

		std::vector<vk::raii::Semaphore> presentCompleteSemaphores;
		std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
		std::vector<vk::raii::Fence> inFlightFences;

		bool framebufferResized = false;

		const int MAX_FRAMES_IN_FLIGHT = 2;
		uint32_t frameIndex = 0;
		uint32_t imageIndex = 0;

		static const uint32_t WIDTH = 800;
		static const uint32_t HEIGHT = 600;

		const std::vector<Vertex> vertices = {
			{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {2.0f, 0.0f}},
			{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 2.0f}},
			{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {2.0f, 2.0f}}
		};

		const std::vector<uint16_t> indices = {
			0, 1, 2, 2, 3, 0
		};

		// 缓冲与内存：内存须晚于缓冲销毁，故先声明内存
		Buffer vertexBuffer;
		Buffer indexBuffer;

		std::vector<Buffer> uniformBuffers;

		vk::raii::DescriptorPool descriptorPool = nullptr;
		std::vector<vk::raii::DescriptorSet> descriptorSets;

		// Image + view + sampler 现在都归 texture 自己持有。
		// 放在这里(原 textureImageMemory 的位置)是为了保持析构顺序不变:
		// 仍先于 descriptorSets / descriptorPool 释放。
		texture mainTexture;

		vk::PipelineStageFlags sourceStage;
		vk::PipelineStageFlags destinationStage;
	private:
		void initWindow();
		void initVulkan();
		void mainLoop();
		void cleanUp();
		void createInstance();

		void createSyncObjects();
		void drawFrame();
		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
	};

}
