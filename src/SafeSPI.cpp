#include "SafeSPI.hpp"

SafeSPI::SafeSPI(SPIClass &spi_ref, uint32_t default_time) : xSPI(spi_ref), default_time(default_time)
{
    xMutexHandle = xSemaphoreCreateMutexStatic(&xMutexBuffer);
}

void SafeSPI::begin()
{
    xSPI.begin();
}

bool SafeSPI::transfer(uint8_t cs_pin, const uint8_t *tx_buffer, uint8_t *rx_buffer, size_t len, uint32_t timeout_ms)
{
    if (xMutexHandle == nullptr)
        return false;
    UniqueLock lock(xMutexHandle, timeout_ms);
    if (lock.owns_lock())
    {
        digitalWrite(cs_pin, LOW);

        for (size_t i = 0; i < len; i++)
        {
            uint8_t received = xSPI.transfer(tx_buffer ? tx_buffer[i] : 0x00);
            if (rx_buffer)
            {
                rx_buffer[i] = received;
            }
        }

        digitalWrite(cs_pin, HIGH);
        return true;
    }
    return false;
}

bool SafeSPI::transfer(uint8_t cs_pin, const uint8_t *tx_buffer, uint8_t *rx_buffer, size_t len)
{
    return transfer(cs_pin, tx_buffer, rx_buffer, len, default_time);
}

uint8_t SafeSPI::transfer_byte(uint8_t cs_pin, uint8_t data, uint32_t timeout_ms)
{
    uint8_t rx_data = 0;
    if (transfer(cs_pin, &data, &rx_data, 1, timeout_ms))
    {
        return rx_data;
    }
    return 0;
}

uint8_t SafeSPI::transfer_byte(uint8_t cs_pin, uint8_t data)
{
    return transfer_byte(cs_pin, data, default_time);
}

SemaphoreHandle_t &SafeSPI::get_mutex() noexcept
{
    return xMutexHandle;
}
