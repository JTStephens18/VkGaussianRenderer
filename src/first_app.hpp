#pragma once

#include "vr_window.hpp"
#include "descriptors.hpp"
#include "game_object.hpp"
#include "vr_device.hpp"
#include "renderer.hpp"
#include "imgui_manager.hpp"

#include <memory>
#include <vector>

namespace vr {
	class FirstApp {
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		FirstApp();
		~FirstApp();

		FirstApp(const FirstApp&) = delete;
		FirstApp& operator=(const FirstApp&) = delete;


		void run();

	private:

		void loadGameObjects();

		VrWindow vrWindow{ WIDTH, HEIGHT, "Hello Vulkan" };
		VrDevice vrDevice{ vrWindow };
		Renderer renderer{ vrWindow, vrDevice };

		std::unique_ptr<VrDescriptorPool> globalPool{};
		std::vector<VrGameObject> gameObjects;

		//ImGuiManager imGuiManager{ vrWindow, vrDevice, renderer };
	};
}

