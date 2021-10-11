#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Engine/EngineAPI.h"
#include "Runtime/Core/System/Module.h"

namespace Omni
{
    class DemoRendererModule : public Module
    {
    public:
        void Destroy() override;
        void Initialize(const EngineInitArgMap&) override;
        void Finalize() override;
    };
}