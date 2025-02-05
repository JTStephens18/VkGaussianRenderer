
#pragma once

#include "vr_window.hpp"
#include "vr_device.hpp"
#include "descriptors.hpp"
#include "renderer.hpp"

#include <memory>
#include <vector>

namespace vr {
	class ImGuiManager {
	public:

		ImGuiManager(VrWindow& window, VrDevice& device, Renderer& renderer);
		~ImGuiManager();

		ImGuiManager(const ImGuiManager&) = delete;
		ImGuiManager& operator = (const ImGuiManager&) = delete;

		void initImGui(VrWindow& vrWindow, VrDevice& device, Renderer& renderer);

		bool getImGuiFlag() const { return isImGuiEnabled; }
		void Vr_ImGui_CreateFontsTexture();
		void renderImGui(VkCommandBuffer commandBuffer);
	private:

		VrWindow& vrWindow;
		VrDevice& vrDevice;
		Renderer& vrRenderer;

		std::unique_ptr<VrDescriptorPool> imGuiPool{};

		bool isImGuiEnabled = true;
	};
}
