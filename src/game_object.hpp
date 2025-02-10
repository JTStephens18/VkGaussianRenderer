#pragma once

#include "vr_model.hpp"
#include "gaussian_model.hpp"

// libs
#include <glm/gtc/matrix_transform.hpp>

// std
#include <memory>
#include <unordered_map>

namespace vr {

    struct TransformComponent {
        glm::vec3 translation{};
        glm::vec3 scale{ 1.f, 1.f, 1.f };
        glm::vec3 rotation{};

        glm::mat4 mat4();

        glm::mat3 normalMatrix();
    };

    struct PointLightComponent {
        float lightIntensity = 1.0f;
    };

    class VrGameObject {
    public:
        using id_t = unsigned int;
        using Map = std::unordered_map<id_t, VrGameObject>;

        static VrGameObject createGameObject() {
            static id_t currentId = 0;
            return VrGameObject{ currentId++ };
        }

        static VrGameObject makePointLight(
            float intensity = 10.f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.f));

        void loadModels();

        VrGameObject(const VrGameObject&) = delete;
        VrGameObject& operator=(const VrGameObject&) = delete;
        VrGameObject(VrGameObject&&) = default;
        VrGameObject& operator=(VrGameObject&&) = default;

        id_t getId() { return id; }

        glm::vec3 color{};
        TransformComponent transform{};

        // Optional pointer components
        std::shared_ptr<VrModel> model{};
        std::shared_ptr<GaussianModel> gaussianModel{};
        std::unique_ptr<PointLightComponent> pointLight = nullptr;

    private:
        VrGameObject(id_t objId) : id{ objId } {}

        id_t id;
    };
}