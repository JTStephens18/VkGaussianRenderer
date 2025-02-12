#include "renderer.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>
#include <cassert>
#include <stdexcept>

namespace vr {
	Renderer::Renderer(VrWindow& window, VrDevice& device) : vrWindow{ window }, vrDevice{ device } {
		recreateSwapChain();
		createCommandBuffers();
		createComputeCommandBuffers();
	}

	Renderer::~Renderer() { 
		freeCommandBuffers(); 
	}

	void Renderer::recreateSwapChain() {
		auto extent = vrWindow.getExtent();
		while (extent.width == 0 || extent.height == 0) {
			extent = vrWindow.getExtent();
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(vrDevice.device());

		if (vrSwapChain == nullptr) {
			vrSwapChain = std::make_unique<VrSwapChain>(vrDevice, extent);
		}
		else {
			std::shared_ptr<VrSwapChain> oldSwapChain = std::move(vrSwapChain);
			vrSwapChain = std::make_unique<VrSwapChain>(vrDevice, extent, oldSwapChain);

			if (!oldSwapChain->compareSwapFormats(*vrSwapChain.get())) {
				throw std::runtime_error("Swap chain image (or depth) format has changed!");
			}
		}
	}

	void Renderer::createCommandBuffers() {
		commandBuffers.resize(vrSwapChain->imageCount());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = vrDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(vrDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate command buffers!");
		}
	};

	void Renderer::createComputeCommandBuffers() {
		computeCommandBuffers.resize(vrSwapChain->imageCount());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = vrDevice.getComputeCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(computeCommandBuffers.size());

		if (vkAllocateCommandBuffers(vrDevice.device(), &allocInfo, computeCommandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate command buffers!");
		}
	};

	void Renderer::freeCommandBuffers() {
		vkFreeCommandBuffers(
			vrDevice.device(),
			vrDevice.getCommandPool(),
			static_cast<float>(commandBuffers.size()),
			commandBuffers.data());
		commandBuffers.clear();

		vkFreeCommandBuffers(
			vrDevice.device(),
			vrDevice.getComputeCommandPool(),
			static_cast<float>(computeCommandBuffers.size()),
			computeCommandBuffers.data());
		computeCommandBuffers.clear();
	}

	std::vector<VkCommandBuffer> Renderer::beginFrame() {
		assert(!isFrameStarted && "Cannot call beginFrame while already in progress");

		frameCommandBuffers.clear();

		auto result = vrSwapChain->acquireNextImage(&currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			frameCommandBuffers[0] = nullptr;
			return frameCommandBuffers;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("Failed to acquire swap chain image!");
		}

		isFrameStarted = true;
		
		auto commandBuffer = getCurrentCommandBuffer();
		auto computeCommandBuffer = getCurrentComputeCommandBuffer();

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		if (vkBeginCommandBuffer(computeCommandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("Failed to begin recording compute command buffer");
		}

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("Failed to begin recording command buffer");
		}

		frameCommandBuffers.push_back(commandBuffer);

		frameCommandBuffers.push_back(computeCommandBuffer);

		return frameCommandBuffers;
	}

	void Renderer::endFrame() {
		assert(isFrameStarted && "Can't call endFrame while frame is not in progress");

		auto computeCommandBuffer = getCurrentComputeCommandBuffer();
		if (vkEndCommandBuffer(computeCommandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("Failed to record compute command buffer");
		}

		auto commandBuffer = getCurrentCommandBuffer();
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("Failed to record command buffer");
		}

		auto result = vrSwapChain->submitCommandBuffers(&commandBuffer, &computeCommandBuffer, &currentImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || vrWindow.wasWindowResized()) {
			vrWindow.resetWindowResizedFlag();
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to present swap chain image!");
		}

		isFrameStarted = false;
		currentFrameIndex = (currentFrameIndex + 1) % VrSwapChain::MAX_FRAMES_IN_FLIGHT;
	}

	void Renderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
		assert(
			commandBuffer == getCurrentCommandBuffer() &&
			"Can't begin render pass on command buffer from a different frame");

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = vrSwapChain->getRenderPass();
		renderPassInfo.framebuffer = vrSwapChain->getFrameBuffer(currentImageIndex);

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = vrSwapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(vrSwapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(vrSwapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, vrSwapChain->getSwapChainExtent() };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void Renderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		assert(isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");
		assert(
			commandBuffer == getCurrentCommandBuffer() &&
			"Can't end render pass on command buffer from a different frame");
		vkCmdEndRenderPass(commandBuffer);
	}

}