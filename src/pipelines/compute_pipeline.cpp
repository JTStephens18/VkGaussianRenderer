#include "compute_pipeline.hpp"

#include <cassert>
#include <stdexcept>
#include <filesystem>
#include <iostream>
#include <fstream>

namespace vr {
	ComputePipeline::ComputePipeline(VrDevice& device, const std::string& compFilepath, const PipelineConfigInfo& configInfo) : vrDevice{ device } {
		createComputePipeline(compFilepath, configInfo);
	};

	ComputePipeline::~ComputePipeline() {
		vkDestroyShaderModule(vrDevice.device(), compShaderModule, nullptr);
		vkDestroyPipeline(vrDevice.device(), computePipeline, nullptr);
	};

	std::vector<char> ComputePipeline::readFile(const std::string& filename) {
		std::ifstream file{ filename, std::ios::ate | std::ios::binary };

		if (!file.is_open()) {
			throw std::runtime_error("Failed to open file: " + filename);
		}

		size_t fileSize = static_cast<size_t>(file.tellg());
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();
		return buffer;
	}

	void ComputePipeline::createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule) {
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		if (vkCreateShaderModule(vrDevice.device(), &createInfo, nullptr, shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create shader module");
		}
	}


	void ComputePipeline::createComputePipeline(const std::string& compFilepath, const PipelineConfigInfo& configInfo) {
		auto compCode = ComputePipeline::readFile(compFilepath);

		ComputePipeline::createShaderModule(compCode, &compShaderModule);

		VkPipelineShaderStageCreateInfo compShaderStage{};
		compShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		compShaderStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		compShaderStage.module = compShaderModule;
		compShaderStage.pName = "main";

		VkComputePipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineInfo.stage = compShaderStage;
		pipelineInfo.layout = configInfo.pipelineLayout;

		if (vkCreateComputePipelines(
			vrDevice.device(),
			VK_NULL_HANDLE,
			1,
			&pipelineInfo,
			nullptr,
			&computePipeline) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create compute pipeline!");
		}
	}

	void ComputePipeline::bind(VkCommandBuffer commandBuffer) {
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline);
	}
}