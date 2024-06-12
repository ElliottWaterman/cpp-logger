//
// Created by balazs on 2020. 12. 04..
//

#ifndef NOWTECH_LOG_QUEUE_FREERTOS
#define NOWTECH_LOG_QUEUE_FREERTOS

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <cstddef>

namespace nowtech::log {

template<typename tMessage, typename tAppInterface, size_t tQueueSize>
class QueueFreeRtos final {
public:
  using tMessage_ = tMessage;
  using tAppInterface_ = tAppInterface;
  using LogTime = typename tAppInterface::LogTime;

  static constexpr size_t csQueueSize = tQueueSize;

private:
  QueueFreeRtos() = delete;

  // static StaticQueue_t xStaticQueue;
  // uint8_t ucQueueStorageArea[tQueueSize * sizeof(tMessage)]; // QUEUE_LENGTH * ITEM_SIZE

  inline static QueueHandle_t sQueue;

public:
  static void init() { // nothing to do
    sQueue = xQueueCreate(tQueueSize, sizeof(tMessage));
    // sQueue = xQueueCreateStatic(tQueueSize, sizeof(tMessage), ucQueueStorageArea, &xStaticQueue);
    // configASSERT(sQueue);
  }

  static void done() {  // nothing to do
  }

  static bool empty() noexcept {
    // TODO[2024.06.07 Elliott]: Determine context (ISR or not)
    return xQueueIsQueueEmptyFromISR(sQueue) == pdTRUE;
  }

  static void push(tMessage const &aMessage) noexcept {
    // TODO[2024.06.07 Elliott]: Determine context (ISR or not)
    xQueueSendFromISR(sQueue, &aMessage, nullptr); // Don't care about the result, pop logic will manage missing messages.
  }

  static bool pop(tMessage &aMessage, LogTime const aPauseLength) noexcept {
    return xQueueReceive(sQueue, &aMessage, aPauseLength) == pdTRUE;
  }
};

}

#endif
