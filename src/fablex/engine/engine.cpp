#include "engine.h"
#include "events.h"
#include "entities/model_entity.h"
#include "components/model_component.h"
#include "components/editor_camera_component.h"
#include "components/light_components.h"
#include "components/material_component.h"

#include "asset_manager/asset_manager.h"
#include "asset_manager/material/opaque_material_settings.h"
#include "core/file_system/file_system.h"
#include "core/task_composer.h"

namespace fe::engine
{

Engine::Engine()
{
    asset::AssetManager::init();
    asset::AssetRegistry::init();

    m_world = std::make_unique<World>();

    subscribe_to_events();
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

    std::vector<std::string> texturePaths = {
        "content/streaky-metal1_albedo.png",
        "content/streaky-metal1_ao.png",
        "content/streaky-metal1_metallic.png",
        "content/streaky-metal1_normal.png",
        "content/streaky-metal1_roughness.png",
        "content/rocky-rugged-terrain_1_albedo.png",
        "content/rocky-rugged-terrain_1_roughness.png",
        "content/rocky-rugged-terrain_1_metallic.png",
        "content/rocky-rugged-terrain_1_normal.png"
    };

    std::mutex mutex;
    std::unordered_map<std::string, asset::Texture*> textureByPath;

    TaskGroup textureTaskGroup; 

    for (auto& texturePath : texturePaths)
    {
        TaskComposer::execute(textureTaskGroup, [&mutex, &textureByPath, texturePath, projectDirectory](TaskExecutionInfo execInfo)
        {
            asset::TextureImportContext textureImportContext;
            textureImportContext.projectDirectory = projectDirectory;
            textureImportContext.originalFilePath = FileSystem::get_absolute_path(texturePath);
            asset::TextureImportResult importResult;
            asset::AssetManager::import_texture(textureImportContext, importResult);
            std::scoped_lock<std::mutex> locker(mutex);
            textureByPath[texturePath] = importResult.texture;
        });
    }

    TaskComposer::wait(textureTaskGroup);

    asset::ModelImportContext importContext;
    importContext.originalFilePath = FileSystem::get_absolute_path("content/horse.glb");
    importContext.projectDirectory = projectDirectory;
    importContext.mergeMeshes = true;
    asset::ModelImportResult importResult;

    asset::AssetManager::import_model(importContext, importResult);

    asset::OpaqueMaterialCreateInfo opaqueMaterialCreateInfo;
    opaqueMaterialCreateInfo.name = "Opaque1";
    opaqueMaterialCreateInfo.projectDirectory = projectDirectory;

    asset::Material* opaqueMaterial1 = asset::AssetManager::create_material(opaqueMaterialCreateInfo);
    auto opaqueMaterialSettings = opaqueMaterial1->material_settings<asset::OpaqueMaterialSettings>();

    opaqueMaterialSettings->set_base_color(Float4(0.5, 0.8, 0.1, 1));
    opaqueMaterialSettings->set_base_color_texture(textureByPath["content/streaky-metal1_albedo.png"]);
    opaqueMaterialSettings->set_roughness_texture(textureByPath["content/streaky-metal1_roughness.png"]);
    opaqueMaterialSettings->set_metallic_texture(textureByPath["content/streaky-metal1_metallic.png"]);
    opaqueMaterialSettings->set_normal_texture(textureByPath["content/streaky-metal1_normal.png"]);
    opaqueMaterialSettings->set_roughness(0.42f);
    opaqueMaterialSettings->set_metallic(0.0f);

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
    matComponent->add_material(opaqueMaterial1);

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
            matComponent->add_material(opaqueMaterial1);
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
    const float yOffset = 3.0f;

    for (uint32 j = 0; j != sphereRowCount; ++j)
    {
        for (uint32 i = 0; i != sphereColumnCount; ++i)
        {
            std::string postfix = std::to_string(i) + "_" + std::to_string(j);

            asset::OpaqueMaterialCreateInfo opaqueMaterialCreateInfo;
            opaqueMaterialCreateInfo.name = "Opaque" + postfix;
            opaqueMaterialCreateInfo.projectDirectory = projectDirectory;

            asset::Material* opaqueMaterial = asset::AssetManager::create_material(opaqueMaterialCreateInfo);
            auto opaqueMaterialSettings = opaqueMaterial->material_settings<asset::OpaqueMaterialSettings>();

            opaqueMaterialSettings->set_base_color(Float4(0.8, 0, 0, 1));
            opaqueMaterialSettings->set_roughness(roughnessValue * (i + 1));
            opaqueMaterialSettings->set_metallic(metallicValue);

            float x = xOffset + (-3.0f * i);
            float y = yOffset + 3.0f * j;
            float z = 0.0f;

            Entity* modelEntity = m_world->create_entity();
            modelEntity->set_name("Sphere_" + postfix);
            ModelComponent* modelComponent = modelEntity->create_component<ModelComponent>();
            modelComponent->set_model(importResult3.models.at(0));
            modelEntity->set_position(Float3(x, y, z));
            MaterialComponent* matComponent = modelEntity->create_component<MaterialComponent>();
            matComponent->add_material(opaqueMaterial);
        }

        metallicValue = 0.0f;
    }

    Entity* cameraEntity = m_world->create_entity();
    cameraEntity->set_name("Camera");
    EditorCameraComponent* cameraComponent = cameraEntity->create_component<EditorCameraComponent>();
    cameraComponent->mouseSensitivity = 0.12f;
    cameraComponent->movementSpeed = 50;

    Entity* lightEntity = m_world->create_entity();
    lightEntity->set_name("Sun");
    lightEntity->create_component<DirectionalLightComponent>()->intensity = 6;
    lightEntity->set_rotation(Float3(1, 0, 0), -50);

    importContext.originalFilePath = FileSystem::get_absolute_path("content/plane.glb");
    importContext.projectDirectory = projectDirectory;
    importContext.mergeMeshes = true;
    asset::ModelImportResult importResult4;

    asset::AssetManager::import_model(importContext, importResult4);

    Entity* planeEntity = m_world->create_entity();
    planeEntity->set_name("plane");
    modelComponent = planeEntity->create_component<ModelComponent>();
    modelComponent->set_model_uuid(importResult4.models.at(0)->get_uuid());
    planeEntity->set_scale(Float3(0.2, 0.2, 0.2));

    opaqueMaterialCreateInfo.name = "Plane Material";
    opaqueMaterialCreateInfo.projectDirectory = projectDirectory;

    asset::Material* opaqueMaterial2 = asset::AssetManager::create_material(opaqueMaterialCreateInfo);
    opaqueMaterialSettings = opaqueMaterial2->material_settings<asset::OpaqueMaterialSettings>();

    opaqueMaterialSettings->set_base_color(Float4(0.5, 0.5, 0.5, 1));
    opaqueMaterialSettings->set_base_color_texture(textureByPath["content/rocky-rugged-terrain_1_albedo.png"]);
    opaqueMaterialSettings->set_roughness_texture(textureByPath["content/rocky-rugged-terrain_1_roughness.png"]);
    opaqueMaterialSettings->set_metallic_texture(textureByPath["content/rocky-rugged-terrain_1_metallic.png"]);
    opaqueMaterialSettings->set_normal_texture(textureByPath["content/rocky-rugged-terrain_1_normal.png"]);
    opaqueMaterialSettings->set_roughness(0.35);
    opaqueMaterialSettings->set_metallic(0.0);

    matComponent = planeEntity->create_component<MaterialComponent>();
    matComponent->add_material(opaqueMaterial2);
}

void Engine::configure_sponza()
{
    std::string projectDirectory = "projects/sponza";
    
    if (load_project(projectDirectory))
        return;

    FileSystem::create_project_directory(projectDirectory);

    create_default_material();
    create_camera();
    create_sun();

    asset::ModelImportContext importContext;
    importContext.originalFilePath = FileSystem::get_absolute_path("content/sponza.glb");
    importContext.projectDirectory = projectDirectory;
    importContext.generateMaterials = true;
    
    asset::ModelImportResult importResult;

    asset::AssetManager::import_model(importContext, importResult);

    asset::OpaqueMaterialCreateInfo opaqueMaterialCreateInfo;
    opaqueMaterialCreateInfo.name = "Opaque1";
    opaqueMaterialCreateInfo.projectDirectory = projectDirectory;

    asset::Material* opaqueMaterial1 = asset::AssetManager::create_material(opaqueMaterialCreateInfo);
    auto opaqueMaterialSettings = opaqueMaterial1->material_settings<asset::OpaqueMaterialSettings>();

    opaqueMaterialSettings->set_base_color(Float4(0.5, 0.5, 0.5, 1));
    opaqueMaterialSettings->set_roughness(0.42f);
    opaqueMaterialSettings->set_metallic(0.0f);

    Entity* modelEntity = m_world->create_entity();
    modelEntity->set_rotation(Float3(0, 1, 0), 90);
    modelEntity->set_name("Sponza");
    auto modelComponent = modelEntity->create_component<ModelComponent>();
    modelComponent->set_model(importResult.models.at(0));
    auto matComponent = modelEntity->create_component<MaterialComponent>();
    matComponent->add_material(opaqueMaterial1);

    matComponent->init(modelComponent->get_model());
}

void Engine::create_project(const std::string& projectName)
{
    FileSystem::create_project(projectName);

    create_default_material();
    create_default_model();
    create_sun();
    create_camera();
}

// For now all resources will be loaded to the memory when project is opened
// Must change this logic in the future to make it smarter
bool Engine::load_project(const std::string& projectPath)
{
    if (!FileSystem::is_project_existed(projectPath))
        return false;

    FileSystem::set_project_path(projectPath);
    asset::AssetRegistry::init();

    std::string worldName = "world.felevel";
    std::string worldPath = FileSystem::get_absolute_path(FileSystem::get_project_path(), worldName);

    TaskGroup taskGroup;

    TaskComposer::execute(taskGroup, [&](TaskExecutionInfo)
    {
        Archive archive(worldPath);
        m_world->deserialize(archive);
    });

    asset::AssetManager::load_assets(taskGroup);

    TaskComposer::wait(taskGroup);

    return true;
}

void Engine::save_project()
{
    save_world();
    asset::AssetManager::save_assets();

    FE_LOG(LogDefault, INFO, "Saved project '{}'.", FileSystem::get_project_name());
}

void Engine::subscribe_to_events()
{
    EventManager::subscribe<ModelEntityCreationRequest>([this](const auto&)
    {
        m_world->create_entity(ModelEntity::get_static_type_info());
    });

    EventManager::subscribe<PointLightEntityCreationRequest>([this](const auto&)
    {
        Entity* entity = m_world->create_entity();
        entity->create_component<PointLightComponent>();
    });

    EventManager::subscribe<EntityRemovalRequest>([this](const auto& event)
    {
        m_world->remove_entity(event.entity());
    });

    EventManager::subscribe<ProjectSavingRequest>([this](const auto&)
    {
        save_project();
    });
}

void Engine::create_default_model()
{
    asset::ModelImportContext importContext;
    importContext.projectDirectory = FileSystem::get_project_path();
    importContext.originalFilePath = "content/sphere.glb";
    importContext.flags = asset::AssetFlag::USE_AS_DEFAULT;
    
    asset::ModelImportResult importResult;
    asset::AssetManager::import_model(importContext, importResult);
}

void Engine::create_default_material()
{
    asset::OpaqueMaterialCreateInfo createInfo;
    createInfo.roughness = 0.7;
    createInfo.metallic = 0.0;
    createInfo.baseColor = Float4(0.65, 0.65, 0.65, 1);
    createInfo.name = "DefaultGray";
    createInfo.projectDirectory = FileSystem::get_project_path();
    createInfo.flags |= asset::AssetFlag::USE_AS_DEFAULT;
    asset::AssetManager::create_material(createInfo);
}

void Engine::create_camera()
{
    m_cameraEntity = m_world->create_entity();
    m_cameraEntity->set_name("Camera");
    EditorCameraComponent* cameraComponent = m_cameraEntity->create_component<EditorCameraComponent>();
    cameraComponent->mouseSensitivity = 0.12f;
    cameraComponent->movementSpeed = 50;
    m_cameraEntity->set_position(Float3(0, 30, 0));
}

void Engine::create_sun()
{
    Entity* lightEntity = m_world->create_entity();
    lightEntity->set_name("Sun");
    lightEntity->create_component<DirectionalLightComponent>()->intensity = 3.5;
    lightEntity->set_rotation(Float3(1, 0, 0), -30);
}

void Engine::save_world()
{
    std::string worldName = "world.felevel";
    std::string path = FileSystem::get_absolute_path(FileSystem::get_project_path(), worldName);

    Archive archive;
    m_world->serialize(archive);
    archive.save(path);
}

}