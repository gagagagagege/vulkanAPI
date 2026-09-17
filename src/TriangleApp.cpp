#include "TriangleApp.h"

namespace Fish {
	TriangleApp::TriangleApp() {}

	void TriangleApp::run()
	{
		initWindow();
		initVulkan();
		mainLoop();
		cleanUp();
	}

	void TriangleApp::initWindow()
	{
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}
	void TriangleApp::initVulkan()
	{
		createInstance();
		setupDebugMessenger(instance, debugMessenger);
		surface::createWin32Surface(instance, surface, window);

		deviceClass = std::make_unique<vkContext>(instance, surface, deviceExtensions, validationLayers);

		swapChain::createSwapChain(deviceClass->physicalDevice, surface, window, deviceClass->device, swapChain, swapChainImages, swapChainImageFormat, swapChainExtent);
		swapChain::createImageViews(swapChainImages, swapChainImageViews, swapChainImageFormat, deviceClass->device);
		uniformBuffer::createDescriptorSetLayout(descriptorSetLayout, deviceClass->device);
		pipeline::createGraphicsPipeline(deviceClass->device, dynamicStates, swapChainExtent, pipelineLayout, swapChainImageFormat, graphicsPipeline, descriptorSetLayout);
		commandPool::createCommandPool(deviceClass->physicalDevice, surface, deviceClass->device, commandPool);
		commandPool::createCommandPool(deviceClass->physicalDevice, surface, deviceClass->device, transientPool);
		mainTexture = texture::loadFromFile(deviceClass.get(), transientPool, "textures/texture.jpg");
		vertexBuffer = Buffer::createVertexBuffer(vertices, deviceClass.get(), transientPool);
		indexBuffer = Buffer::createIndexBuffer(indices, deviceClass.get(), transientPool);
		uniformBuffer::createUniformBuffers(MAX_FRAMES_IN_FLIGHT, uniformBuffers, deviceClass.get());
		descriptorPool = uniformBuffer::createDescriptorPool(MAX_FRAMES_IN_FLIGHT, deviceClass->device);
		uniformBuffer::createDescriptorSets(MAX_FRAMES_IN_FLIGHT, uniformBuffers, descriptorSetLayout, descriptorPool, deviceClass->device, descriptorSets, mainTexture.getView(), mainTexture.getSampler());
		commandPool::createCommandBuffers(commandPool, deviceClass->device, commandBuffers, MAX_FRAMES_IN_FLIGHT);
		createSyncObjects();
	}

	void TriangleApp::mainLoop()
	{
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
			drawFrame();
		}
	}

	void TriangleApp::cleanUp()
	{
		swapChain::cleanupSwapChain(swapChainImageViews, swapChain);

		// 其余 Vulkan 资源由 vk::raii 句柄按成员声明逆序自动释放

		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void TriangleApp::createInstance()
	{
		if (enableValidationLayers && !checkValidationLayerSupport(context, validationLayers)) {
			throw std::runtime_error("validation layers requested, but not available!");
		}

		vk::ApplicationInfo appInfo{};
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_3;

		auto requiredExtensions = getRequiredExtensions();

		vk::InstanceCreateInfo createInfo{};
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
		createInfo.ppEnabledExtensionNames = requiredExtensions.data();
		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}

		// 枚举所有可用的Vulkan扩展
		auto availableExtensions = context.enumerateInstanceExtensionProperties();

		std::cout << "available extensions:\n";
		for (const auto& extension : availableExtensions) {
			std::cout << '\t' << extension.extensionName << '\n';
		}

		// Challenge: 检查所有GLFW要求的扩展是否都在支持的扩展列表中
		std::cout << "\nChecking required GLFW extensions...\n";
		for (const char* requiredExtension : requiredExtensions) {
			bool found = false;
			for (const auto& extension : availableExtensions) {
				if (strcmp(requiredExtension, extension.extensionName) == 0) {
					found = true;
					std::cout << '\t' << requiredExtension << " - supported\n";
					break;
				}
			}
			if (!found) {
				throw std::runtime_error(
					std::string("Required GLFW extension not supported: ") + requiredExtension
				);
			}
		}
		std::cout << "All required GLFW extensions are supported!\n";
		// Challenge-----------------------------------------------------

		instance = vk::raii::Instance(context, createInfo);
	}

	void TriangleApp::createSyncObjects()
	{
		presentCompleteSemaphores.reserve(MAX_FRAMES_IN_FLIGHT);
		renderFinishedSemaphores.reserve(MAX_FRAMES_IN_FLIGHT);
		inFlightFences.reserve(MAX_FRAMES_IN_FLIGHT);

		vk::SemaphoreCreateInfo semaphoreInfo{};

		vk::FenceCreateInfo fenceInfo{};
		fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			presentCompleteSemaphores.emplace_back(deviceClass->device, semaphoreInfo);
			renderFinishedSemaphores.emplace_back(deviceClass->device, semaphoreInfo);
			inFlightFences.emplace_back(deviceClass->device, fenceInfo);
		}
	}

	void TriangleApp::drawFrame()
	{
		auto fenceResult = deviceClass->device.waitForFences(*inFlightFences[frameIndex], vk::True, UINT64_MAX);
		if (fenceResult != vk::Result::eSuccess)
		{
			throw std::runtime_error("failed to wait for fence!");
		}

		auto [result, imageIndex] = swapChain.acquireNextImage(UINT64_MAX, *presentCompleteSemaphores[frameIndex], nullptr);

		if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eErrorSurfaceLostKHR) {
			swapChain::recreateSwapChain(deviceClass->physicalDevice, surface,
				window, deviceClass->device, swapChain,
				swapChainImages, swapChainImageFormat,
				swapChainExtent, swapChainImageViews);
			return;
		}
		else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		deviceClass->device.resetFences(*inFlightFences[frameIndex]);

		commandBuffers[frameIndex].reset();
		commandPool::recordCommandBuffer(commandBuffers[frameIndex], imageIndex,frameIndex, swapChainImages, swapChainImageViews, swapChainExtent,
			graphicsPipeline, vertexBuffer.getHandle(), vertices, indexBuffer.getHandle(), indices,pipelineLayout,descriptorSets);

		uniformBuffer::updateUniformBuffer(frameIndex, swapChainExtent, uniformBuffers);

		vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		const vk::SubmitInfo   submitInfo{ .waitSemaphoreCount = 1,
										  .pWaitSemaphores = &*presentCompleteSemaphores[frameIndex],
										  .pWaitDstStageMask = &waitDestinationStageMask,
										  .commandBufferCount = 1,
										  .pCommandBuffers = &*commandBuffers[frameIndex],
										  .signalSemaphoreCount = 1,
										  .pSignalSemaphores = &*renderFinishedSemaphores[frameIndex] };

		deviceClass->graphicsQueue.submit(submitInfo, *inFlightFences[frameIndex]);

		const vk::PresentInfoKHR presentInfoKHR{ .waitSemaphoreCount = 1,
												.pWaitSemaphores = &*renderFinishedSemaphores[frameIndex],
												.swapchainCount = 1,
												.pSwapchains = &*swapChain,
												.pImageIndices = &imageIndex };

		result = deviceClass->presentQueue.presentKHR(presentInfoKHR);

		if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || result == vk::Result::eErrorSurfaceLostKHR || framebufferResized) {
			framebufferResized = false;
			swapChain::recreateSwapChain(deviceClass->physicalDevice, surface,
				window, deviceClass->device, swapChain,
				swapChainImages, swapChainImageFormat,
				swapChainExtent, swapChainImageViews);
		}
		else if (result != vk::Result::eSuccess) {
			throw std::runtime_error("failed to present swap chain image!");
		}

		frameIndex = (frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void TriangleApp::framebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		auto app = reinterpret_cast<TriangleApp*>(glfwGetWindowUserPointer(window));
		app->framebufferResized = true;
	}

}
