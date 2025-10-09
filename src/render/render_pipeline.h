#pragma once

namespace RealmEngine
{
    class Pipeline
    {
    public:
        virtual ~Pipeline() noexcept = default;

        virtual void initialize() = 0;
        virtual void render()     = 0;
        virtual void disposal()   = 0;
    };

    class ForwardPipeline : public Pipeline
    {
    public:
        void initialize() override;
        void disposal() override;

        void render() override
        {
            renderShadowMaps();
            renderForward();
            renderPostprocess();
        }

    protected:
        void renderShadowMaps();
        void renderForward();
        void renderPostprocess();
    };
} // namespace RealmEngine