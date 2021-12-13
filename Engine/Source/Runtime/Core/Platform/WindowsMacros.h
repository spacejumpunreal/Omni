#pragma once
#include "Runtime/Prelude/Omni.h"

#define RELEASE_COM(comPtr) { comPtr->Release(); comPtr = nullptr; }