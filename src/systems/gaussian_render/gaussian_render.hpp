#pragma once

#include "camera.hpp"
#include "vr_device.hpp"
#include "frame_info.hpp"
#include "./pipelines/vr_pipeline.hpp"
#include "./pipelines/compute_pipeline.hpp"
#include "gaussian_model.hpp"

#include <memory>
#include <vector>
#include <filesystem>
#include <iostream>
#include <glm/glm.hpp>

namespace vr {

	struct PlyProperty {
		std::string type;
		std::string name;
	};

	struct PlyHeader {
		std::string format;
		int numVertices;
		int numFaces;
		std::vector<PlyProperty> vertexProperties;
		std::vector<PlyProperty> faceProperties;
	};

	class GaussianRenderSystem {
	public:
		GaussianRenderSystem(const std::string& filepath, VrDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~GaussianRenderSystem();

		GaussianRenderSystem(const GaussianRenderSystem&) = delete;
		GaussianRenderSystem& operator=(const GaussianRenderSystem&) = delete;

		void load();

		void renderGameObjects(FrameInfo& frameInfo, std::vector<VrGameObject>& gameObjects, int& bindIdx);

		std::vector<GaussianModel::Gaussian> getGaussians() {
			return gaussianStorage;
		}

		uint64_t getNumVertices() const {
			return header.numVertices;
		}

	private:
		void loadPlyHeader(std::ifstream& ifstream);
		void createPipelineLayout(VkDescriptorSetLayout globalLayout);
		void createPipeline(VkRenderPass renderPass);

		std::string filename;
		PlyHeader header;
		VrDevice& vrDevice;
		std::vector<GaussianModel::Gaussian> gaussianStorage;
		std::unique_ptr<VrPipeline> gaussianPipeline;
		std::unique_ptr<ComputePipeline> gaussianComputePipeline;
		VkPipelineLayout pipelineLayout;
	};
}