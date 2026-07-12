#ifndef FREE_RTOS_BASE_H
#define FREE_RTOS_BASE_H

#if defined(ESP32)
#define RTOS_INC(esp32_h, rp2040_h, avr_h) esp32_h
#elif defined(ARDUINO_ARCH_RP2040) || defined(TARGET_RP2040)
#define RTOS_INC(esp32_h, rp2040_h, avr_h) rp2040_h
#elif defined(__AVR_ATmega328P__) || defined(ARDUINO_AVR_UNO)
#define RTOS_INC(esp32_h, rp2040_h, avr_h) avr_h
#else
#error "Unsupported platform."
#endif

#include RTOS_INC("freertos/FreeRTOS.h", <FreeRTOS.h>, <Arduino_FreeRTOS.h>)

#if defined(ESP32)
using CoreAffinity = BaseType_t;
constexpr CoreAffinity CORE_0 = 0;
constexpr CoreAffinity CORE_1 = 1;
constexpr CoreAffinity CORE_ANY = tskNO_AFFINITY;

#elif defined(ARDUINO_ARCH_RP2040) || defined(TARGET_RP2040)
using CoreAffinity = UBaseType_t;
constexpr CoreAffinity CORE_0 = (1u << 0);
constexpr CoreAffinity CORE_1 = (1u << 1);
constexpr CoreAffinity CORE_ANY = 0; // resolvido depois em runtime (todos os cores)

#else // AVR — single core, os valores existem só para compilar, mas são ignorados
using CoreAffinity = uint8_t;
constexpr CoreAffinity CORE_0 = 0;
constexpr CoreAffinity CORE_1 = 0;
constexpr CoreAffinity CORE_ANY = 0;
#endif

#if defined(ESP32)
#include <Arduino.h>
#include <utility>
namespace custom_std
{
    using std::forward;
}

extern portMUX_TYPE esp32_kernel_mux;

#ifdef taskENTER_CRITICAL
#undef taskENTER_CRITICAL
#define taskENTER_CRITICAL() portENTER_CRITICAL(&esp32_kernel_mux)
#endif

#ifdef taskEXIT_CRITICAL
#undef taskEXIT_CRITICAL
#define taskEXIT_CRITICAL() portEXIT_CRITICAL(&esp32_kernel_mux)
#endif

#elif defined(ARDUINO_ARCH_RP2040) || defined(TARGET_RP2040)
#include <utility>
namespace custom_std
{
    using std::forward;
}

#else

#if defined(__AVR_ATmega328P__) || defined(ARDUINO_AVR_UNO)

#ifdef portYIELD_FROM_ISR
#undef portYIELD_FROM_ISR
#endif
#define portYIELD_FROM_ISR(x) portYIELD()

namespace custom_std
{
    template <typename T>
    struct remove_reference
    {
        typedef T type;
    };
    template <typename T>
    struct remove_reference<T &>
    {
        typedef T type;
    };
    template <typename T>
    struct remove_reference<T &&>
    {
        typedef T type;
    };

    template <typename T>
    constexpr T &&forward(typename remove_reference<T>::type &t) noexcept
    {
        return static_cast<T &&>(t);
    }
    template <typename T>
    constexpr T &&forward(typename remove_reference<T>::type &&t) noexcept
    {
        return static_cast<T &&>(t);
    }
}

#warning "Static instances aren't supported by Arduino_FreeRTOS.h, so every call is an alias to the regular counterpart (with memory allocation). Keep in mind that this doesn't follow the JSF++ standard"

#else
#error "Unsupported platform. FreeRTOS_Base.h supports ESP32 and RP2040 only."
#endif
#endif

#endif