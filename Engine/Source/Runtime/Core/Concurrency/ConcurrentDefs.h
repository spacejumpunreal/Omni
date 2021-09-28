#pragma once
#include "Omni.h"

namespace Omni
{
    enum class QueueKind : u32
    {
#define QUEUE_KIND(x) x,
#include "Concurrency/QueueKind.inl"
#undef QUEUE_KIND
    };
}

