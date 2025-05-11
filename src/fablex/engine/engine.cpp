#include "engine.h"
#include "asset_manager/asset_manager.h"
#include "core/file_system/file_system.h"
#include "components/model_component.h"
#include "components/editor_camera_component.h"
#include "components/light_components.h"
#include "components/material_component.h"

namespace fe::engine
{

Engine::Engine()
{
    asset::AssetManager::init();
    asset::AssetRegistry::init();

    m_world = std::make_unique<World>();
}

void Engine::update()
{
    m_world->update_pre_entities_update();
    m_world->update_camera_entities();
}

void Engine::configure_test_scene()
{
    std::string projectDirectory = "projects/3d_model_rendering";
    FileSystem::create_project_directory(projectDirectory);

    asset::ModelImportContext importContext;
    importContext.originalFilePath = FileSystem::get_absolute_path("content/horse.glb");
    importContext.projectDirectory = projectDirectory;
    importContext.mergeMeshes = true;
    asset::ModelImportResult importResult;

    asset::AssetManager::import_model(importContext, importResult);
    asset::OpaqueMaterial* opaqueMaterial1 = (asset::OpaqueMaterial*)asset::AssetManager::create_material(
        {{"Opaque1", projectDirectory}, asset::MaterialType::OPAQUE});

    opaqueMaterial1->set_base_color(Float4(0.5, 0.8, 0.1, 1));
    opaqueMaterial1->set_roughness(0.5f);
    opaqueMaterial1->set_metallic(1.0f);

    importContext.originalFilePath = FileSystem::get_absolute_path("content/boulder.glb");
    importContext.projectDirectory = projectDirectory;
    importContext.mergeMeshes = true;
    asset::ModelImportResult importResult2;

    asset::AssetManager::import_model(importContext, importResult2);

    Entity* modelEntity = m_world->create_entity();
    modelEntity->set_name("Boulder");
    ModelComponent* modelComponent = modelEntity->create_component<ModelComponent>();
    modelComponent->set_model(importResult2.models.at(0));
    modelEntity->set_position(Float3(-2, 0, 5));
    MaterialComponent* matComponent = modelEntity->create_component<MaterialComponent>();
    matComponent->set_material(opaqueMaterial1);

    const uint32 instanceColumnCount = 20;
    const uint32 instanceRowCount = 10;

    for (uint32 j = 0; j != instanceRowCount; ++j)
    {
        for (uint32 i = 0; i != instanceColumnCount; ++i)
        {
            float x = 3.0f * i;
            float y = 0;
            float z = 3.0f * j;

            Entity* modelEntity = m_world->create_entity();
            modelEntity->set_name("Model_" + std::to_string(j) + "_" + std::to_string(i));
            ModelComponent* modelComponent = modelEntity->create_component<ModelComponent>();
            modelComponent->set_model(importResult.models.at(0));
            modelEntity->set_position(Float3(x, y, z));
            MaterialComponent* matComponent = modelEntity->create_component<MaterialComponent>();
            matComponent->set_material(opaqueMaterial1);
        }
    }

    importContext.originalFilePath = FileSystem::get_absolute_path("content/sphere.glb");
    importContext.projectDirectory = projectDirectory;
    importContext.mergeMeshes = true;
    asset::ModelImportResult importResult3;

    asset::AssetManager::import_model(importContext, importResult3);

    const uint32 sphereColumnCount = 10;
    const uint32 sphereRowCount = 2;

    float metallicValue = 1.0f;
    const float roughnessValue = 0.1f;
    const float xOffset = -10.0f;
    const float yOffset = 10.0f;

    for (uint32 j = 0; j != sphereRowCount; ++j)
    {
        for (uint32 i = 0; i != sphereColumnCount; ++i)
        {
            std::string postfix = std::to_string(i) + "_" + std::to_string(j);

            asset::OpaqueMaterial* opaqueMaterial = (asset::OpaqueMaterial*)asset::AssetManager::create_material(
                {{"Opaque" + postfix, projectDirectory}, asset::MaterialType::OPAQUE});

            opaqueMaterial->set_base_color(Float4(1, 0, 0, 1));
            opaqueMaterial->set_roughness(roughnessValue * (i + 1));
            opaqueMaterial->set_metallic(metallicValue);

            float x = xOffset + (-3.0f * i);
            float y = yOffset + 3.0f * j;
            float z = 0.0f;

            Entity* modelEntity = m_world->create_entity();
            modelEntity->set_name("Sphere_" + postfix);
            ModelComponent* modelComponent = modelEntity->create_component<ModelComponent>();
            modelComponent->set_model(importResult3.models.at(0));
            modelEntity->set_position(Float3(x, y, z));
            MaterialComponent* matComponent = modelEntity->create_component<MaterialComponent>();
            matComponent->set_material(opaqueMaterial);
        }

        metallicValue = 0.0f;
    }

    Entity* cameraEntity = m_world->create_entity();
    cameraEntity->set_name("Camera");
    EditorCameraComponent* cameraComponent = cameraEntity->create_component<EditorCameraComponent>();
    cameraComponent->mouseSensitivity = 0.12f;
    cameraComponent->movementSpeed = 9.0f;
    cameraComponent->window = m_window;

    Entity* lightEntity = m_world->create_entity();
    lightEntity->set_name("Sun");
    lightEntity->create_component<DirectionalLightComponent>()->intensity = 3.5;
    lightEntity->set_rotation(Float3(1, 0, 0), -30);

    importContext.originalFilePath = FileSystem::get_absolute_path("content/plane.glb");
    importContext.projectDirectory = projectDirectory;
    importContext.mergeMeshes = true;
    asset::ModelImportResult importResult4;

    asset::AssetManager::import_model(importContext, importResult4);

    Entity* planeEntity = m_world->create_entity();
    planeEntity->set_name("plane");
    modelComponent = planeEntity->create_component<ModelComponent>();
    modelComponent->set_model_uuid(importResult4.models.at(0)->get_uuid());

    asset::OpaqueMaterial* opaqueMaterial = (asset::OpaqueMaterial*)asset::AssetManager::create_material(
        {{"Plane Material", projectDirectory}, asset::MaterialType::OPAQUE});

    opaqueMaterial->set_base_color(Float4(0.5, 0.5, 0.5, 1));
    opaqueMaterial->set_roughness(0.7);
    opaqueMaterial->set_metallic(0.0);
    FE_LOG(LogDefault, INFO, "Model: {}", importResult4.models.at(0)->get_name());
    matComponent = planeEntity->create_component<MaterialComponent>();
    matComponent->set_material(opaqueMaterial);
}

}