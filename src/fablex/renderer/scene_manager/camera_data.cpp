#include "camera_data.h"
#include "engine/entity/entity.h"
#include "engine/components/camera_component.h"

namespace fe::renderer
{

void CameraData::fill(engine::CameraComponent* cameraComponent)
{
    position = cameraComponent->get_entity()->get_position();
    view = cameraComponent->view;
    projection = cameraComponent->projection;
    viewProjection = cameraComponent->viewProjection;
    prevViewProjection = cameraComponent->prevViewProjection;
    inverseView = cameraComponent->inverseView;
    inverseProjection = cameraComponent->inverseProjection;
    inverseViewProjection = cameraComponent->inverseViewProjection;
    zNear = cameraComponent->zNear;
    zFar = cameraComponent->zFar;
    create_frustum();
}

}