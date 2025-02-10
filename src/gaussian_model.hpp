#pragma once

#include "vr_device.hpp"
#include "buffer.hpp"

#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace vr {
	class GaussianModel {
	public:
		struct Gaussian {
			glm::vec3 position;
			glm::vec3 normal;
			float sh[48];
			float opacity;
			glm::vec3 scale;
			glm::vec4 rotation;

			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

			bool operator==(const Gaussian& other) const {
				return position == other.position && scale == other.scale && normal == other.normal && rotation == other.rotation && opacity == other.opacity;
			}
		};

		struct Builder {
			std::vector<Gaussian> gaussians{};
			std::vector<uint32_t> indices{};
		};

		GaussianModel(VrDevice& device, const GaussianModel::Builder& builder);
		~GaussianModel();

		GaussianModel(const GaussianModel&) = delete;
		GaussianModel& operator=(const GaussianModel&) = delete;

		static std::unique_ptr<GaussianModel> createModelFromGaussians(
			VrDevice& device, const std::vector<Gaussian>& gaussians
		);

		void bind(VkCommandBuffer commandBuffer, int& bindIdx);
		void draw(VkCommandBuffer commandBuffer);

	private:
		void createVertexBuffers(const std::vector<Gaussian>& gaussians);
		void createIndexBuffers(const std::vector<uint32_t>& indices);

		VrDevice& device;
		std::unique_ptr<Buffer> vertexBuffer;
		uint32_t vertexCount;

		bool hasIndexBuffer = false;
		std::unique_ptr<Buffer> indexBuffer;
		uint32_t indexCount;
	};

}