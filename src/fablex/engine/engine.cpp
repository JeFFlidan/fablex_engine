#include "engine.h"
#include "asset_manager/asset_manager.h"
#include "core/file_system/file_system.h"
#include "components/model_component.h"
#include "components/editor_camera_component.h"

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
    importContext.originalFilePath = FileSystem::get_absolute_path("content/cubes.glb");
    importContext.projectDirectory = projectDirectory;
    importContext.mergeMeshes = true;
    asset::ModelImportResult importResult;

    asset::AssetManager::import_model(importContext, importResult);

    Entity* modelEntity = m_world->create_entity();
    modelEntity->set_name("Model");
    FE_LOG(LogDefault, INFO, "Entity name: {}", modelEntity->get_name());
    ModelComponent* modelComponent = modelEntity->create_component<ModelComponent>();
    modelComponent->set_model(importResult.models.at(0));

    Entity* cameraEntity = m_world->create_entity();
    cameraEntity->set_name("Camera");
    EditorCameraComponent* cameraComponent = cameraEntity->create_component<EditorCameraComponent>();
    cameraComponent->mouseSensitivity = 0.12f;
    cameraComponent->movementSpeed = 60.0f;
    cameraComponent->window = m_window;
}

}