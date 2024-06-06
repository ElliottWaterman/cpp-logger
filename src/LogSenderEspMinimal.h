#pragma once

#ifndef NOWTECH_LOG_SENDER_ESP_MINIMAL
#define NOWTECH_LOG_SENDER_ESP_MINIMAL

#include "Log.h"

#include <esp_log.h>

#include <cstdio>

static const char* TAG = "LOG";

namespace nowtech::log {

// For slow transmission medium like UART on embedded this class could implement double buffering.
template<typename tAppInterface, typename tConverter, size_t tTransmitBufferSize, typename tAppInterface::LogTime tTimeout>
class SenderEspMinimal final {
public:
    using tAppInterface_   = tAppInterface;
    using tConverter_      = tConverter;
    using ConversionResult = typename tConverter::ConversionResult;
    using Iterator         = typename tConverter::Iterator;

    static constexpr bool csVoid = false;

private:
    // inline static UART_HandleTypeDef *sSerialDescriptor = nullptr;
    inline static ConversionResult   *sTransmitBuffer;
    inline static Iterator            sBegin;
    inline static Iterator            sEnd;

    SenderEspMinimal() = delete;

public:
    // static void init(UART_HandleTypeDef * const aUart) {
        // sSerialDescriptor = aUart;
    static void init() {
        sTransmitBuffer = tAppInterface::template _newArray<ConversionResult>(tTransmitBufferSize);
        sBegin = sTransmitBuffer;
        sEnd = sTransmitBuffer + tTransmitBufferSize;
    }

    static void done() noexcept {
    }

    static void send(char const * const aBegin, char const * const aEnd) {
        // printf
        // esp_log_write(ESP_LOG_INFO, "SEND", aBegin, aEnd - aBegin);
        ESP_LOGI(TAG, "%.*s", aBegin, aEnd - aBegin);

        // if (sSerialDescriptor != nullptr) {
        //     HAL_UART_Transmit(sSerialDescriptor, reinterpret_cast<uint8_t*>(const_cast<char*>(aBegin)), aEnd - aBegin, tTimeout);
        // }
    }

    static auto getBuffer() {
        return std::pair(sBegin, sEnd);
    }
};

}

#endif
