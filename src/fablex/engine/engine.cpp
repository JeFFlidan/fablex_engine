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
    importContext.originalFilePath = FileSystem::get_absolute_path("content/horse.glb");
    importContext.projectDirectory = projectDirectory;
    importContext.mergeMeshes = true;
    asset::ModelImportResult importResult;

    asset::AssetManager::import_model(importContext, importResult);

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
        }
    }

    importContext.originalFilePath = FileSystem::get_absolute_path("content/boulder.glb");
    importContext.projectDirectory = projectDirectory;
    importContext.mergeMeshes = true;
    asset::ModelImportResult importResult2;

    asset::AssetManager::import_model(importContext, importResult2);

    Entity* modelEntity = m_world->create_entity();
    modelEntity->set_name("Boulder");
    ModelComponent* modelComponent = modelEntity->create_component<ModelComponent>();
    modelComponent->set_model(importResult2.models.at(0));
    modelEntity->set_position(Float3(-5, 0, 0));

    Entity* cameraEntity = m_world->create_entity();
    cameraEntity->set_name("Camera");
    EditorCameraComponent* cameraComponent = cameraEntity->create_component<EditorCameraComponent>();
    cameraComponent->mouseSensitivity = 0.12f;
    cameraComponent->movementSpeed = 9.0f;
    cameraComponent->window = m_window;
}

}