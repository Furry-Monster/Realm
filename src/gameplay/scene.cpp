#include "gameplay/scene.h"

#include <memory>
#include "gameplay/camera_controller.h"
#include "render/render_camera.h"

namespace RealmEngine
{
    void Scene::initialize(std::shared_ptr<RenderCamera> camera)
    {
        m_camera_controller = std::make_shared<CameraController>();
        m_camera_controller->initialize(camera);
    }

    void Scene::tick(float delta_time)
    {
        if (m_camera_controller)
            m_camera_controller->update(delta_time);
    }
} // namespace RealmEngine
