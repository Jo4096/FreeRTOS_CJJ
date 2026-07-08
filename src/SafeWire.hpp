#ifndef SAFEWIRE_HPP
#define SAFEWIRE_HPP

#include <Arduino.h>
#include <Wire.h>
#include "FreeRTOS_Base.h"
#include "LGuards.hpp"

class SafeWire
{
private:
    SemaphoreHandle_t xMutexHandle = nullptr;
    StaticSemaphore_t xMutexBuffer;
    TwoWire &xWire;
    uint32_t default_time;

public:
    SafeWire() = delete;
    SafeWire(const SafeWire &) = delete;
    SafeWire &operator=(const SafeWire &) = delete;
    SafeWire(SafeWire &&) = delete;
    SafeWire &operator=(SafeWire &&) = delete;

    SafeWire(TwoWire &wire_ref, uint32_t default_time = 100);

    void begin();

    void begin(int master_address);

    [[nodiscard]] bool write_register(uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, size_t len, uint32_t timeout_ms);

    [[nodiscard]] bool write_register(uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, size_t len);

    [[nodiscard]] bool read_register(uint8_t dev_addr, uint8_t reg_addr, uint8_t *output_buffer, uint8_t quantity, uint32_t timeout_ms);

    [[nodiscard]] bool read_register(uint8_t dev_addr, uint8_t reg_addr, uint8_t *output_buffer, uint8_t quantity);

    [[nodiscard]] SemaphoreHandle_t &get_mutex() noexcept;
};

#endif