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
#include "frameData.h"
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

		// present 用的信号量,每个 swapchain image 一份。
		// 基数 = swapChainImages.size(),不是 MAX_FRAMES_IN_FLIGHT
		std::vector<vk::raii::Semaphore> renderFinishedSemaphores;

		std::vector<vk::DynamicState> dynamicStates = {
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor
		};

		vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
		vk::raii::PipelineLayout pipelineLayout = nullptr;
		vk::raii::Pipeline graphicsPipeline = nullptr;

		vk::raii::CommandPool commandPool = nullptr;
		vk::raii::CommandPool transientPool = nullptr;

		bool framebufferResized = false;

		// 每帧飞行的份数。唯一的配置点。
		const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

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

		vk::raii::DescriptorPool descriptorPool = nullptr;
		// 声明位置是被约束的:必须晚于 commandPool 和 descriptorPool,
		// 逆序析构时 frames 才会先于它们释放(命令缓冲要还给 commandPool)。
		Frames frames;
		// Image + view + sampler 归 texture 自己持有。
		// 保持在 frames 之后声明,沿用重构前的相对顺序(mainTexture 先释放)。
		texture mainTexture;

	private:
		void initWindow();
		void initVulkan();
		void mainLoop();
		void cleanUp();
		void createInstance();

		void drawFrame();
		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
	};

}
