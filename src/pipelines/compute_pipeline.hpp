#pragma once

#include "vr_pipeline.hpp"

namespace vr {
	class ComputePipeline {
	public:
		ComputePipeline(VrDevice& device, const std::string& compFilepath, const PipelineConfigInfo& configInfo);
		~ComputePipeline();

		ComputePipeline(const ComputePipeline&) = delete;
		ComputePipeline& operator=(const ComputePipeline&) = delete;

		void bind(VkCommandBuffer commandBuffer);
	private:
		void createComputePipeline(
			const std::string& compFilepath,
			const PipelineConfigInfo& configInfo);

		static std::vector<char> readFile(const std::string& filename);

		void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

		VrDevice& vrDevice;
		VkPipeline computePipeline;
		VkShaderModule compShaderModule;
	};
}