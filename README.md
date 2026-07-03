# FreeRTOS_CJJ

**FreeRTOS_CJJ** is a modern, lightweight C++11/14 object-oriented wrapper library designed for FreeRTOS. It abstracts complex FreeRTOS C-APIs into clean, intuitive, and type-safe C++ classes, significantly reducing boilerplate code while making multitasking embedded development robust and highly maintainable.

The library is built heavily around **RAII (Resource Acquisition Is Initialization)** principles to completely eliminate common concurrency bugs, such as unreleased semaphores or mutexes.

While optimized out-of-the-box for high-performance 32-bit dual-core architectures (like the **ESP32** and **Raspberry Pi Pico (RP2040)**) by leveraging deterministic static memory allocations, it gracefully falls back to dynamic allocations on memory-constrained 8-bit platforms like the **Arduino Uno (ATmega328P)**.

---

## Key Architectural Benefits

### Zero-Overhead Abstraction
Enjoy modern C++ idioms, such as templates, inheritance, and encapsulation, without sacrificing the core real-time determinism or execution speed of vanilla FreeRTOS.

### RAII-Driven Safety
* **Lock Management:** Smart resource guards (`LockGuard`, `UniqueLock`, `CriticalGuard`) automatically release mutexes, critical sections, or ISR-safe locks when they go out of scope.
* **Task Lifecycles:** When a one-shot thread finishes execution, it automatically cleans up its own task handle and releases its resources from the scheduler safely.

### No More `void*` Parameter Mess
In vanilla FreeRTOS, passing parameters to tasks requires messy `void*` casting. **FreeRTOS_CJJ** takes a pure object-oriented approach.
* Tasks are structured as classes derived from `ThreadABS`.
* Because tasks are true C++ objects, thread-specific parameters can be safely passed via the class constructor and kept secure within `private` or `protected` scopes.
* All internal data is cleanly accessible from inside the overridden `Run()` execution context.

The library provides two primary thread paradigms out of the box:
* **`ThreadRTOS`:** Best for standard FreeRTOS workflows where you implement your own infinite `for(;;)` loop. If left to return naturally, it acts as a self-deleting one-shot task.
* **`ThreadArduino`:** A beginner-friendly approach featuring native `setup()` and `loop()` methods to mimic the familiar Arduino execution lifecycle.

### Compile-Time Type Safety
Vanilla FreeRTOS queues and buffers lack data type awareness, opening the door to memory mismatches. **FreeRTOS_CJJ** utilizes compile-time templates for IPC elements, ensuring you can never accidentally push an incompatible data type into a specialized stream or message buffer.

### Deterministic Memory Management
By default, the library enforces predictable execution footprints on supported 32-bit platforms by utilizing static API variants (`xTaskCreateStatic`, `xSemaphoreCreateMutexStatic`, etc.). This guarantees compile-time memory guarantees and eliminates runtime heap fragmentation.

---

## Feature-Rich Component Ecosystem

Beyond tasks and locks, **FreeRTOS_CJJ** provides object-oriented wrappers for the entire FreeRTOS core feature set:

* **Synchronization & Concurrency:** Clean abstractions for `Mutex`, `RecursiveMutex`, `BinarySemaphore`, and `CountingSemaphore`.
* **Event Groups:** Easily broadcast state changes or synchronize multiple threads via `EventGroup` bitmasks.
* **Timers:** Implement native asynchronous software execution using template-configured `PeriodicTimer` or `OneShotTimer` modules.
* **Data Streams:** Fast, overhead-free data passing using object-wrapped `StreamBuffer` and `MessageBuffer` implementations.
* **Hardware Communication Security:** Embedded thread-safe wrappers (`SafeWire`, `SafeSPI`, `SafeSerial`) to guarantee hazard-free shared-bus access across concurrent tasks.
