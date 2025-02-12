#pragma once

#include "vr_window.hpp"
#include "vr_device.hpp"
#include "vr_swap_chain.hpp"

#include <cassert>
#include <memory>
#include <vector>

namespace vr {
	class Renderer {
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		Renderer(VrWindow& window, VrDevice& device);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;

		VkRenderPass getSwapChainRenderPass() const { return vrSwapChain->getRenderPass(); }
		float getAspectRatio() const { return vrSwapChain->extentAspectRatio(); }
		bool isFrameInProgress() const { return isFrameStarted; }

		VkCommandBuffer getCurrentCommandBuffer() const {
			assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
			return commandBuffers[currentFrameIndex];
		}

		VkCommandBuffer getCurrentComputeCommandBuffer() const {
			assert(isFrameStarted && "Cannot get compute command buffer when frame not in progress");
			return computeCommandBuffers[currentFrameIndex];
		}

		int getFrameIndex() const {
			assert(isFrameStarted && "Cannot get frame index when frame not in progress");
			return currentFrameIndex;
		}

		std::vector<VkCommandBuffer> beginFrame();
		void endFrame();
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

	private:

		void createCommandBuffers();
		void createComputeCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();

		VrWindow& vrWindow;
		VrDevice& vrDevice;
		std::unique_ptr<VrSwapChain> vrSwapChain;
		std::vector<VkCommandBuffer> commandBuffers;
		std::vector<VkCommandBuffer> computeCommandBuffers;
		std::vector<VkCommandBuffer> frameCommandBuffers;

		uint32_t currentImageIndex;
		int currentFrameIndex{ 0 };
		bool isFrameStarted{ false };
	};
}