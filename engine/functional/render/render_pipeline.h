#pragma once

#include <memory>
#include <string>
#include <vector>

#include "functional/render/render_pass.h"

namespace RealmEngine
{
    class RHIDevice;

    // Owns an ordered list of RenderPass objects and executes them each frame.
    class RenderPipeline
    {
    public:
        RenderPipeline() = default;
        ~RenderPipeline() noexcept;

        RenderPipeline(const RenderPipeline&)            = delete;
        RenderPipeline& operator=(const RenderPipeline&) = delete;

        void addPass(std::unique_ptr<RenderPass> pass);

        template<typename T>
        T* getPass(const std::string& name) const;

        void initialize(RHIDevice& device);
        void execute(const RenderContext& ctx);
        void dispose();

    private:
        std::vector<std::unique_ptr<RenderPass>> m_passes;
    };

    // Template implementation
    template<typename T>
    T* RenderPipeline::getPass(const std::string& name) const
    {
        for (const auto& pass : m_passes)
        {
            if (pass->getName() == name)
                return dynamic_cast<T*>(pass.get());
        }
        return nullptr;
    }

} // namespace RealmEngine
