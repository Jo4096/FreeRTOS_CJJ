#include "SafeWire.hpp"

namespace fcjj
{
    SafeWire::SafeWire(TwoWire &wire_ref, uint32_t default_time) : xWire(wire_ref), default_time(default_time)
    {
        xMutexHandle = xSemaphoreCreateMutexStatic(&xMutexBuffer);
    }

    void SafeWire::begin()
    {
        xWire.begin();
    }

    void SafeWire::begin(int master_address)
    {
        xWire.begin(master_address);
    }

    bool SafeWire::write_register(uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, size_t len, uint32_t timeout_ms)
    {
        if (xMutexHandle == nullptr)
            return false;

        UniqueLock lock(xMutexHandle, timeout_ms);
        if (lock.owns_lock())
        {
            xWire.beginTransmission(dev_addr);
            xWire.write(reg_addr);
            for (size_t i = 0; i < len; i++)
            {
                xWire.write(data[i]);
            }
            return (xWire.endTransmission() == 0);
        }

        return false;
    }

    bool SafeWire::write_register(uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, size_t len)
    {
        return write_register(dev_addr, reg_addr, data, len, default_time);
    }

    bool SafeWire::read_register(uint8_t dev_addr, uint8_t reg_addr, uint8_t *output_buffer, uint8_t quantity, uint32_t timeout_ms)
    {
        if (xMutexHandle == nullptr)
            return false;

        UniqueLock lock(xMutexHandle, timeout_ms);
        if (lock.owns_lock())
        {
            xWire.beginTransmission(dev_addr);
            xWire.write(reg_addr);
            if (xWire.endTransmission(false) != 0)
                return false;

            uint8_t bytes_received = xWire.requestFrom(dev_addr, quantity);
            if (bytes_received != quantity)
                return false;

            for (uint8_t i = 0; i < quantity; i++)
            {
                if (xWire.available())
                {
                    output_buffer[i] = xWire.read();
                }
            }
            return true;
        }
        return false;
    }

    bool SafeWire::read_register(uint8_t dev_addr, uint8_t reg_addr, uint8_t *output_buffer, uint8_t quantity)
    {
        return read_register(dev_addr, reg_addr, output_buffer, quantity, default_time);
    }

    SemaphoreHandle_t &SafeWire::get_mutex() noexcept
    {
        return xMutexHandle;
    }
}