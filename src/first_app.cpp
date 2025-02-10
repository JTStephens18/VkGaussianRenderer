#include "first_app.hpp"

#include "movement_controller.hpp"
#include "buffer.hpp"
#include "camera.hpp"
#include "systems/simple_render/simple_render.hpp"
#include "systems/gaussian_render/gaussian_render.hpp"	

#include <stdexcept>
#include <array>
#include <cassert>
#include <chrono>

namespace vr {

	struct GlobalUbo {
		glm::mat4 projectionView{ 1.f };
		glm::vec3 lightDirection = glm::normalize(glm::vec3{ 1.f, -3.f, -1.f });
	};

	FirstApp::FirstApp() {

		globalPool = VrDescriptorPool::Builder(vrDevice)
			.setMaxSets(VrSwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VrSwapChain::MAX_FRAMES_IN_FLIGHT)
			.build();
	}

	FirstApp::~FirstApp() {
		vkDestroyPipelineLayout(vrDevice.device(), nullptr, nullptr);
	}

	void FirstApp::run() {

		std::vector<std::unique_ptr<Buffer>> uboBuffers(VrSwapChain::MAX_FRAMES_IN_FLIGHT);

		for (int i = 0; i < uboBuffers.size(); i++) {
			uboBuffers[i] = std::make_unique<Buffer>(
				vrDevice,
				sizeof(GlobalUbo),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			uboBuffers[i]->map();
		}

		auto globalSetLayout = VrDescriptorSetLayout::Builder(vrDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.build();

		std::vector<VkDescriptorSet>
			globalDescriptorSets(VrSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < globalDescriptorSets.size(); i++) {
			auto bufferInfo = uboBuffers[i]->descriptorInfo();
			VrDescriptorWriter(*globalSetLayout, *globalPool)
				.writeBuffer(0, &bufferInfo)
				.build(globalDescriptorSets[i]);
		}

		SimpleRenderSystem simpleRenderSystem{
			vrDevice,
			renderer.getSwapChainRenderPass(),
			globalSetLayout->getDescriptorSetLayout()
		};

		GaussianRenderSystem gaussianRenderSystem{
		"C:/Users/JTSte/Downloads//02880940/02880940-c25fd49b75c12ef86bbb74f0f607cdd.ply",
		vrDevice, 
		renderer.getSwapChainRenderPass(),
			globalSetLayout->getDescriptorSetLayout()
		};
		

		gaussians = gaussianRenderSystem.getGaussians();

		loadGameObjects();

		int bindIdx = 0;
		int gaussianBindIdx = 1;

		Camera camera{};

		auto viewerObject = VrGameObject::createGameObject();
		KeyboardMovementController cameraController{};
		
		auto currentTime = std::chrono::high_resolution_clock::now();

		while (!vrWindow.shouldClose()) {
			glfwPollEvents();

			auto newTime = std::chrono::high_resolution_clock::now();

			float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();

			cameraController.moveInPlaneXZ(vrWindow.getGLFWwindow(), frameTime, viewerObject);
			camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

			float aspect = renderer.getAspectRatio();
			camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);
			
			if (auto commandBuffer = renderer.beginFrame()) {

				imGuiManager.Vr_ImGui_CreateFontsTexture();

				int frameIndex = renderer.getFrameIndex();

				FrameInfo frameInfo{
					frameIndex,
					frameTime,
					commandBuffer,
					camera,
					globalDescriptorSets[frameIndex]
				};

				GlobalUbo ubo{};
				ubo.projectionView = camera.getProjection() * camera.getView();
				uboBuffers[frameIndex]->writeToBuffer(&ubo);
				uboBuffers[frameIndex]->flush();

				renderer.beginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjects(frameInfo, gameObjects, bindIdx);
				//std::cout << "Render game objects" << std::endl;
				gaussianRenderSystem.renderGameObjects(frameInfo, gaussianObjects, gaussianBindIdx);
				//std::cout << "Render gaussian objects" << std::endl;
				imGuiManager.renderImGui(commandBuffer);
				renderer.endSwapChainRenderPass(commandBuffer);
				renderer.endFrame();
			}
		}

		vkDeviceWaitIdle(vrDevice.device());
	}

	void FirstApp::loadGameObjects() {
		std::shared_ptr<VrModel> vaseModel =
			VrModel::createModelFromFile(vrDevice, "../../../src/models/flat_vase.obj");
		auto flatVase = VrGameObject::createGameObject();
		flatVase.model = vaseModel;
		flatVase.transform.translation = { -.5f, .5f, 2.5f };
		flatVase.transform.scale = { 3.f, 1.5f, 3.f };
		gameObjects.push_back(std::move(flatVase));

		std::shared_ptr<VrModel> cubeModel = VrModel::createModelFromCube(vrDevice);
		auto cube = VrGameObject::createGameObject();
		cube.model = cubeModel;
		gameObjects.push_back(std::move(cube));

		std::shared_ptr<GaussianModel> gaussianModel = GaussianModel::createModelFromGaussians(vrDevice, gaussians);
		auto gaussian = VrGameObject::createGameObject();
		gaussian.gaussianModel = gaussianModel;
		gaussianObjects.push_back(std::move(gaussian));
	}

}