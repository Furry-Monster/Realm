#include "engine.h"
#include "config_manager.h"
#include "global_context.h"
#include "render/render_material.h"
#include "render/render_mesh.h"
#include "render/render_object.h"
#include "render/render_resource.h"
#include "render/render_scene.h"
#include "render/renderer.h"
#include "resource/asset_manager.h"
#include "resource/datatype/model/material.h"
#include "resource/datatype/model/mesh.h"
#include "utils.h"
#include "window.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

namespace RealmEngine
{
    // Create a simple test cube
    void createTestCube()
    {
        // Create cube vertices
        std::vector<Vertex> vertices = {// Front face
                                        {{-0.5f, -0.5f, 0.5f},
                                         {0.0f, 0.0f, 1.0f},
                                         {0.0f, 0.0f},
                                         {1.0f, 0.0f, 0.0f},
                                         {0.0f, 1.0f, 0.0f},
                                         {1.0f, 1.0f, 1.0f, 1.0f}},
                                        {{0.5f, -0.5f, 0.5f},
                                         {0.0f, 0.0f, 1.0f},
                                         {1.0f, 0.0f},
                                         {1.0f, 0.0f, 0.0f},
                                         {0.0f, 1.0f, 0.0f},
                                         {1.0f, 1.0f, 1.0f, 1.0f}},
                                        {{0.5f, 0.5f, 0.5f},
                                         {0.0f, 0.0f, 1.0f},
                                         {1.0f, 1.0f},
                                         {1.0f, 0.0f, 0.0f},
                                         {0.0f, 1.0f, 0.0f},
                                         {1.0f, 1.f, 1.0f, 1.0f}},
                                        {{-0.5f, 0.5f, 0.5f},
                                         {0.0f, 0.0f, 1.0f},
                                         {0.0f, 1.0f},
                                         {1.0f, 0.0f, 0.0f},
                                         {0.0f, 1.0f, 0.0f},
                                         {1.0f, 1.0f, 1.0f, 1.0f}},

                                        // Back face
                                        {{-0.5f, -0.5f, -0.5f},
                                         {0.0f, 0.0f, -1.0f},
                                         {1.0f, 0.0f},
                                         {-1.0f, 0.0f, 0.0f},
                                         {0.0f, 1.0f, 0.0f},
                                         {1.0f, 1.0f, 1.0f, 1.0f}},
                                        {{0.5f, -0.5f, -0.5f},
                                         {0.0f, 0.0f, -1.0f},
                                         {0.0f, 0.0f},
                                         {-1.0f, 0.0f, 0.0f},
                                         {0.0f, 1.0f, 0.0f},
                                         {1.0f, 1.0f, 1.0f, 1.0f}},
                                        {{0.5f, 0.5f, -0.5f},
                                         {0.0f, 0.0f, -1.0f},
                                         {0.0f, 1.0f},
                                         {-1.0f, 0.0f, 0.0f},
                                         {0.0f, 1.0f, 0.0f},
                                         {1.0f, 1.0f, 1.0f, 1.0f}},
                                        {{-0.5f, 0.5f, -0.5f},
                                         {0.0f, 0.0f, -1.0f},
                                         {1.0f, 1.0f},
                                         {-1.0f, 0.0f, 0.0f},
                                         {0.0f, 1.0f, 0.0f},
                                         {1.0f, 1.0f, 1.0f, 1.0f}},

                                        // Left face
                                        {{-0.5f, -0.5f, -0.5f},
                                         {-1.0f, 0.0f, 0.0f},
                                         {0.0f, 0.0f},
                                         {0.0f, 0.0f, 1.0f},
                                         {0.0f, 1.0f, 0.0f},
                                         {1.0f, 1.0f, 1.0f, 1.0f}},
                                        {{-0.5f, -0.5f, 0.5f},
                                         {-1.0f, 0.0f, 0.0f},
                                         {1.0f, 0.0f},
                                         {0.0f, 0.0f, 1.0f},
                                         {0.0f, 1.0f, 0.0f},
                                         {1.0f, 1.0f, 1.0f, 1.0f}},
                                        {{-0.5f, 0.5f, 0.5f},
                                         {-1.0f, 0.0f, 0.0f},
                                         {1.0f, 1.0f},
                                         {0.0f, 0.0f, 1.0f},
                                         {0.0f, 1.0f, 0.0f},
                                         {1.0f, 1.0f, 1.0f, 1.0f}},
                                        {{-0.5f, 0.5f, -0.5f},
                                         {-1.0f, 0.0f, 0.0f},
                                         {0.0f, 1.0f},
                                         {0.0f, 0.0f, 1.0f},
                                         {0.0f, 1.0f, 0.0f},
                                         {1.0f, 1.0f, 1.0f, 1.0f}},

                                        // Right face
                                        {{0.5f, -0.5f, -0.5f},
                                         {1.0f, 0.0f, 0.0f},
                                         {1.0f, 0.0f},
                                         {0.0f, 0.0f, -1.0f},
                                         {0.0f, 1.0f, 0.0f},
                                         {1.0f, 1.0f, 1.0f, 1.0f}},
                                        {{0.5f, -0.5f, 0.5f},
                                         {1.0f, 0.0f, 0.0f},
                                         {0.0f, 0.0f},
                                         {0.0f, 0.0f, -1.0f},
                                         {0.0f, 1.0f, 0.0f},
                                         {1.0f, 1.0f, 1.0f, 1.0f}},
                                        {{0.5f, 0.5f, 0.5f},
                                         {1.0f, 0.0f, 0.0f},
                                         {0.0f, 1.0f},
                                         {0.0f, 0.0f, -1.0f},
                                         {0.0f, 1.0f, 0.0f},
                                         {1.0f, 1.0f, 1.0f, 1.0f}},
                                        {{0.5f, 0.5f, -0.5f},
                                         {1.0f, 0.0f, 0.0f},
                                         {1.0f, 1.0f},
                                         {0.0f, 0.0f, -1.0f},
                                         {0.0f, 1.0f, 0.0f},
                                         {1.0f, 1.0f, 1.0f, 1.0f}},

                                        // Top face
                                        {{-0.5f, 0.5f, -0.5f},
                                         {0.0f, 1.0f, 0.0f},
                                         {0.0f, 0.0f},
                                         {1.0f, 0.0f, 0.0f},
                                         {0.0f, 0.0f, -1.0f},
                                         {1.0f, 1.0f, 1.0f, 1.0f}},
                                        {{0.5f, 0.5f, -0.5f},
                                         {0.0f, 1.0f, 0.0f},
                                         {1.0f, 0.0f},
                                         {1.0f, 0.0f, 0.0f},
                                         {0.0f, 0.0f, -1.0f},
                                         {1.0f, 1.0f, 1.0f, 1.0f}},
                                        {{0.5f, 0.5f, 0.5f},
                                         {0.0f, 1.0f, 0.0f},
                                         {1.0f, 1.0f},
                                         {1.0f, 0.0f, 0.0f},
                                         {0.0f, 0.0f, -1.0f},
                                         {1.0f, 1.0f, 1.0f, 1.0f}},
                                        {{-0.5f, 0.5f, 0.5f},
                                         {0.0f, 1.0f, 0.0f},
                                         {0.0f, 1.0f},
                                         {1.0f, 0.0f, 0.0f},
                                         {0.0f, 0.0f, -1.0f},
                                         {1.0f, 1.0f, 1.0f, 1.0f}},

                                        // Bottom face
                                        {{-0.5f, -0.5f, -0.5f},
                                         {0.0f, -1.0f, 0.0f},
                                         {1.0f, 0.0f},
                                         {1.0f, 0.0f, 0.0f},
                                         {0.0f, 0.0f, 1.0f},
                                         {1.0f, 1.0f, 1.0f, 1.0f}},
                                        {{0.5f, -0.5f, -0.5f},
                                         {0.0f, -1.0f, 0.0f},
                                         {0.0f, 0.0f},
                                         {1.0f, 0.0f, 0.0f},
                                         {0.0f, 0.0f, 1.0f},
                                         {1.0f, 1.0f, 1.0f, 1.0f}},
                                        {{0.5f, -0.5f, 0.5f},
                                         {0.0f, -1.0f, 0.0f},
                                         {0.0f, 1.0f},
                                         {1.0f, 0.0f, 0.0f},
                                         {0.0f, 0.0f, 1.0f},
                                         {1.0f, 1.0f, 1.0f, 1.0f}},
                                        {{-0.5f, -0.5f, 0.5f},
                                         {0.0f, -1.0f, 0.0f},
                                         {1.0f, 1.0f},
                                         {1.0f, 0.0f, 0.0f},
                                         {0.0f, 0.0f, 1.0f},
                                         {1.0f, 1.0f, 1.0f, 1.0f}}};

        // Create cube indices
        std::vector<uint32_t> indices = {// Front face
                                         0,
                                         1,
                                         2,
                                         2,
                                         3,
                                         0,
                                         // Back face
                                         4,
                                         5,
                                         6,
                                         6,
                                         7,
                                         4,
                                         // Left face
                                         8,
                                         9,
                                         10,
                                         10,
                                         11,
                                         8,
                                         // Right face
                                         12,
                                         13,
                                         14,
                                         14,
                                         15,
                                         12,
                                         // Top face
                                         16,
                                         17,
                                         18,
                                         18,
                                         19,
                                         16,
                                         // Bottom face
                                         20,
                                         21,
                                         22,
                                         22,
                                         23,
                                         20};

        // Create mesh
        Mesh cube_mesh;
        cube_mesh.setVertices(std::move(vertices));
        cube_mesh.setIndices(std::move(indices));

        // Create material with metallic properties
        Material cube_material;
        cube_material.setBaseColorFactor(glm::vec4(0.7f, 0.1f, 0.1f, 1.0f)); // Darker red
        cube_material.setMetallicFactor(0.8f);                               // More metallic
        cube_material.setRoughnessFactor(0.3f);                              // Less rough (more shiny)

        // Create render mesh and material
        RenderMesh render_mesh;
        render_mesh.sync(*g_context.m_renderer->getRHI(), cube_mesh);

        RenderMaterial render_material;
        render_material.sync(*g_context.m_renderer->getRHI(), cube_material, g_context.m_renderer->getPBRProgram());

        // Add to render resource
        uint32_t mesh_index = g_context.m_renderer->getRenderResource()->addRenderMesh(std::move(render_mesh));
        uint32_t material_index =
            g_context.m_renderer->getRenderResource()->addRenderMaterial(std::move(render_material));

        // Create render object
        RenderObject cube_obj;
        cube_obj.setMesh(mesh_index);
        cube_obj.setMaterial(material_index);

        // Move cube back a bit to ensure it's in front of camera
        glm::mat4 model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -1.0f));
        cube_obj.setModelMatrix(model_matrix);
        cube_obj.setVisible(true);

        // Add to render scene
        g_context.m_renderer->getRenderScene()->addRenderObject(std::move(cube_obj));

        // Add directional light with higher intensity
        DirectionalLight dir_light;
        dir_light.direction = glm::normalize(glm::vec3(1.0f, -1.0f, 1.0f));
        dir_light.color     = glm::vec3(1.0f, 1.0f, 1.0f);
        dir_light.intensity = 2.5f; // Brighter directional light
        g_context.m_renderer->getRenderScene()->addDirectionalLight(dir_light);

        // Add ambient light with higher intensity
        AmbientLight ambient_light;
        ambient_light.color     = glm::vec3(0.2f, 0.2f, 0.2f);
        ambient_light.intensity = 0.6f; // Brighter ambient light
        g_context.m_renderer->getRenderScene()->setAmbientLight(ambient_light);

        // Set camera position to look at the cube
        g_context.m_renderer->getRenderCamera()->setPosition(glm::vec3(0.0f, 0.0f, 3.0f));
        g_context.m_renderer->getRenderCamera()->setRotation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f)); // Identity quaternion

        // Set camera perspective projection
        g_context.m_renderer->getRenderCamera()->setPerspective(45.0f, 16.0f / 9.0f, 0.1f, 100.0f);

        info("Test cube created successfully");
    }

    void Engine::boot()
    {
        g_context.create();

        // Create test scene
        createTestCube();

        info("<<< Boot Engine Done. >>>");
    }

    void Engine::run()
    {
        info("Starting render loop...");
        int frame_count = 0;

        // Check window properties
        info("Window should be visible now. Press any key to continue...");

        while (!g_context.m_window->shouldClose() && frame_count < 600) // Run for 600 frames (about 10 seconds)
        {
            tick();
            frame_count++;

            if (frame_count % 60 == 0) // Log every second
            {
                info("Rendered " + std::to_string(frame_count) + " frames");
            }
        }

        info("Render loop completed. Total frames: " + std::to_string(frame_count));
    }

    void Engine::terminate()
    {
        info("<<< Now Teminating Engine. >>>");
        g_context.destroy();
    }

    void Engine::tick()
    {
        logicalTick();
        renderTick();
    }

    void Engine::logicalTick() { g_context.m_window->pollEvents(); }

    void Engine::renderTick()
    {
        g_context.m_renderer->renderFrame();
        g_context.m_window->swapBuffer();
    }

} // namespace RealmEngine
