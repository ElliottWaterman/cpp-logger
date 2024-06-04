cmake_minimum_required(VERSION 3.17)

include_directories(cpp-memory-manager)
include_directories(src)

add_library(cpp-logger STATIC
    cpp-memory-manager/PoolAllocator.h
    src/Log.h
    src/LogAppInterfaceFreeRtosMinimal.h
    # src/LogAppInterfaceStd.h
    src/LogAtomicBuffers.h
    src/LogConverterCustomText.h
    src/LogMessageBase.h
    src/LogMessageCompact.h
    src/LogMessageVariant.h
    src/LogNumericSystem.h
    src/LogQueueFreeRtos.h
    # src/LogQueueStdBoost.h
    # src/LogQueueStdCircular.h
    src/LogQueueVoid.h
    # src/LogSenderRos2.h
    # src/LogSenderStdOstream.h
    # src/LogSenderStmHalMinimal.h
    src/LogSenderVoid.h
)
