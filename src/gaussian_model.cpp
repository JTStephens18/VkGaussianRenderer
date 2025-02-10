#include "gaussian_model.hpp"

#include "utils.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <cassert>
#include <cstring>
#include <unordered_map>
#include <iostream>

namespace vr {
	GaussianModel::GaussianModel(VrDevice& device, const GaussianModel::Builder& builder) : device{ device } {
		createVertexBuffers(builder.gaussians);
		createIndexBuffers(builder.indices);
	}

	GaussianModel::~GaussianModel() {}

	std::unique_ptr<GaussianModel> GaussianModel::createModelFromGaussians(
		VrDevice& device, const std::vector<Gaussian>& gaussians
	) {
		Builder builder{};
		builder.gaussians = gaussians;
		return std::make_unique<GaussianModel>(device, builder);
	}

	void GaussianModel::createVertexBuffers(const std::vector<Gaussian>& vertices) {
		vertexCount = static_cast<uint32_t>(vertices.size());
		assert(vertexCount >= 3 && "Vertex count must be at least 3!");
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
		
		uint32_t vertexSize = sizeof(vertices[0]);
		Buffer stagingBuffer{
			device,
			vertexSize,
			vertexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		};

		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)vertices.data());

		vertexBuffer = std::make_unique<Buffer>(
			device,
			vertexSize,
			vertexCount,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		device.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
	}

	void GaussianModel::createIndexBuffers(const std::vector<uint32_t>& indices) {
		indexCount = static_cast<uint32_t>(indices.size());
		hasIndexBuffer = indexCount > 0;

		if (!hasIndexBuffer) {
			return;
		}

		VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
		uint32_t indexSize = sizeof(indices[0]);

		Buffer stagingBuffer{
			device,
			indexSize,
			indexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};

		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)indices.data());

		indexBuffer = std::make_unique<Buffer>(
			device,
			indexSize,
			indexCount,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		device.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);
	}

	void GaussianModel::draw(VkCommandBuffer commandBuffer) {
		if (hasIndexBuffer) {
			vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
		}
		else {
			vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
		}
	}

	void GaussianModel::bind(VkCommandBuffer commandBuffer, int& bindIdx) {
		VkBuffer buffers[] = { vertexBuffer->getBuffer() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 1, 1, buffers, offsets);
		if (hasIndexBuffer) {
			vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
		}
	}

	std::vector<VkVertexInputBindingDescription> GaussianModel::Gaussian::getBindingDescriptions() {
		std::vector <VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 1;
		bindingDescriptions[0].stride = sizeof(Gaussian);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescriptions;
	}

	std::vector<VkVertexInputAttributeDescription> GaussianModel::Gaussian::getAttributeDescriptions() {
		std::vector <VkVertexInputAttributeDescription> attributeDescriptions{};

		attributeDescriptions.push_back({ 4, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Gaussian, position) });

		attributeDescriptions.push_back({ 5, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Gaussian, normal) });

		// SH Coefficients
		for (uint32_t i = 0; i < 12; ++i) {
			VkVertexInputAttributeDescription shAttr{};
			shAttr.location = 6 + i;
			shAttr.binding = 1;
			shAttr.format = VK_FORMAT_R32G32B32A32_SFLOAT;
			shAttr.offset = offsetof(Gaussian, sh) + i * sizeof(glm::vec4);
			attributeDescriptions.push_back(shAttr);
		};

		attributeDescriptions.push_back({ 18, 1, VK_FORMAT_R32_SFLOAT, offsetof(Gaussian, opacity) });

		attributeDescriptions.push_back({ 19, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Gaussian, scale) });

		attributeDescriptions.push_back({ 20, 1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Gaussian, rotation) });

		return attributeDescriptions;
	};
}