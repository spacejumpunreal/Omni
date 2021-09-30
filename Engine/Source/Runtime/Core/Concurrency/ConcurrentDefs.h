#pragma once
#include "Runtime/Prelude/Omni.h"

namespace Omni
{
    enum class QueueKind : u32
    {
#define QUEUE_KIND(x) x,
#include "Runtime/Core/Concurrency/QueueKind.inl"
#undef QUEUE_KIND
    };
}

