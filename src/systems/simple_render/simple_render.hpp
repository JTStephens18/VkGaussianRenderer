#pragma once

#include "camera.hpp"
#include "vr_device.hpp"
#include "frame_info.hpp"
#include "./pipelines/vr_pipeline.hpp"

#include <memory>
#include <vector>

namespace vr {
	class SimpleRenderSystem {
	public:
		SimpleRenderSystem(VrDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~SimpleRenderSystem();

		SimpleRenderSystem(const SimpleRenderSystem&) = delete;
		SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

		void renderGameObjects(FrameInfo& frameInfo, std::vector<VrGameObject>& gameObjects, int& bindIdx);

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalLayout);
		void createPipeline(VkRenderPass renderPass);

		VrDevice& vrDevice;
		std::unique_ptr<VrPipeline> vrPipeline;
		VkPipelineLayout pipelineLayout;
	};
}