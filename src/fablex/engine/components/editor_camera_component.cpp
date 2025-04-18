#include "editor_camera_component.h"
#include "engine/entity/entity.h"
#include "core/input.h"

namespace fe::engine
{

FE_DEFINE_OBJECT(EditorCameraComponent, CameraComponent);

void EditorCameraComponent::update(float deltaTime)
{
    FE_CHECK(window);
    FE_CHECK(m_entity);

    read_input(deltaTime);
    update_matrices();
}

void EditorCameraComponent::read_input(float deltaTime)
{
    if (!Input::is_mouse_button_pressed(MouseButton::RIGHT))
        return;

    Float2 deltaPosition = Input::get_delta_position();
    float xDelta = deltaPosition.x;
    float yDelta = deltaPosition.y;

    xDelta = mouseSensitivity * xDelta / (1.0f * 60.0f);
    yDelta = mouseSensitivity * yDelta / (1.0f * 60.0f);

    float clampedDeltaTime = std::min(deltaTime, 0.1f);
    bool isShiftPressed = Input::is_key_pressed(Key::LEFT_SHIFT);
    float speed = movementSpeed * (isShiftPressed ? 5.0f : 1.0f);

    Vector move = Vector4::create(0.0f);
    if (Input::is_key_pressed(Key::A))
        move += Vector4::create(-1.0f, 0.0f, 0.0f, 0.0f);
    if (Input::is_key_pressed(Key::D))
        move += Vector4::create(1.0f, 0.0f, 0.0f, 0.0f);
    if (Input::is_key_pressed(Key::W))
        move += Vector4::create(0.0f, 0.0f, 1.0f, 0.0f);
    if (Input::is_key_pressed(Key::S))
        move += Vector4::create(0.0f, 0.0f, -1.0f, 0.0f);

    float velocity = speed * clampedDeltaTime;
    move *= Vector4::create(velocity);

    float moveLength = Vector3::length(move);
    if (abs(xDelta) + abs(yDelta) > 0 || moveLength > 0.0001f)
    {
        Matrix cameraRotation = Matrix::rotation(m_entity->get_rotation());
        Vector rotatedMove = Vector3::transform_normal(move, cameraRotation);
        m_entity->translate(rotatedMove);
        m_entity->set_rotation(Float3(yDelta, xDelta, 0.0f), AngleUnit::RADIANS);
    }
}

void EditorCameraComponent::update_matrices()
{
    Vector eyeBase = Vector4::create(0.0f, 0.0f, 0.0f, 1.0f);
    Vector atBase = Vector4::create(0.0f, 0.0f, 1.0f, 0.0f);
    Vector upBase = Vector4::create(0.0f, 1.0f, 0.0f, 0.0f);

    float width = window->get_info().width;
    float height = window->get_info().height;

    Matrix worldTransform(m_entity->get_world_transform());
    eye = Vector3::transform(eyeBase, worldTransform);
    at = Vector3::normalize(Vector3::transform_normal(atBase, worldTransform));
    up = Vector3::normalize(Vector3::transform_normal(upBase, worldTransform));

    Matrix viewMat = Matrix::look_to_lh(eye, at, up);
    Matrix projectionMat = Matrix::perspective_for_lh(to_radians(fov), width / height, zFar, zNear);
    Matrix viewProjectionMat = viewMat * projectionMat;

    view = viewMat;
    projection = projectionMat;
    viewProjection = viewProjectionMat;
    
    inverseView = viewMat.inverse();
    inverseProjection = projectionMat.inverse();
    inverseViewProjection = viewProjectionMat.inverse(); 
}

}