#include "panels/properties_widget.h"

#include "bridge/editor_engine_bridge.h"
#include "editor_context.h"
#include "scene/components/lighting/area.h"
#include "scene/components/lighting/directional.h"
#include "scene/components/lighting/point.h"
#include "scene/components/lighting/spot.h"
#include "scene/components/renderable.h"
#include "scene/components/transform.h"
#include "scene/scene.h"

#include "render/render_mesh.h"
#include "render/render_object.h"
#include "render/rhi/rhi_texture.h"

#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace RealmEngine
{
    PropertiesWidget::PropertiesWidget(const std::shared_ptr<EditorContext>& context, EditorEngineBridge& bridge) :
        Widget("Properties"), m_context(context), m_bridge(&bridge)
    {}

    void PropertiesWidget::render()
    {
        ImGui::Begin(m_name.c_str(), &m_open);

        const auto scene = m_bridge->getCurrentScene();
        if (!m_context || !m_context->hasSelectedEntity() || !scene)
        {
            ImGui::Text("No entity selected");
            ImGui::End();
            return;
        }

        const entt::entity entity = m_context->getSelectedEntity();
        if (!scene->valid(entity))
        {
            ImGui::Text("Invalid entity");
            ImGui::End();
            return;
        }

        if (scene->has<Transform>(entity))
            renderTransform();
        if (scene->has<Renderable>(entity))
            renderRenderable();
        if (scene->has<PointLight>(entity))
            renderPointLight();
        if (scene->has<SpotLight>(entity))
            renderSpotLight();
        if (scene->has<DirectionalLight>(entity))
            renderDirectionalLight();
        if (scene->has<AreaLight>(entity))
            renderAreaLight();

        ImGui::End();
    }

    void PropertiesWidget::renderTransform()
    {
        const auto         scene  = m_bridge->getCurrentScene();
        const entt::entity entity = m_context->getSelectedEntity();
        auto*              tf     = scene->tryGet<Transform>(entity);
        if (!tf)
            return;

        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::DragFloat3("Position", &tf->position.x, 0.1f))
                scene->markDirty();

            glm::vec3 euler_degrees = glm::degrees(tf->getEulerAngles());
            if (ImGui::DragFloat3("Rotation", &euler_degrees.x, 1.0f))
            {
                tf->rotation = glm::quat(glm::radians(euler_degrees));
                scene->markDirty();
            }

            if (ImGui::DragFloat3("Scale", &tf->scale.x, 0.1f))
                scene->markDirty();
        }
    }

    void PropertiesWidget::renderRenderable()
    {
        const auto         scene  = m_bridge->getCurrentScene();
        const entt::entity entity = m_context->getSelectedEntity();
        const auto*        r      = scene->tryGet<Renderable>(entity);
        if (!r)
            return;

        if (ImGui::CollapsingHeader("Renderable", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (!r->model_path.empty())
            {
                char buffer[256];
                strncpy(buffer, r->model_path.c_str(), sizeof(buffer) - 1);
                buffer[sizeof(buffer) - 1] = '\0';
                ImGui::InputText("Model Path", buffer, sizeof(buffer), ImGuiInputTextFlags_ReadOnly);
            }
            else
            {
                ImGui::Text("No model path");
            }

            ImGui::Text(r->render_object ? "Render Object: Loaded" : "Render Object: Not loaded");

            if (r->render_object && ImGui::TreeNodeEx("Meshes", ImGuiTreeNodeFlags_DefaultOpen))
            {
                const size_t mesh_count = r->render_object->getMeshCount();
                for (size_t i = 0; i < mesh_count; ++i)
                {
                    RenderMesh* mesh = r->render_object->getMesh(i);
                    if (!mesh)
                        continue;

                    ImGui::PushID(static_cast<int>(i));
                    std::string mesh_label = mesh->m_name.empty() ? "Mesh " + std::to_string(i) : mesh->m_name;
                    if (ImGui::TreeNodeEx(mesh_label.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        ImGui::Text("Vertices: %zu", mesh->m_vertices.size());
                        ImGui::Text("Indices: %zu", mesh->m_indices.size());
                        ImGui::Text("Triangles: %d", mesh->getTriangleCount());

                        std::string mat_label = mesh->m_material.name.empty() ? "Material" : mesh->m_material.name;
                        if (ImGui::TreeNodeEx(mat_label.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            renderMaterialEditor(mesh->m_material);
                            ImGui::TreePop();
                        }
                        ImGui::TreePop();
                    }
                    ImGui::PopID();
                }
                ImGui::TreePop();
            }
        }
    }

    void PropertiesWidget::renderTextureSlot(const char*                  label,
                                             const std::string&           use_key,
                                             const std::string&           tex_key,
                                             const MaterialPropertyBlock& props)
    {
        ImGui::Text("%s:", label);
        ImGui::SameLine(140);
        const auto tex     = props.getTexture(tex_key);
        const bool use_tex = props.getBool(use_key);
        if (tex)
        {
            ImGui::Text("(%dx%d) %s", tex->getWidth(), tex->getHeight(), use_tex ? "on" : "off");
            constexpr float   preview_sz = 48.0f;
            const ImTextureID tid        = static_cast<ImTextureID>(static_cast<intptr_t>(tex->getNativeHandle()));
            ImGui::Image(tid, ImVec2(preview_sz, preview_sz), ImVec2(0, 1), ImVec2(1, 0));
        }
        else
        {
            ImGui::TextDisabled("[none] %s", use_tex ? "on" : "off");
        }
    }

    void PropertiesWidget::renderMaterialEditor(Material& mat)
    {
        const auto scene = m_bridge->getCurrentScene();
        auto&      props = mat.properties;

        // Shading Model
        static const char* s_shading_names[] = {"Standard PBR", "Unlit", "Custom"};
        int                sm                = static_cast<int>(mat.shading_model);
        if (ImGui::Combo("Shading Model", &sm, s_shading_names, IM_ARRAYSIZE(s_shading_names)))
        {
            mat.shading_model = static_cast<ShadingModel>(sm);
            scene->markDirty();
        }

        // Blend Mode
        static const char* s_blend_names[] = {"Opaque", "Alpha Test", "Transparent"};
        int                bm              = static_cast<int>(mat.blend_mode);
        if (ImGui::Combo("Blend Mode", &bm, s_blend_names, IM_ARRAYSIZE(s_blend_names)))
        {
            mat.blend_mode = static_cast<BlendMode>(bm);
            scene->markDirty();
        }

        if (mat.blend_mode == BlendMode::AlphaTest)
        {
            if (ImGui::DragFloat("Alpha Cutoff", &mat.alpha_cutoff, 0.01f, 0.0f, 1.0f))
            {
                props.setFloat("material.alphaCutout", mat.alpha_cutoff);
                scene->markDirty();
            }
        }

        // Render Face
        static const char* s_face_names[] = {"Front", "Back", "Both"};
        int                rf             = static_cast<int>(mat.render_face);
        if (ImGui::Combo("Render Face", &rf, s_face_names, IM_ARRAYSIZE(s_face_names)))
        {
            mat.render_face = static_cast<RenderFace>(rf);
            scene->markDirty();
        }

        if (ImGui::DragInt("Render Queue", &mat.render_queue, 1, 0, 5000))
            scene->markDirty();

        // Custom shader paths
        if (mat.shading_model == ShadingModel::Custom || mat.hasCustomShader())
        {
            ImGui::Separator();
            ImGui::Text("Shader");

            char vbuf[512], fbuf[512];
            strncpy(vbuf, mat.vert_path.c_str(), sizeof(vbuf) - 1);
            vbuf[sizeof(vbuf) - 1] = '\0';
            strncpy(fbuf, mat.frag_path.c_str(), sizeof(fbuf) - 1);
            fbuf[sizeof(fbuf) - 1] = '\0';

            if (ImGui::InputText("Vertex Shader", vbuf, sizeof(vbuf)))
            {
                mat.vert_path = vbuf;
                scene->markDirty();
            }
            if (ImGui::InputText("Fragment Shader", fbuf, sizeof(fbuf)))
            {
                mat.frag_path = fbuf;
                scene->markDirty();
            }

            if (ImGui::Button("Reload Shaders"))
                m_bridge->reloadCustomShaders();
        }

        ImGui::Separator();

        // Standard PBR properties
        if (mat.shading_model == ShadingModel::StandardPBR)
        {
            glm::vec3 albedo = props.getVec3("material.albedo", glm::vec3(0.7f));
            if (ImGui::ColorEdit3("Albedo", &albedo.x))
            {
                props.setVec3("material.albedo", albedo);
                scene->markDirty();
            }

            float metallic = props.getFloat("material.metallic");
            if (ImGui::DragFloat("Metallic", &metallic, 0.01f, 0.0f, 1.0f))
            {
                props.setFloat("material.metallic", metallic);
                scene->markDirty();
            }

            float roughness = props.getFloat("material.roughness", 0.5f);
            if (ImGui::DragFloat("Roughness", &roughness, 0.01f, 0.0f, 1.0f))
            {
                props.setFloat("material.roughness", roughness);
                scene->markDirty();
            }

            float ao = props.getFloat("material.ambientOcclusion", 1.0f);
            if (ImGui::DragFloat("Ambient Occlusion", &ao, 0.01f, 0.0f, 1.0f))
            {
                props.setFloat("material.ambientOcclusion", ao);
                scene->markDirty();
            }

            float opacity = props.getFloat("material.opacity", 1.0f);
            if (ImGui::DragFloat("Opacity", &opacity, 0.01f, 0.0f, 1.0f))
            {
                props.setFloat("material.opacity", opacity);
                scene->markDirty();
            }

            glm::vec3 emissive = props.getVec3("material.emissive");
            if (ImGui::ColorEdit3("Emissive", &emissive.x))
            {
                props.setVec3("material.emissive", emissive);
                scene->markDirty();
            }

            float emissive_str = props.getFloat("material.emissiveStrength", 1.0f);
            if (ImGui::DragFloat("Emissive Strength", &emissive_str, 0.01f, 0.0f, 10.0f))
            {
                props.setFloat("material.emissiveStrength", emissive_str);
                scene->markDirty();
            }

            // SSS
            bool sss = props.getBool("material.subsurfaceEnabled");
            if (ImGui::Checkbox("Subsurface", &sss))
            {
                props.setBool("material.subsurfaceEnabled", sss);
                scene->markDirty();
            }
            if (sss)
            {
                float sss_radius = props.getFloat("material.subsurfaceRadius", 1.0f);
                if (ImGui::DragFloat("SSS Radius", &sss_radius, 0.1f, 0.1f, 10.0f))
                {
                    props.setFloat("material.subsurfaceRadius", sss_radius);
                    scene->markDirty();
                }
                glm::vec3 sss_color = props.getVec3("material.subsurfaceColor", glm::vec3(1.0f, 0.2f, 0.1f));
                if (ImGui::ColorEdit3("SSS Color", &sss_color.x))
                {
                    props.setVec3("material.subsurfaceColor", sss_color);
                    scene->markDirty();
                }
            }

            // Textures
            ImGui::Separator();
            ImGui::Text("Textures");
            renderTextureSlot("Albedo", "material.useTextureAlbedo", "material.textureAlbedo", props);
            renderTextureSlot(
                "MetRough", "material.useTextureMetallicRoughness", "material.textureMetallicRoughness", props);
            renderTextureSlot("Normal", "material.useTextureNormal", "material.textureNormal", props);
            renderTextureSlot("AO", "material.useTextureAmbientOcclusion", "material.textureAmbientOcclusion", props);
            renderTextureSlot("Emissive", "material.useTextureEmissive", "material.textureEmissive", props);
            renderTextureSlot("Opacity", "material.useTextureOpacity", "material.textureOpacity", props);
        }

        // Dynamic properties for Custom / Unlit
        if (mat.shading_model != ShadingModel::StandardPBR)
        {
            ImGui::Separator();
            ImGui::Text("Properties");

            static const char* s_type_names[] = {"Bool", "Float", "Int", "Vec2", "Vec3", "Vec4", "Texture2D"};

            auto& all_props = props.getProperties();
            for (size_t p = 0; p < all_props.size(); ++p)
            {
                auto& [pname, prop] = all_props[p];
                if (prop.type == PropType::Texture2D)
                    continue;

                ImGui::PushID(static_cast<int>(p));

                char name_buf[128];
                strncpy(name_buf, pname.c_str(), sizeof(name_buf) - 1);
                name_buf[sizeof(name_buf) - 1] = '\0';
                ImGui::SetNextItemWidth(140);
                if (ImGui::InputText("##name", name_buf, sizeof(name_buf)))
                {
                    pname = name_buf;
                    scene->markDirty();
                }

                ImGui::SameLine();

                int type_idx = static_cast<int>(prop.type);
                ImGui::SetNextItemWidth(80);
                if (ImGui::Combo("##type", &type_idx, s_type_names, IM_ARRAYSIZE(s_type_names)))
                {
                    prop.type = static_cast<PropType>(type_idx);
                    scene->markDirty();
                }

                ImGui::SameLine();

                bool changed = false;
                switch (prop.type)
                {
                    case PropType::Bool: {
                        bool bv = prop.values[0] != 0.0f;
                        if (ImGui::Checkbox("##val", &bv))
                        {
                            prop.values[0] = bv ? 1.0f : 0.0f;
                            changed        = true;
                        }
                        break;
                    }
                    case PropType::Float:
                        ImGui::SetNextItemWidth(100);
                        changed = ImGui::DragFloat("##val", &prop.values[0], 0.01f);
                        break;
                    case PropType::Int: {
                        int iv = static_cast<int>(prop.values[0]);
                        ImGui::SetNextItemWidth(100);
                        if (ImGui::DragInt("##val", &iv))
                        {
                            prop.values[0] = static_cast<float>(iv);
                            changed        = true;
                        }
                        break;
                    }
                    case PropType::Vec2:
                        ImGui::SetNextItemWidth(160);
                        changed = ImGui::DragFloat2("##val", prop.values, 0.01f);
                        break;
                    case PropType::Vec3:
                        ImGui::SetNextItemWidth(200);
                        changed = ImGui::DragFloat3("##val", prop.values, 0.01f);
                        break;
                    case PropType::Vec4:
                        ImGui::SetNextItemWidth(240);
                        changed = ImGui::DragFloat4("##val", prop.values, 0.01f);
                        break;
                    case PropType::Texture2D:
                        break;
                }
                if (changed)
                    scene->markDirty();

                ImGui::SameLine();
                if (ImGui::Button("X"))
                {
                    all_props.erase(all_props.begin() + static_cast<ptrdiff_t>(p));
                    scene->markDirty();
                    ImGui::PopID();
                    break;
                }

                ImGui::PopID();
            }

            if (ImGui::Button("+ Add Property"))
            {
                props.setFloat("newParam", 0.0f);
                scene->markDirty();
            }
        }
    }

    void PropertiesWidget::renderPointLight()
    {
        const auto         scene  = m_bridge->getCurrentScene();
        const entt::entity entity = m_context->getSelectedEntity();
        auto*              pl     = scene->tryGet<PointLight>(entity);
        if (!pl)
            return;

        if (ImGui::CollapsingHeader("Point Light", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::Checkbox("Enabled", &pl->enabled))
                scene->markDirty();
            if (ImGui::ColorEdit3("Color", &pl->color.x, ImGuiColorEditFlags_Float))
                scene->markDirty();
            if (ImGui::DragFloat("Intensity", &pl->intensity, 0.1f, 0.0f, 100.0f))
                scene->markDirty();
            if (ImGui::DragFloat("Range", &pl->range, 1.0f, 0.0f, 1000.0f))
                scene->markDirty();

            if (ImGui::TreeNode("Attenuation"))
            {
                if (ImGui::DragFloat("Constant", &pl->constant, 0.01f, 0.0f, 10.0f))
                    scene->markDirty();
                if (ImGui::DragFloat("Linear", &pl->linear, 0.001f, 0.0f, 1.0f))
                    scene->markDirty();
                if (ImGui::DragFloat("Quadratic", &pl->quadratic, 0.0001f, 0.0f, 1.0f))
                    scene->markDirty();
                ImGui::TreePop();
            }
        }
    }

    void PropertiesWidget::renderSpotLight()
    {
        const auto         scene  = m_bridge->getCurrentScene();
        const entt::entity entity = m_context->getSelectedEntity();
        auto*              sl     = scene->tryGet<SpotLight>(entity);
        if (!sl)
            return;

        if (ImGui::CollapsingHeader("Spot Light", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::Checkbox("Enabled", &sl->enabled))
                scene->markDirty();
            if (ImGui::ColorEdit3("Color", &sl->color.x, ImGuiColorEditFlags_Float))
                scene->markDirty();
            if (ImGui::DragFloat("Intensity", &sl->intensity, 0.1f, 0.0f, 100.0f))
                scene->markDirty();
            if (ImGui::DragFloat("Range", &sl->range, 1.0f, 0.0f, 1000.0f))
                scene->markDirty();
            if (ImGui::DragFloat("Inner Cone Angle", &sl->inner_cone_angle, 1.0f, 0.0f, 180.0f))
                scene->markDirty();
            if (ImGui::DragFloat("Outer Cone Angle", &sl->outer_cone_angle, 1.0f, 0.0f, 180.0f))
                scene->markDirty();

            if (ImGui::TreeNode("Attenuation"))
            {
                if (ImGui::DragFloat("Constant", &sl->constant, 0.01f, 0.0f, 10.0f))
                    scene->markDirty();
                if (ImGui::DragFloat("Linear", &sl->linear, 0.001f, 0.0f, 1.0f))
                    scene->markDirty();
                if (ImGui::DragFloat("Quadratic", &sl->quadratic, 0.0001f, 0.0f, 1.0f))
                    scene->markDirty();
                ImGui::TreePop();
            }
        }
    }

    void PropertiesWidget::renderDirectionalLight()
    {
        const auto         scene  = m_bridge->getCurrentScene();
        const entt::entity entity = m_context->getSelectedEntity();
        auto*              dl     = scene->tryGet<DirectionalLight>(entity);
        if (!dl)
            return;

        if (ImGui::CollapsingHeader("Directional Light", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::Checkbox("Enabled", &dl->enabled))
                scene->markDirty();
            if (ImGui::ColorEdit3("Color", &dl->color.x, ImGuiColorEditFlags_Float))
                scene->markDirty();
            if (ImGui::DragFloat("Intensity", &dl->intensity, 0.1f, 0.0f, 100.0f))
                scene->markDirty();
        }
    }

    void PropertiesWidget::renderAreaLight()
    {
        const auto         scene  = m_bridge->getCurrentScene();
        const entt::entity entity = m_context->getSelectedEntity();
        auto*              al     = scene->tryGet<AreaLight>(entity);
        if (!al)
            return;

        if (ImGui::CollapsingHeader("Area Light", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::Checkbox("Enabled", &al->enabled))
                scene->markDirty();
            if (ImGui::ColorEdit3("Color", &al->color.x, ImGuiColorEditFlags_Float))
                scene->markDirty();
            if (ImGui::DragFloat("Intensity", &al->intensity, 0.1f, 0.0f, 100.0f))
                scene->markDirty();
            if (ImGui::DragFloat("Width", &al->width, 0.1f, 0.0f, 100.0f))
                scene->markDirty();
            if (ImGui::DragFloat("Height", &al->height, 0.1f, 0.0f, 100.0f))
                scene->markDirty();
        }
    }

} // namespace RealmEngine
