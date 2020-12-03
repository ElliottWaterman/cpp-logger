//
// Created by balazs on 2020. 12. 03..
//

#include "LogAppInterfaceStd.h"
#include "LogConverterCustomText.h"
#include "LogSenderStdOstream.h"
#include "LogSenderVoid.h"
#include "LogQueueStdBoost.h"
#include "LogQueueVoid.h"
#include "LogMessageCompact.h"
#include "LogMessageVariant.h"
#include "Log.h"

#include <iostream>

// clang++ -std=c++20 -Isrc -Icpp-memory-manager -Os -Wl,-Map,test-sizes.map -demangle test/test-sizes.cpp -lpthread -o test-sizes
// clang++ -std=c++20 -Isrc -Icpp-memory-manager -Os -S -fno-asynchronous-unwind-tables -fno-dwarf2-cfi-asm -masm=intel test/test-sizes.cpp -o test-sizes.s
// llvm-size-10 test-sizes

constexpr size_t cgThreadCount = 1;

namespace nowtech::LogTopics {
nowtech::log::TopicInstance system;
nowtech::log::TopicInstance surplus;
}

constexpr nowtech::log::TaskId cgMaxTaskCount = cgThreadCount + 1;
constexpr bool cgLogFromIsr = false;
constexpr bool cgArchitecture64 = true;
constexpr uint8_t cgAppendStackBufferSize = 100u;
constexpr bool cgAppendBasePrefix = true;
constexpr bool cgAlignSigned = false;
constexpr size_t cgTransmitBufferSize = 123u;
constexpr size_t cgPayloadSize = 8u;
constexpr size_t cgQueueSize = 444u;
constexpr nowtech::log::LogTopic cgMaxTopicCount = 2;
constexpr nowtech::log::TaskRepresentation cgTaskRepresentation = nowtech::log::TaskRepresentation::cName;

/*
constexpr size_t cgDirectBufferSize = 43u;
using LogAppInterfaceStd = nowtech::log::AppInterfaceStd<cgMaxTaskCount, cgLogFromIsr>;
constexpr typename LogAppInterfaceStd::LogTime cgTimeout = 123u;
constexpr typename LogAppInterfaceStd::LogTime cgRefreshPeriod = 444;
using LogMessage = nowtech::log::MessageVariant<cgPayloadSize>;
using LogConverterCustomText = nowtech::log::ConverterCustomText<LogMessage, cgArchitecture64, cgAppendStackBufferSize, cgAppendBasePrefix, cgAlignSigned>;
using LogSenderStdOstream = nowtech::log::SenderStdOstream<LogAppInterfaceStd, LogConverterCustomText, cgTransmitBufferSize, cgTimeout>;
using LogQueueVoid = nowtech::log::QueueVoid<LogMessage, LogAppInterfaceStd, cgQueueSize>;
using Log = nowtech::log::Log<LogQueueVoid, LogSenderStdOstream, cgMaxTopicCount, cgTaskRepresentation, cgDirectBufferSize, cgRefreshPeriod>;
   text	   data	    bss	    dec	    hex	filename
  17930	   1122	    560	  19612	   4c9c	test-sizes
 */

/* No log present at all:
   text	   data	    bss	    dec	    hex	filename
   3489	    826	    280	   4595	   11f3	test-sizes
*/


/*
constexpr size_t cgDirectBufferSize = 43u;
using LogAppInterfaceStd = nowtech::log::AppInterfaceStd<cgMaxTaskCount, cgLogFromIsr>;
constexpr typename LogAppInterfaceStd::LogTime cgTimeout = 123u;
constexpr typename LogAppInterfaceStd::LogTime cgRefreshPeriod = 444;
using LogMessage = nowtech::log::MessageVariant<cgPayloadSize>;
using LogConverterCustomText = nowtech::log::ConverterCustomText<LogMessage, cgArchitecture64, cgAppendStackBufferSize, cgAppendBasePrefix, cgAlignSigned>;
using LogSenderVoid = nowtech::log::SenderVoid<LogAppInterfaceStd, LogConverterCustomText, cgTransmitBufferSize, cgTimeout>;
using LogQueueVoid = nowtech::log::QueueVoid<LogMessage, LogAppInterfaceStd, cgQueueSize>;
using Log = nowtech::log::Log<LogQueueVoid, LogSenderVoid, cgMaxTopicCount, cgTaskRepresentation, cgDirectBufferSize, cgRefreshPeriod>;
   text	   data	    bss	    dec	    hex	filename
   3489	    826	    280	   4595	   11f3	test-sizes
 */

/*
constexpr size_t cgDirectBufferSize = 0u;
using LogAppInterfaceStd = nowtech::log::AppInterfaceStd<cgMaxTaskCount, cgLogFromIsr>;
constexpr typename LogAppInterfaceStd::LogTime cgTimeout = 123u;
constexpr typename LogAppInterfaceStd::LogTime cgRefreshPeriod = 444;
using LogMessage = nowtech::log::MessageVariant<cgPayloadSize>;
using LogConverterCustomText = nowtech::log::ConverterCustomText<LogMessage, cgArchitecture64, cgAppendStackBufferSize, cgAppendBasePrefix, cgAlignSigned>;
using LogSenderStdOstream = nowtech::log::SenderStdOstream<LogAppInterfaceStd, LogConverterCustomText, cgTransmitBufferSize, cgTimeout>;
using LogQueueStdBoost = nowtech::log::QueueStdBoost<LogMessage, LogAppInterfaceStd, cgQueueSize>;
using Log = nowtech::log::Log<LogQueueStdBoost, LogSenderStdOstream, cgMaxTopicCount, cgTaskRepresentation, cgDirectBufferSize, cgRefreshPeriod>;
   text	   data	    bss	    dec	    hex	filename
  29477	   1402	    952	  31831	   7c57	test-sizes
  */

constexpr size_t cgDirectBufferSize = 0u;
using LogAppInterfaceStd = nowtech::log::AppInterfaceStd<cgMaxTaskCount, cgLogFromIsr>;
constexpr typename LogAppInterfaceStd::LogTime cgTimeout = 123u;
constexpr typename LogAppInterfaceStd::LogTime cgRefreshPeriod = 444;
using LogMessage = nowtech::log::MessageCompact<cgPayloadSize>;
using LogConverterCustomText = nowtech::log::ConverterCustomText<LogMessage, cgArchitecture64, cgAppendStackBufferSize, cgAppendBasePrefix, cgAlignSigned>;
using LogSenderStdOstream = nowtech::log::SenderStdOstream<LogAppInterfaceStd, LogConverterCustomText, cgTransmitBufferSize, cgTimeout>;
using LogQueueStdBoost = nowtech::log::QueueStdBoost<LogMessage, LogAppInterfaceStd, cgQueueSize>;
using Log = nowtech::log::Log<LogQueueStdBoost, LogSenderStdOstream, cgMaxTopicCount, cgTaskRepresentation, cgDirectBufferSize, cgRefreshPeriod>;
/*
   text	   data	    bss	    dec	    hex	filename
  27957	   1402	    952	  30311	   7667	test-sizes
 */

uint64_t constexpr cgUint32 = 1234567890;
int64_t constexpr cgInt32 = -1234567890;
double constexpr cgReal = 3.3;
char constexpr cgString[] = "string\n";
size_t constexpr cgLength = 7u;

void todo() noexcept {
  std::cout.write(cgString, cgLength);
}

int main() {
  std::thread thread(todo);
  thread.join();

  nowtech::log::LogConfig logConfig;
  logConfig.allowRegistrationLog = true;
  LogSenderStdOstream::init(&std::cout);
  Log::init(logConfig);

  Log::registerTopic(nowtech::LogTopics::system, "system");
  Log::registerTopic(nowtech::LogTopics::surplus, "surplus");
  Log::registerCurrentTask("main");

  Log::i(nowtech::LogTopics::surplus) << cgUint32 << Log::end;
  Log::i(nowtech::LogTopics::system) << cgInt32 << Log::end;
  Log::n() << cgReal << cgString << Log::end;
  Log::n() << cgReal << LC::St << cgString << Log::end;

  Log::unregisterCurrentTask();
  Log::done();

  return 0;
}

