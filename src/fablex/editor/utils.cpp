#include "utils.h"
#include "core/object.h"
#include "imgui.h"

namespace fe::editor
{

void Utils::draw_properties_ui(const PropertyArray& properties, Object* object)
{
    for (Property* property : properties)
    {
        float minValue = 0.0f;
        float maxValue = 0.0f;
        float speed = 1.0f;

        if (const ClampMin* clampMin = property->get_attribute<ClampMin>())
            minValue = clampMin->min;

        if (const ClampMax* clampMax = property->get_attribute<ClampMax>())
            maxValue = clampMax->max;

        if (const SliderSpeed* sliderSpeed = property->get_attribute<SliderSpeed>())
            speed = sliderSpeed->speed;

        switch (property->get_type())
        {
            case PropertyType::BOOL:
            {
                bool value = property->get_value<bool>(object);
                ImGui::Checkbox(property->get_name().c_str(), &value);
                property->set_value(object, value);
                break;
            }
            case PropertyType::INTEGER:
            {
                int32 value = property->get_value<int32>(object);
                ImGui::SliderInt(property->get_name().c_str(), &value, (int)minValue, (int)maxValue);
                property->set_value(object, value);
                break;
            }
            case PropertyType::UUID:
            {

            }
            case PropertyType::FLOAT:
            {
                float value = property->get_value<float>(object);
                ImGui::DragFloat(property->get_name().c_str(), &value, speed, minValue, maxValue);
                property->set_value(object, value);
                break;
            }
            case PropertyType::FLOAT2:
            {
                Float2 value = property->get_value<Float2>(object);
                ImGui::DragFloat2(property->get_name().c_str(), &value.x, speed, minValue, maxValue);
                property->set_value(object, value);
                break;
            }
            case PropertyType::FLOAT3:
            {
                Float3 value = property->get_value<Float3>(object);
                ImGui::DragFloat3(property->get_name().c_str(), &value.x, speed, minValue, maxValue);
                property->set_value(object, value);
                break;
            }
            case PropertyType::FLOAT4:
            {
                Float4 value = property->get_value<Float4>(object);

                if (property->get_attribute<Color>())
                    ImGui::ColorEdit4(property->get_name().c_str(), &value.x);
                else
                    ImGui::DragFloat4(property->get_name().c_str(), &value.x, speed, minValue, maxValue);

                property->set_value(object, value);
                break;
            }
            case PropertyType::FLOAT3X4:
            {
                FE_LOG(LogEditor, ERROR, "Float3x4 property UI is not implemented.");
                break;
            }
            case PropertyType::FLOAT4X4:
            {
                FE_LOG(LogEditor, ERROR, "Float4x4 property UI is not implemented.");
                break;
            }
            case PropertyType::QUAT:
            {
                FE_LOG(LogEditor, ERROR, "Quat property UI is not implemented.");
                break;
            }
            case PropertyType::STRING:
            {
                FE_LOG(LogEditor, ERROR, "String property UI is not implemented.");
                break;
            }
            case PropertyType::ARRAY:
            {
                FE_LOG(LogEditor, ERROR, "Array property UI is not implemented.");
                break;
            }
        }
    }
}

}