#include "Omni.h"
#include "MacroUtils.h"

constexpr int N = OMNI_CONFIG_LIST(OMNI_WINDOWS, 4);// , OMNI_WINDOWS, 1, OMNI_ANY_PLATFORM, 2);
constexpr int NN = OMNI_BOOL(OMNI_ANDROID);
constexpr int NNN = OMNI_IF_ELSE(0)(2)(3);

