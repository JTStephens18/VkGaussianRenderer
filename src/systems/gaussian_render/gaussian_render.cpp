#include "gaussian_render.hpp"
#include "gaussian_model.hpp"
#include <fstream>

namespace vr {

    struct GaussianPushConstantData {
        glm::mat4 modelMatrix{ 1.f };
        glm::mat4 normalMatrix{ 1.f };
    };

    GaussianRenderSystem::GaussianRenderSystem(const std::string& filepath, VrDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : vrDevice{ device } {
        if (!std::filesystem::exists(filepath)) {
            throw std::runtime_error("File does not exist: " + filepath);
        }
        else {
            createPipelineLayout(globalSetLayout);
            createPipeline(renderPass);
            filename = filepath;
            GaussianRenderSystem::load();
        }
    }

    GaussianRenderSystem::~GaussianRenderSystem() {
        vkDestroyPipelineLayout(vrDevice.device(), pipelineLayout, nullptr);
    };

    void GaussianRenderSystem::load() {
        auto startTime = std::chrono::high_resolution_clock::now();

        std::ifstream plyFile(filename, std::ios::binary);
        loadPlyHeader(plyFile);

        std::cout << "Num vertices " << header.numVertices << std::endl;

        for (auto i = 0; i < header.numVertices; i++) {
            assert(plyFile.is_open());
            assert(!plyFile.eof());
            GaussianModel::Gaussian gaussianData;
            plyFile.read(reinterpret_cast<char*>(&gaussianData), sizeof(GaussianModel::Gaussian));
            gaussianStorage.push_back(gaussianData);
        }

        auto endTime = std::chrono::high_resolution_clock::now();
    }

    void GaussianRenderSystem::loadPlyHeader(std::ifstream& plyFile) {
        if (!plyFile.is_open()) {
            throw std::runtime_error("Could not open file: " + filename);
        }

        std::string line;
        bool headerEnd = false;

        while (std::getline(plyFile, line)) {
            std::istringstream iss(line);
            std::string token;

            iss >> token;

            if (token == "ply") {
                // PLY format indicator
            }
            else if (token == "format") {
                iss >> header.format;
            }
            else if (token == "element") {
                iss >> token;

                if (token == "vertex") {
                    iss >> header.numVertices;
                }
                else if (token == "face") {
                    iss >> header.numFaces;
                }
            }
            else if (token == "property") {
                PlyProperty property;
                iss >> property.type >> property.name;

                if (header.vertexProperties.size() < static_cast<size_t>(header.numVertices)) {
                    header.vertexProperties.push_back(property);
                }
                else {
                    header.faceProperties.push_back(property);
                }
            }
            else if (token == "end_header") {
                headerEnd = true;
                break;
            }
        }

        if (!headerEnd) {
            throw std::runtime_error("Could not find end of header");
        }
    }

    void GaussianRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {

        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(GaussianPushConstantData);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(vrDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create pipeline layout");
        }
    }

    void GaussianRenderSystem::createPipeline(VkRenderPass renderPass) {
        assert(pipelineLayout != nullptr && "Cannot create pipline before pipeline layout");

        auto bindingDescriptions = GaussianModel::Gaussian::getBindingDescriptions();
        auto attributeDescriptions = GaussianModel::Gaussian::getAttributeDescriptions();

        PipelineConfigInfo pipelineConfig{};
        VrPipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        gaussianPipeline = std::make_unique<VrPipeline>(
            vrDevice,
            "../../../shaders/gaussian_shader.vert.spv",
            "../../../shaders/gaussian_shader.frag.spv",
            pipelineConfig,
            bindingDescriptions,
            attributeDescriptions
        );

        gaussianComputePipeline = std::make_unique<ComputePipeline>(
            vrDevice,
            "../../../shaders/preprocess.comp.spv",
            pipelineConfig
        );
    }

    void GaussianRenderSystem::renderGameObjects(FrameInfo& frameInfo, std::vector<VrGameObject>& gameObjects, int& bindIdx) {
        gaussianPipeline->bind(frameInfo.commandBuffer);
        gaussianComputePipeline->bind(frameInfo.computeCommandBuffer);

        vkCmdBindDescriptorSets(
            frameInfo.computeCommandBuffer,
            VK_PIPELINE_BIND_POINT_COMPUTE,
            pipelineLayout,
            0,
            1,
            &frameInfo.globalDescriptorSet,
            0,
            nullptr
        );

        vkCmdDispatch(
            frameInfo.computeCommandBuffer,
            16, 1, 1
        );

        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout,
            0,
            1,
            &frameInfo.globalDescriptorSet,
            0,
            nullptr);

        for (auto& obj : gameObjects) {

            GaussianPushConstantData push{};
            push.modelMatrix = obj.transform.mat4();
            push.normalMatrix = obj.transform.normalMatrix();

            vkCmdPushConstants(
                frameInfo.commandBuffer,
                pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(GaussianPushConstantData),
                &push
            );

            obj.gaussianModel->bind(frameInfo.commandBuffer, bindIdx);
            obj.gaussianModel->draw(frameInfo.commandBuffer);
        }
    }
}