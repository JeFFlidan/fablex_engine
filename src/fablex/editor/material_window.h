#pragma once

#include "window_ui.h"
#include "core/uuid.h"
#include "asset_manager/fwd.h"

namespace fe::editor
{

class MaterialWindow : public WindowUI
{
public:
    MaterialWindow(asset::Material* material);

    virtual bool draw() override;

private:
    asset::Material* m_material = nullptr;

    UUID draw_texture_combo_box(UUID selectedTextureUUID, const std::string& label);
};

}