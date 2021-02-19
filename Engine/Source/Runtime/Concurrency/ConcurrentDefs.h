#pragma once
#include "Runtime/Omni.h"

namespace Omni
{
    enum class QueueKind : u32
    {
#define QUEUE_KIND(x) x,
#include "Runtime/Concurrency/QueueKind.inl"
#undef QUEUE_KIND
    };
}

