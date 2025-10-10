#pragma once

namespace RealmEngine
{
    class RenderPipeline
    {
    public:
        virtual ~RenderPipeline() noexcept = default;

        virtual void initialize() = 0;
        virtual void render()     = 0;
        virtual void disposal()   = 0;
    };

    class ForwardPipeline : public RenderPipeline
    {
    public:
        ForwardPipeline()           = default;
        ~ForwardPipeline() noexcept = default;

        ForwardPipeline(const ForwardPipeline& that)            = delete;
        ForwardPipeline(ForwardPipeline&& that)                 = delete;
        ForwardPipeline& operator=(const ForwardPipeline& that) = delete;
        ForwardPipeline& operator=(ForwardPipeline&& that)      = delete;

        void initialize() override;
        void disposal() override;

        void render() override;

    protected:
        void renderShadowMaps();
        void renderForward();
        void renderPostprocess();
    };

    class DefferedPipeline : public RenderPipeline
    {
    public:
        DefferedPipeline()           = default;
        ~DefferedPipeline() noexcept = default;

        DefferedPipeline(const DefferedPipeline& that)            = delete;
        DefferedPipeline(DefferedPipeline&& that)                 = delete;
        DefferedPipeline& operator=(const DefferedPipeline& that) = delete;
        DefferedPipeline& operator=(DefferedPipeline&& that)      = delete;

        void initialize() override;
        void disposal() override;

        void render() override;

    protected:
    };
} // namespace RealmEngine