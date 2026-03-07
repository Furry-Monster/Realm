#include "registration.h"
#include "engine.h"
#include "functional/resource/config_manager.h"
#include "platform/input/input.h"

#include "core/debug/debug_console.h"
#include "core/sched/scheduler.h"
#include "functional/camera/camera_sync.h"
#include "functional/ecs/components/transform.h"
#include "functional/ecs/components/world_transform.h"
#include "functional/ecs/systems/hierarchy_system.h"
#include "functional/ecs/systems/transform_system.h"
#include "functional/render/components/lighting/light_probe.h"
#include "functional/render/light_probe_baker.h"
#include "functional/render/render_scene_sync.h"
#include "functional/render/renderer.h"
#include "functional/render/viewport_controller.h"
#include "functional/render/viewport_display_mode.h"
#include "functional/scene/scene_manager.h"
#include "platform/info/platform_info.h"
#include "platform/window/window.h"

#if REALM_BUILD_AUDIO
#  include "module/audio/audio_listener_resolve.h"
#  include "module/audio/audio_system.h"
#endif

namespace RealmEngine
{
    void registerSystems(Scheduler& scheduler, Engine& engine)
    {
        scheduler.registerSystem(
            SystemPhase::PreLogic, 0, "InputTick", [&engine](SystemContext&) { engine.getInput().tick(); });
        scheduler.registerSystem(
            SystemPhase::PreLogic, 1, "PollEvents", [&engine](SystemContext&) { engine.getWindow().pollEvents(); });
        scheduler.registerSystem(SystemPhase::PreLogic, 2, "SceneSetup", [&engine](SystemContext& ctx) {
            if (!ctx.scene)
                return;
            if (ctx.scene->getViewportController())
                return;
            auto                  ctrl = std::make_shared<ViewportController>();
            const ViewportConfig& v    = engine.getConfig().getViewportConfig();
            ctrl->initialize(engine.getRenderer().getCamera(),
                             engine.getInput(),
                             v.camera_mouse_sensitivity,
                             v.camera_move_speed,
                             v.camera_sprint_multiplier);
            ctx.scene->setViewportController(std::move(ctrl));
        });

        scheduler.registerSystem(SystemPhase::Logic, "HierarchySystem", [](SystemContext& ctx) {
            if (ctx.scene)
                HierarchySystem::update(*ctx.scene);
        });

        scheduler.registerSystem(SystemPhase::Logic, "TransformSystem", [](SystemContext& ctx) {
            if (ctx.scene)
                TransformSystem::update(*ctx.scene);
        });

        scheduler.registerSystem(SystemPhase::PostLogic, "ViewportController", [&engine](SystemContext& ctx) {
            if (engine.getViewportMode() != ViewportMode::Scene || !ctx.scene)
                return;
            if (auto* ctrl = ctx.scene->getViewportController().get())
                ctrl->update(ctx.delta_time);
        });

#if REALM_BUILD_AUDIO
        scheduler.registerSystem(SystemPhase::PostLogic, "AudioTick", [&engine](SystemContext& ctx) {
            if (auto* audio = engine.getModuleManager().getAudioSystem(); audio && ctx.scene)
                audio->tick(ctx.scene, ctx.delta_time, engine.getConfig().getAssetFolder().string());
        });

        scheduler.registerSystem(SystemPhase::PostLogic, "AudioListener", [&engine](SystemContext& ctx) {
            auto* audio = engine.getModuleManager().getAudioSystem();
            if (!audio || !ctx.scene)
                return;
            const entt::entity listener_entity = findPrimaryAudioListenerEntity(*ctx.scene);
            if (listener_entity != entt::null)
            {
                const ListenerPose pose = getListenerPoseFromEntity(*ctx.scene, listener_entity);
                audio->setListener(pose.position, pose.forward, pose.up);
            }
            else if (const auto* cam = engine.getRenderer().getCamera().get())
            {
                audio->setListener(cam->getPosition(), cam->getLocalForward(), cam->getLocalUp());
            }
        });
#endif

        scheduler.registerSystem(SystemPhase::PreRender, "RenderSceneSync", [&engine](SystemContext& ctx) {
            auto scene = engine.getSceneManager().getCurrentScene();
            syncFromScene(scene, *engine.getRenderer().getRenderScene());
            engine.getRenderer().setCurrentEcsScene(ctx.scene);
        });

        scheduler.registerSystem(SystemPhase::PreRender, "CameraSync", [&engine](SystemContext& ctx) {
            if (engine.getViewportMode() != ViewportMode::Game || !ctx.scene)
                return;
            const entt::entity cam_entity = findPrimaryCameraEntity(*ctx.scene);
            if (cam_entity != entt::null)
            {
                const float aspect = static_cast<float>(engine.getWindow().getWidth()) /
                                     static_cast<float>(engine.getWindow().getHeight());
                syncEntityCameraToRenderCamera(*ctx.scene, cam_entity, *engine.getRenderer().getCamera(), aspect);
            }
        });

        scheduler.registerSystem(SystemPhase::PreRender, "LightProbeBake", [&engine](SystemContext& ctx) {
            if (!ctx.scene || !engine.getRenderer().getLightProbeBaker())
                return;
            auto&      registry = ctx.scene->getRegistry();
            const auto view     = registry.view<LightProbe>();

            for (const auto entity : view)
            {
                auto& lp = view.get<LightProbe>(entity);
                if (!lp.needs_update)
                    continue;

                glm::vec3 pos {0.0f};
                if (auto* wt = ctx.scene->tryGet<WorldTransform>(entity))
                    pos = glm::vec3(wt->matrix[3]);
                else if (const auto* t = ctx.scene->tryGet<Transform>(entity))
                    pos = t->position;

                const auto [sh_coefficients, success] =
                    engine.getRenderer().getLightProbeBaker()->bake(pos, *engine.getRenderer().getRenderScene());
                if (success)
                {
                    lp.sh_coefficients = sh_coefficients;
                    lp.needs_update    = false;
                }
            }
        });

        scheduler.registerSystem(
            SystemPhase::Render, "Renderer", [&engine](SystemContext&) { engine.getRenderer().render(); });

        scheduler.registerSystem(SystemPhase::PostRender, "FrameStats", [&engine](SystemContext& ctx) {
            FrameStats stats {};
            stats.frame_time_ms             = ctx.delta_time * 1000.0;
            stats.fps                       = (ctx.delta_time > 1e-9f) ? (1.0 / ctx.delta_time) : 0.0;
            stats.draw_calls                = engine.getRenderer().getRenderScene()->getDrawCallCount();
            stats.triangle_count            = engine.getRenderer().getRenderScene()->getTriangleCount();
            static double s_rss_accumulator = 0.0;
            static size_t s_cached_rss      = 0;
            s_rss_accumulator += ctx.delta_time;
            if (s_rss_accumulator >= 1.0)
            {
                s_cached_rss      = PlatformInfo::getProcessRSSKB();
                s_rss_accumulator = 0.0;
            }
            stats.memory_rss_kb = s_cached_rss;
            EditorConsole::instance().setFrameStats(stats);
        });
    }
} // namespace RealmEngine
