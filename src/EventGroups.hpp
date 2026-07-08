#ifndef EVENT_GROUPS_HPP
#define EVENT_GROUPS_HPP

#include "FreeRTOS_Base.h"
#include RTOS_INC("freertos/event_groups.h", <event_groups.h>, <event_groups.h>)

#if defined(__AVR_ATmega328P__) || defined(ARDUINO_AVR_UNO)

#ifdef xEventGroupSetBitsFromISR
#undef xEventGroupSetBitsFromISR
#endif
#define xEventGroupSetBitsFromISR(xEG, uxBits, pxWoken) (pdFAIL)

#ifdef xEventGroupClearBitsFromISR
#undef xEventGroupClearBitsFromISR
#endif
#define xEventGroupClearBitsFromISR(xEG, uxBits) (pdFAIL)

#ifndef xEventGroupCreateStatic
#define xEventGroupCreateStatic(pxEventGroupBuffer) \
  xEventGroupCreate()
#endif
#endif

class EventGroup
{
private:
  EventGroupHandle_t xHandle = nullptr;
  StaticEventGroup_t xBuffer;

public:
  EventGroup();

  EventGroup(const EventGroup &) = delete;
  EventGroup &operator=(const EventGroup &) = delete;
  EventGroup(EventGroup &&) = delete;
  EventGroup &operator=(EventGroup &&) = delete;

  ~EventGroup();

  [[nodiscard]] EventBits_t set(EventBits_t bits);

  [[nodiscard]] bool set_from_isr(EventBits_t bits, BaseType_t *pxHigherPriorityTaskWoken = nullptr);

  [[nodiscard]] EventBits_t clear(EventBits_t bits);

  [[nodiscard]] EventBits_t clear_from_isr(EventBits_t bits);

  [[nodiscard]] EventBits_t get() const;

  [[nodiscard]] EventBits_t get_from_isr() const;

  [[nodiscard]] EventBits_t wait_any(EventBits_t bits_to_wait, bool clear_on_exit = true, uint32_t timeout_ms = 0xFFFFFFFF);

  [[nodiscard]] EventBits_t wait_all(EventBits_t bits_to_wait, bool clear_on_exit = true, uint32_t timeout_ms = 0xFFFFFFFF);

  [[nodiscard]] EventBits_t sync(EventBits_t bits_to_set, EventBits_t bits_to_wait, uint32_t timeout_ms = 0xFFFFFFFF);

  [[nodiscard]] bool all_set(EventBits_t bits) const;

  [[nodiscard]] bool any_set(EventBits_t bits) const;

  [[nodiscard]] EventGroupHandle_t get_handle() const noexcept;
};

#endif