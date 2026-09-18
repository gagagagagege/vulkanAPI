#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_raii.hpp>

#include "ValidationLayers.h"
#include "vkContext.h"
#include "surface.h"
#include "SwapChain.h"
#include "Pipeline.h"
#include "shader.h"
#include "CommandPool.h"
#include "DescriptorAllocator.h"
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

		// 交换链 + image + view + present 信号量,一个对象管完。
		// 重建边界就是这四样的边界,见 SwapChain::recreate。
		SwapChain swapChain;

		std::vector<vk::DynamicState> dynamicStates = {
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor
		};

		// 描述符的 layout + pool,设备级全局一份。必须早于 frames ——
		// 描述符集要还给这个池(见下面 frames 的声明位置注释)。
		DescriptorAllocator descriptorAllocator;

		// pipelineLayout 归 Pipeline 自己持有
		Pipeline graphicsPipeline;

		// 两个池都是"批量单位":commandPool 管每帧的命令缓冲,
		// transientPool 管上传用的一次性拷贝。必须早于 frames —— 命令缓冲要还回来。
		CommandPool commandPool;
		CommandPool transientPool;

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

		Buffer vertexBuffer;
		Buffer indexBuffer;

		// 声明位置是被约束的:必须晚于 commandPool / transientPool 和 descriptorAllocator。
		// 逆序析构时 frames 才会先于它们释放 —— 命令缓冲的析构要调
		// vkFreeCommandBuffers、描述符集的析构要调 vkFreeDescriptorSets,
		// 两者都需要各自的池还活着。
		Frames frames;
		// Image + view + sampler 归 texture 自己持有。
		// 排在 frames 之后是沿用重构前的顺序,这里不是硬约束 ——
		// 和上面 frames 那条不同,释放描述符集不碰 image view / sampler。
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
