#pragma once

#include "../vr_device.hpp"
#include <string>
#include <vector>

namespace vr {

	struct PipelineConfigInfo {

		VkPipelineViewportStateCreateInfo viewportInfo;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo multisampleInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo colorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
		std::vector<VkDynamicState> dynamicStateEnables;
		VkPipelineDynamicStateCreateInfo dynamicStateInfo;
		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
		uint32_t subpass = 0;
	};

	class VrPipeline {
		public:
			VrPipeline(
				VrDevice& device, 
				const std::string& vertFilepath, 
				const std::string& fragFilepath, 
				const PipelineConfigInfo& configInfo);
			~VrPipeline();

			VrPipeline(const VrPipeline&) = delete;
			VrPipeline& operator=(const VrPipeline&) = delete;

			void bind(VkCommandBuffer commandBuffer);

			static void defaultPipelineConfigInfo(PipelineConfigInfo &configInfo);

		private:
			static std::vector<char> readFile(const std::string& filename);

			void createGraphicsPipeline(
				const std::string& vertFilepath, 
				const std::string& fragFilepath, 
				const PipelineConfigInfo& configInfo);

			void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

			VrDevice& vrDevice;
			VkPipeline graphicsPipeline;
			VkShaderModule vertShaderModule;
			VkShaderModule fragShaderModule;


	};
}