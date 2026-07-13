#include "FreeRTOS_Base.h"

// esp32_kernel_mux is only DECLARED (extern) in FreeRTOS_Base.h.
// It must be DEFINED exactly once, in a .cpp file, or the linker
// will fail with "undefined reference to esp32_kernel_mux" as soon
// as anything calls Kernel::enter_critical() / taskENTER_CRITICAL()
// on ESP32 (e.g. SafeRingBuffer, or any Kernel::CriticalGuard usage).
// I forgot about it...
#if defined(ESP32)
portMUX_TYPE esp32_kernel_mux = portMUX_INITIALIZER_UNLOCKED;
#endif