#ifndef SAFESPI_HPP
#define SAFESPI_HPP

#include <Arduino.h>
#include <SPI.h>
#include "FreeRTOS_Base.h"
#include "LGuards.hpp"

namespace fcjj
{
    class SafeSPI
    {
    private:
        SemaphoreHandle_t xMutexHandle = nullptr;
        StaticSemaphore_t xMutexBuffer;
        SPIClass &xSPI;
        uint32_t default_time;

    public:
        SafeSPI() = delete;
        SafeSPI(const SafeSPI &) = delete;
        SafeSPI &operator=(const SafeSPI &) = delete;
        SafeSPI(SafeSPI &&) = delete;
        SafeSPI &operator=(SafeSPI &&) = delete;

        SafeSPI(SPIClass &spi_ref, uint32_t default_time = 100);

        void begin();

        [[nodiscard]] bool transfer(uint8_t cs_pin, const uint8_t *tx_buffer, uint8_t *rx_buffer, size_t len, uint32_t timeout_ms);

        [[nodiscard]] bool transfer(uint8_t cs_pin, const uint8_t *tx_buffer, uint8_t *rx_buffer, size_t len);

        [[nodiscard]] uint8_t transfer_byte(uint8_t cs_pin, uint8_t data, uint32_t timeout_ms);

        [[nodiscard]] uint8_t transfer_byte(uint8_t cs_pin, uint8_t data);

        [[nodiscard]] SemaphoreHandle_t &get_mutex() noexcept;
    };
}
#endif