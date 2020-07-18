//
// Copyright 2018-2020 Now Technologies Zrt.
//
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without restriction,
// including without limitation the rights to use, copy, modify, merge,
// publish, distribute, sublicense, and/or sell copies of the Software,
// and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH
// THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#ifndef NOWTECH_LOG_INCLUDED
#define NOWTECH_LOG_INCLUDED

#include "ArrayMap.h"
#include <cstdint>
#include <type_traits>
#include <algorithm>
#include <atomic>
#include <limits>
#include <cmath>

namespace nowtech::log {

namespace NumericSystem {
  static constexpr uint8_t cBinary      =  2u;
  static constexpr uint8_t cDecimal     = 10u;
  static constexpr uint8_t cHexadecimal = 16u;
}

enum class Exception : uint8_t {
  cOutOfTaskIdsOrDoubleRegistration = 0u,
  cOutOfTopics                      = 1u,
  cCount                            = 2u
};

// We stick to 8-bit task IDs to let them fit in the first byte of a chunk.
typedef uint8_t TaskIdType;
typedef int8_t LogTopicType; // this needs to be signed to let the overload resolution work

template<typename tAppInterface, typename tInterface, size_t tMaxTopicCount, uint8_t tSizeofIntegerTransform, uint8_t tAppendStackBufferLength>
class Log;

class LogTopicInstance final {
  template<typename tAppInterface, typename tInterface, size_t tMaxTopicCount, uint8_t tSizeofIntegerTransform, uint8_t tAppendStackBufferLength>
  friend class Log;

public:
  static constexpr LogTopicType cInvalidTopic = std::numeric_limits<LogTopicType>::min();

private:
  LogTopicType mValue = cInvalidTopic;

  LogTopicType operator=(LogTopicType const aValue) {
    mValue = aValue;
    return aValue;
  }

public:
  LogTopicType operator*() {
    return mValue;
  }

  operator LogTopicType() {
    return mValue;
  }
};

/// Struct holding numeric system and zero fill information.
struct LogFormat {
  /// Base of the numeric system. Can be 2, 10, 16.
  uint8_t base;

  /// Number of digits to emit with zero fill, or 0 if no fill.
  uint8_t fill;

  /// Constructor.
  constexpr LogFormat()
  : base(0u)
  , fill(0u) {
  }

  /// Constructor.
  constexpr LogFormat(uint8_t const aBase, uint8_t const aFill)
  : base(aBase)
  , fill(aFill) {
  }

  LogFormat(LogFormat const &) = default;
  LogFormat(LogFormat &&) = default;
  LogFormat& operator=(LogFormat const &) = default;
  LogFormat& operator=(LogFormat &&) = default;

  bool isValid() const noexcept {
    return base == NumericSystem::cBinary || base == NumericSystem::cDecimal || base == NumericSystem::cHexadecimal;
  }
};

/// Configuration struct with default values for general usage.
struct LogConfig final {
public:
  /// Type of info to log about the sender task
  enum class TaskRepresentation : uint8_t {cInvalid, cId, cName};

  /// This is the default logging format and the only one I will document
  /// here. For the others, the letter represents the base of the number
  /// system and the number represents the minimum digits to write, possibly
  /// with leading zeros. When formats are applied to floating point
  /// numbers, the numeric system info is discarded.
  inline static constexpr LogFormat cDefault = LogFormat(10u, 0u);
  inline static constexpr LogFormat cInvalid = LogFormat(0u, 0u);
  inline static constexpr LogFormat cB4  = LogFormat( 2u, 4u);
  inline static constexpr LogFormat cB8  = LogFormat( 2u, 8u);
  inline static constexpr LogFormat cB12 = LogFormat( 2u, 12u);
  inline static constexpr LogFormat cB16 = LogFormat( 2u, 16u);
  inline static constexpr LogFormat cB24 = LogFormat( 2u, 24u);
  inline static constexpr LogFormat cB32 = LogFormat( 2u, 32u);
  inline static constexpr LogFormat cD1  = LogFormat(10u,  1u);
  inline static constexpr LogFormat cD2  = LogFormat(10u,  2u);
  inline static constexpr LogFormat cD3  = LogFormat(10u,  3u);
  inline static constexpr LogFormat cD4  = LogFormat(10u,  4u);
  inline static constexpr LogFormat cD5  = LogFormat(10u,  5u);
  inline static constexpr LogFormat cD6  = LogFormat(10u,  6u);
  inline static constexpr LogFormat cD7  = LogFormat(10u,  7u);
  inline static constexpr LogFormat cD8  = LogFormat(10u,  8u);
  inline static constexpr LogFormat cD16 = LogFormat(10u, 16u);
  inline static constexpr LogFormat cX1  = LogFormat(16u,  1u);
  inline static constexpr LogFormat cX2  = LogFormat(16u,  2u);
  inline static constexpr LogFormat cX3  = LogFormat(16u,  3u);
  inline static constexpr LogFormat cX4  = LogFormat(16u,  4u);
  inline static constexpr LogFormat cX6  = LogFormat(16u,  6u);
  inline static constexpr LogFormat cX8  = LogFormat(16u,  8u);
  inline static constexpr LogFormat cX16 = LogFormat(16u, 16u);

  /// If true, task registration will be sent to the output in the form
  /// in the form -=- Registered task: taskname (1) -=-
  bool allowRegistrationLog = true;

  /// If true, logging will work from ISR.
  bool logFromIsr = false;

  /// Length of a FreeRTOS queue in chunks.
  uint32_t queueLength = 64u;

  /// Length of the circular buffer used for message sorting, measured also in
  /// chunks.
  uint32_t circularBufferLength = 64u;

  /// Length of a buffer in the transmission double-buffer pair, in chunks.
  uint32_t transmitBufferLength = 32u;

  /// Length of a pause in ms during waiting for transmission of the other
  /// buffer or timeout while reading from the FreeRTOS queue.
  uint32_t pauseLength = 100u;

  /// Length of the period used to wait for messages before transmitting a partially
  /// filled transmission buffer. The shorter the aValue the more prompt the display.
  uint32_t refreshPeriod = 1000u;

  /// Signs if writing the FreeRTOS queue can block or should return on the expense
  /// of losing chunks. Note, that even in blocking mode the throughput can not
  /// reach the theoretical UART bps limit.
  /// Important! In non-blocking mode high demands will result in loss of complete
  /// messages and often an internal lockup of the log system.
  bool blocks = true;

  /// Representation of a task in the message header, if any. It can be missing,
  /// numeric task ID or FreeRTOS task name.
  TaskRepresentation taskRepresentation = TaskRepresentation::cId;

  /// True if number formatter should append 0b or 0x
  bool appendBasePrefix = false;

  /// Format for displaying the task ID in the message header.
  LogFormat taskIdFormat = cX2;

  /// Format for displaying the FreeRTOS ticks in the header, if any. Should be
  /// LogFormat::cInvalid to disable tick output.
  LogFormat tickFormat   = cD5;

  /// These are default formats for some types.
  LogFormat int8Format       = cDefault;
  LogFormat int16Format      = cDefault;
  LogFormat int32Format      = cDefault;
  LogFormat int64Format      = cDefault;
  LogFormat uint8Format      = cDefault;
  LogFormat uint16Format     = cDefault;
  LogFormat uint32Format     = cDefault;
  LogFormat uint64Format     = cDefault;
  LogFormat floatFormat      = cD5;
  LogFormat doubleFormat     = cD8;
  LogFormat longDoubleFormat = cD16;

  /// If true, positive numbers will be prepended with a space to let them align negatives.
  bool alignSigned = false;

  LogConfig() noexcept = default;
};

/// Dummy type to use in << chain as end marker.
enum class LogShiftChainMarker : uint8_t {
  cEnd      = 0u
};

class LogInterfaceBase {
protected:
  inline static constexpr char cUnknownTaskName[]   = "UNKNOWN";
  inline static constexpr char cAnonymousTaskName[] = "ANONYMOUS";
  inline static constexpr char cIsrTaskName[]       = "ISR";

  LogInterfaceBase() = delete;
};

// template<typename tLogSizeType, bool tBlocks = false, tLogSizeType tChunkSize = 8u>
// class LogInterface : public LogInterfaceBase {
// public:
//   typedef tLogSizeType LogSizeType;
//   static constexpr tLogSizeType cChunkSize = tChunkSize;
// };

template<typename tAppInterface, typename tInterface, size_t tMaxTopicCount, uint8_t tSizeofIntegerConversion = 4u, uint8_t tAppendStackBufferLength = 70u>
class Log final {
private:
  static constexpr uint8_t  cSizeofIntegerConversion = tSizeofIntegerConversion;
  typedef typename std::conditional<cSizeofIntegerConversion == 8u, uint64_t, uint32_t>::type IntegerConversionUnsigned;
  typedef typename std::conditional<cSizeofIntegerConversion == 8u, int64_t, int32_t>::type IntegerConversionSigned;
  typedef typename tInterface::LogSizeType LogSizeType;

  static constexpr TaskIdType  cInvalidTaskId  = tInterface::cInvalidTaskId;
  static constexpr TaskIdType  cLocalTaskId    = tInterface::cLocalTaskId;
  static constexpr TaskIdType  cMaxTaskIdCount = tInterface::cMaxTaskCount + 1u;
  static constexpr LogSizeType cChunkSize      = tInterface::cChunkSize;

  static constexpr LogTopicType cFreeTopicIncrement = 1;
  static constexpr LogTopicType cFirstFreeTopic = 0;

  static_assert(tMaxTopicCount <= std::numeric_limits<LogTopicType>::max());
  static_assert(std::is_unsigned<LogSizeType>::value);
  static_assert(cInvalidTaskId == std::numeric_limits<TaskIdType>::max());
  static_assert(tInterface::cMaxTaskCount < std::numeric_limits<TaskIdType>::max() - 1u);
  static_assert(tInterface::cMaxTaskCount == cLocalTaskId);
  static_assert(tSizeofIntegerConversion == 2u || tSizeofIntegerConversion == 4u || tSizeofIntegerConversion == 8u);

  static constexpr char cNumericError            = '#';

  static constexpr char cEndOfMessage            = '\r';
  static constexpr char cEndOfLine               = '\n';
  static constexpr char cNumericFill             = '0';
  static constexpr char cNumericMarkBinary       = 'b';
  static constexpr char cNumericMarkHexadecimal  = 'x';
  static constexpr char cMinus                   = '-';
  static constexpr char cSpace                   = ' ';
  static constexpr char cSeparatorFailure        = '@';
  static constexpr char cFractionDot             = '.';
  static constexpr char cPlus                    = '+';
  static constexpr char cScientificE             = 'e';


  inline static constexpr char cNan[]               = "nan";
  inline static constexpr char cInf[]               = "inf";
  inline static constexpr char cRegisteredTask[]    = "-=- Registered task: ";
  inline static constexpr char cUnregisteredTask[]  = "-=- Unregistered task: ";
  inline static constexpr char cStringToLogOnDone[] = "\n\r";

  inline static constexpr char cDigit2char[NumericSystem::cHexadecimal] = {
    '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'
  };

  class Appender final {
  private:
    LogSizeType mIndex = 1u;
    char mChunk[cChunkSize];

  public:
    Appender() noexcept : mIndex(1u) {
      mChunk[0u] = cInvalidTaskId;
    }

    Appender(TaskIdType const aTaskId) noexcept : mIndex(1u) {
      mChunk[0u] = aTaskId;
    }

    void startWithTaskId(TaskIdType const aTaskId) noexcept {
      mIndex = 1u;
      mChunk[0u] = aTaskId;
    }

    uint8_t getTaskId() noexcept {
      return static_cast<uint8_t>(*mChunk);
    }

    bool isValid() const noexcept {
      return mChunk[0u] != cInvalidTaskId;
    }

    void invalidate() noexcept {
      mChunk[0u] = cInvalidTaskId;
    }

    void push(char const aChar) noexcept {
      mChunk[mIndex] = aChar;
      ++mIndex;
      if(mIndex == cChunkSize) {
        tInterface::push(mChunk);
        mIndex = 1u;
      }
      else { // nothing to do
      }
    }

    void flush() noexcept {
      mChunk[mIndex] = cEndOfMessage;
      tInterface::push(mChunk);
      mIndex = 1u;
    }
  }; // class Appender

  class Chunk final {
  private:
    char * mOrigin;
    char * mChunk;
    LogSizeType mBufferBytes;

  public:
    Chunk(char * const aChunk, LogSizeType const aBufferLength, char * const aNow) noexcept
      : mOrigin(aChunk)
      , mChunk(aNow)
      , mBufferBytes(aBufferLength * cChunkSize) {
    }

    Chunk(char * const aChunk
      , LogSizeType const aBufferLength
      , TaskIdType const aTaskId) noexcept
      : mOrigin(aChunk)
      , mChunk(aChunk)
      , mBufferBytes(aBufferLength * cChunkSize) {
      mChunk[0] = *reinterpret_cast<char const *>(&aTaskId);
    }

    char* getData() const noexcept {
      return mChunk;
    }

    TaskIdType getTaskId() const noexcept {
      return *reinterpret_cast<TaskIdType*>(mChunk);
    }

    char* operator++() noexcept {
      mChunk += cChunkSize;
      if(mChunk == (mOrigin + mBufferBytes)) {
        mChunk = mOrigin;
      }
      else { // nothing to do
      }
      return mChunk;
    }

    void operator=(char * const aStart) noexcept {
      mChunk = const_cast<char*>(aStart);
    }

    void operator=(Chunk const& aChunk) noexcept {
      std::copy_n(mChunk, cChunkSize, aChunk.mChunk);
    }

    void invalidate() noexcept {
      mChunk[0] = static_cast<char>(cInvalidTaskId);
    }

    void fetch() noexcept {
      if(!tInterface::fetch(mChunk)) {
        mChunk[0u] = static_cast<char>(cInvalidTaskId);
      }
      else { // nothing to do
      }
    }
  }; // class Chunk

  class CircularBuffer final {
  private:
    /// Counted in chunks
    LogSizeType const cBufferLengthInChunks;
    char * const mBuffer;
    Chunk mStuffStart;
    Chunk mStuffEnd;
    LogSizeType mCount = 0u;
    LogSizeType mInspectedCount = 0u;
    bool mInspected = true;
    Chunk mFound;

  public:
    CircularBuffer(LogSizeType const aBufferLength) noexcept
      : cBufferLengthInChunks(aBufferLength)
      , mBuffer(tAppInterface::template _newArray<char>(aBufferLength * cChunkSize))
      , mStuffStart(mBuffer, aBufferLength, cInvalidTaskId)
      , mStuffEnd(mBuffer, aBufferLength, cInvalidTaskId)
      , mFound(mBuffer, aBufferLength, cInvalidTaskId) {
    }

    /// Not intended to be destroyed
    ~CircularBuffer() {
      tAppInterface::template _deleteArray<char>(mBuffer);
    }

    bool isEmpty() const noexcept {
      return mCount == 0;
    }

    bool isFull() const noexcept {
      return mCount == cBufferLengthInChunks;
    }

    bool isInspected() const noexcept {
      return mInspected;
    }

    void clearInspected() noexcept {
      mInspected = false;
      mInspectedCount = 0;
      mFound = mStuffStart.getData();
    }

    Chunk const &fetch() noexcept {
      mStuffEnd.fetch();
      return mStuffEnd;
    }

    Chunk const &peek() const noexcept {
      return mStuffStart;
    }

    void pop() noexcept {
      --mCount;
      ++mStuffStart;
      mFound = mStuffStart.getData();
    }

    void keepFetched() noexcept {
      ++mCount;
      ++mStuffEnd;
    }

    Chunk const &inspect(TaskIdType const aTaskId) noexcept {
      while(mInspectedCount < mCount && mFound.getTaskId() != aTaskId) {
        ++mInspectedCount;
        ++mFound;
      }
      if(mInspectedCount == mCount) {
        Chunk source(mBuffer, cBufferLengthInChunks, mStuffStart.getData());
        Chunk destination(mBuffer, cBufferLengthInChunks, mStuffStart.getData());
        while(source.getData() != mStuffEnd.getData()) {
          if(destination.getTaskId() != cInvalidTaskId) {
            if(source.getData() == destination.getData()) {
              ++source;
            }
            else { // nothing to do
            }
            ++destination;
          }
          else {
            if(source.getTaskId() == cInvalidTaskId) {
              ++source;
            }
            else {
              destination = source;
              source.invalidate();
            }
          }
        }
        char *startRemoved = destination.getData();
        char *endRemoved = mStuffEnd.getData();
        if(startRemoved > endRemoved) {
          endRemoved += cChunkSize * cBufferLengthInChunks;
        }
        else { // nothing to do
        }
        mCount -= (endRemoved - startRemoved) / cChunkSize;
        mStuffEnd = destination.getData();
        mInspected = true;
      }
      else { // nothing to do
      }
      return mFound;
    }

    void removeFound() noexcept {
      mFound.invalidate();
    }
  }; // class CircularBuffer

  class TransmitBuffers final {
  private:
    /// counted in chunks
    LogSizeType const cBufferLengthInChunks;
    LogSizeType const cBufferLengthInBytes;
    LogSizeType mBufferToWrite = 0u;
    char * mBuffers[2];
    LogSizeType mChunkCount[2] = { 0u, 0u };
    LogSizeType mIndex[2] = { 0u, 0u };
    uint8_t mActiveTaskId = cInvalidTaskId;
    bool mWasTerminalChunk = false;
    std::atomic<bool> mRefreshNeeded = false;

  public:
    TransmitBuffers(LogSizeType const aBufferLength) noexcept
      : cBufferLengthInChunks(aBufferLength)
      , cBufferLengthInBytes(aBufferLength * (cChunkSize - 1u)) {
      mBuffers[0] = tAppInterface::template _newArray<char>(cBufferLengthInBytes);
      mBuffers[1] = tAppInterface::template _newArray<char>(cBufferLengthInBytes);
    }

    ~TransmitBuffers() noexcept {
      tAppInterface::template _deleteArray<char>(mBuffers[0]);
      tAppInterface::template _deleteArray<char>(mBuffers[1]);
    }

    bool hasActiveTask() const noexcept {
      return mActiveTaskId != cInvalidTaskId;
    }

    TaskIdType getActiveTaskId() const noexcept {
      return mActiveTaskId;
    }

    void refreshNeeded() noexcept {
      mRefreshNeeded = true;
    }

    bool gotTerminalChunk() const noexcept {
      return mWasTerminalChunk;
    }

    /// Assumes that the buffer to write has space for it
    TransmitBuffers &operator<<(Chunk const &aChunk) noexcept {
      if(aChunk.getTaskId() != cInvalidTaskId) {
        LogSizeType i = 1u;
        char const * const origin = aChunk.getData();
        mWasTerminalChunk = false;
        char * buffer = mBuffers[mBufferToWrite];
        LogSizeType &index = mIndex[mBufferToWrite];
        while(!mWasTerminalChunk && i < cChunkSize) {
          if(index < cBufferLengthInBytes) {
            buffer[index] = origin[i];
            if (origin[i] == cEndOfMessage) {
              buffer[index] = cEndOfLine;
            }
            else { // nothing to do
            }
            ++index;
          }
          else { // nothing to do
          }
          mWasTerminalChunk = (origin[i] == cEndOfMessage);
          ++i;          
        }
        ++mChunkCount[mBufferToWrite];
        if(mWasTerminalChunk) {
          mActiveTaskId = cInvalidTaskId;
        }
        else {
          mActiveTaskId = aChunk.getTaskId();
        }
      }
      return *this;
    }

    void transmitIfNeeded() noexcept {
      if(mChunkCount[mBufferToWrite] == 0u) {
        return;
      }
      else {
        if(mChunkCount[mBufferToWrite] == cBufferLengthInChunks) {
          tInterface::waitWhileTransmitInProgress();
          mRefreshNeeded = true;
        }
        else { // nothing to do
        }
        if(tInterface::isTransmitDone() && mRefreshNeeded) {
          tInterface::transmit(mBuffers[mBufferToWrite], mIndex[mBufferToWrite]);
          mBufferToWrite = 1 - mBufferToWrite;
          mIndex[mBufferToWrite] = 0u;
          mChunkCount[mBufferToWrite] = 0u;
          mRefreshNeeded = false;
          tInterface::startRefreshTimer();
        }
        else { // nothing to do
        }
      }
    }
  }; // class TransmitBuffers

   /// Dummy type to use in << chain as end marker.
  enum class LogShiftChainMarker : uint8_t {
    cEnd      = 0u
  };

  Log() = delete;
  
  inline static std::atomic<bool> sKeepRunning = true;
  inline static LogConfig const * sConfig;
  inline static std::atomic<LogTopicType> sNextFreeTopic = cFirstFreeTopic;
  inline static ArrayMap<LogTopicType, char const *, tMaxTopicCount> sRegisteredTopics;
  inline static CircularBuffer* sCircularBuffer;
  inline static TransmitBuffers* sTransmitBuffers;
  inline static Appender* sShiftChainingAppenders;

public:
  /// Will be used as Log << something << to << log << Log::end;
  static constexpr LogShiftChainMarker end = LogShiftChainMarker::cEnd;

  // TODO remark in docs: must come before registering topics
  static void init(LogConfig const &aConfig) {
    sConfig = &aConfig;
    sCircularBuffer = tAppInterface::template _new<CircularBuffer>(sConfig->circularBufferLength);
    sTransmitBuffers = tAppInterface::template _new<TransmitBuffers>(sConfig->transmitBufferLength);
    tInterface::init(aConfig, [](){ transmitterTaskFunction(); }, [](){ sTransmitBuffers->refreshNeeded(); });
    sShiftChainingAppenders = tAppInterface::template _newArray<Appender>(cMaxTaskIdCount);
    tInterface::startRefreshTimer();
  }

  // TODO note in docs about init and done sequence
  static void done() {
    sKeepRunning = false;
    tInterface::done();
    tAppInterface::template _delete<CircularBuffer>(sCircularBuffer);
    tAppInterface::template _delete<TransmitBuffers>(sTransmitBuffers);
    tAppInterface::template _deleteArray<Appender>(sShiftChainingAppenders);
  }

  /// Registers the current task if not already present. It can register
  /// at most 255 tasks. All others will be handled as one.
  /// NOTE: this method locks to inhibit concurrent access of methods with the same name.
  static void registerCurrentTask() noexcept {
    registerCurrentTask(nullptr);
  }

  /// Registers the current task if not already present. It can register
  /// at most 254 tasks. All others will be handled as one.
  /// @param aTaskName Task name to use, when the osInterface supports it.
  static void registerCurrentTask(char const * const aTaskName) noexcept {
    TaskIdType taskId = tInterface::registerCurrentTask(aTaskName);
    if(taskId != cInvalidTaskId) {
      if(sConfig->allowRegistrationLog) {
        Appender appender(taskId);
        append(appender, cRegisteredTask);
        append(appender, aTaskName);
        append(appender, cSpace);
        append(appender, LogConfig::cD3, static_cast<uint16_t>(taskId));
        appender.flush();
      }
      else { // nothing to do
      }
    }
    else {
      tAppInterface::fatalError(Exception::cOutOfTaskIdsOrDoubleRegistration);
    }
  }

  static void unregisterCurrentTask() noexcept {
    TaskIdType taskId = tInterface::unregisterCurrentTask();
    if(taskId != cInvalidTaskId && sConfig->allowRegistrationLog) {
      Appender appender(taskId);
      append(appender, cUnregisteredTask);
      append(appender, LogConfig::cD3, static_cast<uint16_t>(taskId));
      appender.flush();
    }
    else { // nothing to do
    }
  }

  static void registerTopic(LogTopicInstance &aTopic, char const * const aPrefix) noexcept {
    aTopic = sNextFreeTopic.fetch_add(cFreeTopicIncrement);
    if(!sRegisteredTopics.insert(aTopic, aPrefix)) {
      tAppInterface::fatalError(Exception::cOutOfTopics);
    }
    else { // nothing to do
    }
  }

  static void fetchViaCircularAndTransmit() noexcept {
    Chunk const &chunk = sCircularBuffer->fetch();
    if(chunk.getTaskId() != cInvalidTaskId) {
      if(sTransmitBuffers->getActiveTaskId() == chunk.getTaskId()) {
        *sTransmitBuffers << chunk;
      }
      else {
        sCircularBuffer->keepFetched();
      }
    }
    else { // nothing to do
    }
  }

// TODO supplement .cpp for FreeRTOS with extern "C"
  /// Transmitter thread implementation.
  static void transmitterTaskFunction() noexcept {
    // we assume all the buffers are valid
    while(sKeepRunning) {
      tInterface::waitForData();
      // At this point the transmitBuffers must have free space for a chunk
      if(!sTransmitBuffers->hasActiveTask()) {
        if(sCircularBuffer->isEmpty()) {
          static_cast<void>(*sTransmitBuffers << sCircularBuffer->fetch());
        }
        else { // the circularbuffer may be full or not
          static_cast<void>(*sTransmitBuffers << sCircularBuffer->peek());
          sCircularBuffer->pop();
        }
      }
      else { // There is a task in the transmitBuffers to be continued
        if(sCircularBuffer->isEmpty()) {
          fetchViaCircularAndTransmit();
        }
        else if(!sCircularBuffer->isFull()) {
          if(sCircularBuffer->isInspected()) {
            fetchViaCircularAndTransmit();
          }
          else {
            Chunk const &chunk = sCircularBuffer->inspect(sTransmitBuffers->getActiveTaskId());
            if(!sCircularBuffer->isInspected()) {
              *sTransmitBuffers << chunk;
              sCircularBuffer->removeFound();
            }
            else { // nothing to do
            }
          }
        }
        else { // the circular buffer is full
          static_cast<void>(*sTransmitBuffers << sCircularBuffer->peek());
          sCircularBuffer->pop();
          sCircularBuffer->clearInspected();
        }
      }
      if(sTransmitBuffers->gotTerminalChunk()) {
        sCircularBuffer->clearInspected();
      }
      else {
      }
      sTransmitBuffers->transmitIfNeeded();
    }
    tInterface::finishedTransmitterTask();
  }

  template<typename tValueType>
  static void send(LogTopicType const aTopic, LogFormat const &aFormat, tValueType const aValue, TaskIdType const aTaskId = cLocalTaskId) noexcept {
    Appender appender(aTaskId);
    startSend(appender, aTopic);
    if(appender.isValid()) {
      append(appender, aFormat, aValue);
      appender.flush();
    }
    else { // nothing to do
    }
  }

  template<typename tValueType>
  static void send(tValueType const aValue, TaskIdType const aTaskId = cLocalTaskId) noexcept {
    Appender appender(aTaskId);
    startSend(appender);
    append(appender, LogConfig::cInvalid, aValue);
    appender.flush();
  }

  template<typename tValueType>
  static void sendNoHeader(LogTopicType const aTopic, LogFormat const &aFormat, tValueType const aValue, TaskIdType const aTaskId = cLocalTaskId) noexcept {
    Appender appender(aTaskId);
    startSendNoHeader(appender, aTopic);
    if(appender.isValid()) {
      append(appender, aFormat, aValue);
      appender.flush();
    }
    else { // nothing to do
    }
  }

  template<typename tValueType>
  static void sendNoHeader(tValueType const aValue, TaskIdType const aTaskId = cLocalTaskId) noexcept {
    Appender appender(aTaskId);
    startSendNoHeader(appender);
    append(appender, LogConfig::cInvalid, aValue);
    appender.flush();
  }

private:
  static void startSend(Appender& aAppender) noexcept {
    startSendNoHeader(aAppender);
    if(aAppender.isValid()) {
      if(sConfig->taskRepresentation == LogConfig::TaskRepresentation::cId) {
        append(aAppender, sConfig->taskIdFormat, aAppender.getTaskId());
        append(aAppender, cSpace);
      }
      else if(sConfig->taskRepresentation == LogConfig::TaskRepresentation::cName) {
        append(aAppender, tInterface::getCurrentTaskName());
        append(aAppender, cSpace);
      }
      else { // nothing to do
      }
      if(sConfig->tickFormat.isValid()) {
        append(aAppender, sConfig->tickFormat, tInterface::getLogTime());
        append(aAppender, cSpace);
      }
      else { // nothing to do
      }
    }
    else { // nothing to do
    }
  }

  static void startSend(Appender& aAppender, LogTopicType aTopic) noexcept {
    auto found = sRegisteredTopics.find(aTopic);
    if(found != sRegisteredTopics.end()) {
      startSend(aAppender);
      append(aAppender, found->value);
      append(aAppender, cSpace);
    }
    else {
      aAppender.invalidate();
    }
  }

  static void startSendNoHeader(Appender& aAppender) noexcept {
    if(tInterface::isInterrupt() && !sConfig->logFromIsr) {
      aAppender.invalidate();
    }
    else { // nothing to do
    }
  }

  static void startSendNoHeader(Appender& aAppender, LogTopicType aTopic) noexcept {
    auto found = sRegisteredTopics.find(aTopic);
    if(found != sRegisteredTopics.end()) {
      startSendNoHeader(aAppender);
      append(aAppender, found->value);
      append(aAppender, cSpace);
    }
    else {
      aAppender.invalidate();
    }
  }

private:
  static void append(Appender &aAppender, LogFormat const, bool const aBool) noexcept {
    if(aBool) {
      append(aAppender, "true");
    }
    else {
      append(aAppender, "false");
    }
  }

  static void append(Appender &aAppender, char const aCh) noexcept {
    aAppender.push(aCh);
  }

  static void append(Appender &aAppender, LogFormat const, char const aCh) noexcept {
    aAppender.push(aCh);
  }

  static void append(Appender &aAppender, char const * const aString) noexcept {
    char const * pointer = aString;
    if(pointer != nullptr) {
      while(*pointer != 0) {
        aAppender.push(*pointer);
        ++pointer;
      }
    }
    else { // nothing to do
    }
  }

  static void append(Appender &aAppender, LogFormat const, char const * const aString) noexcept {
    append(aAppender, aString);
  }

  static void append(Appender &aAppender, LogFormat const aFormat, int8_t const aValue) noexcept {
    if(aFormat.isValid()) {
      append(aAppender, static_cast<IntegerConversionSigned>(aValue), static_cast<IntegerConversionSigned>(aFormat.base), aFormat.fill);
    }
    else {
      append(aAppender, static_cast<IntegerConversionSigned>(aValue), static_cast<IntegerConversionSigned>(sConfig->int8Format.base), sConfig->int8Format.fill);
    }
  }

  static void append(Appender &aAppender, LogFormat const aFormat, int16_t const aValue) noexcept {
    if(aFormat.isValid()) {
      append(aAppender, static_cast<IntegerConversionSigned>(aValue), static_cast<IntegerConversionSigned>(aFormat.base), aFormat.fill);
    }
    else {
      append(aAppender, static_cast<IntegerConversionSigned>(aValue), static_cast<IntegerConversionSigned>(sConfig->int16Format.base), sConfig->int16Format.fill);
    }
  }

  static void append(Appender &aAppender, LogFormat const aFormat, int32_t const aValue) noexcept {
    if(aFormat.isValid()) {
      append(aAppender, static_cast<IntegerConversionSigned>(aValue), static_cast<IntegerConversionSigned>(aFormat.base), aFormat.fill);
    }
    else {
      append(aAppender, static_cast<IntegerConversionSigned>(aValue), static_cast<IntegerConversionSigned>(sConfig->int32Format.base), sConfig->int32Format.fill);
    }
  }

  static void append(Appender &aAppender, LogFormat const aFormat, int64_t const aValue) noexcept {
    if(aFormat.isValid()) {
      append(aAppender, aValue, static_cast<int64_t>(aFormat.base), aFormat.fill);
    }
    else {
      append(aAppender, aValue, static_cast<int64_t>(sConfig->int64Format.base), sConfig->int64Format.fill);
    }
  }

  static void append(Appender &aAppender, LogFormat const aFormat, uint8_t const aValue) noexcept {
    if(aFormat.isValid()) {
      append(aAppender, static_cast<IntegerConversionUnsigned>(aValue), static_cast<IntegerConversionUnsigned>(aFormat.base), aFormat.fill);
    }
    else {
      append(aAppender, static_cast<IntegerConversionUnsigned>(aValue), static_cast<IntegerConversionUnsigned>(sConfig->uint8Format.base), sConfig->uint8Format.fill);
    }
  }

  static void append(Appender &aAppender, LogFormat const aFormat, uint16_t const aValue) noexcept {
    if(aFormat.isValid()) {
      append(aAppender, static_cast<IntegerConversionUnsigned>(aValue), static_cast<IntegerConversionUnsigned>(aFormat.base), aFormat.fill);
    }
    else {
      append(aAppender, static_cast<IntegerConversionUnsigned>(aValue), static_cast<IntegerConversionUnsigned>(sConfig->uint16Format.base), sConfig->uint16Format.fill);
    }
  }

  static void append(Appender &aAppender, LogFormat const aFormat, uint32_t const aValue) noexcept {
    if(aFormat.isValid()) {
      append(aAppender, static_cast<IntegerConversionUnsigned>(aValue), static_cast<IntegerConversionUnsigned>(aFormat.base), aFormat.fill);
    }
    else {
      append(aAppender, static_cast<IntegerConversionUnsigned>(aValue), static_cast<IntegerConversionUnsigned>(sConfig->uint32Format.base), sConfig->uint32Format.fill);
    }
  }

  static void append(Appender &aAppender, LogFormat const aFormat, uint64_t const aValue) noexcept {
    if(aFormat.isValid()) {
      append(aAppender, aValue, static_cast<uint64_t>(aFormat.base), aFormat.fill);
    }
    else {
      append(aAppender, aValue, static_cast<uint64_t>(sConfig->uint64Format.base), sConfig->uint64Format.fill);
    }
  }

  static void append(Appender &aAppender, LogFormat const aFormat, float const aValue) noexcept {
    if(aFormat.isValid()) {
      append(aAppender, static_cast<long double>(aValue), aFormat.fill);
    }
    else {
      append(aAppender, static_cast<long double>(aValue), sConfig->floatFormat.fill);
    }
  }

  static void append(Appender &aAppender, LogFormat const aFormat, double const aValue) noexcept {
    if(aFormat.isValid()) {
      append(aAppender, static_cast<long double>(aValue), aFormat.fill);
    }
    else {
      append(aAppender, static_cast<long double>(aValue), sConfig->doubleFormat.fill);
    }
  }

  static void append(Appender &aAppender, LogFormat const aFormat, long double const aValue) noexcept {
    if(aFormat.isValid()) {
      append(aAppender, static_cast<long double>(aValue), aFormat.fill);
    }
    else {
      append(aAppender, static_cast<long double>(aValue), sConfig->longDoubleFormat.fill);
    }
  }

  class LogShiftChainHelper final {
    Appender& mAppender;
    LogFormat mNextFormat;

  public:
    LogShiftChainHelper() noexcept = delete;

    LogShiftChainHelper(Appender& aAppender) noexcept : mAppender(aAppender) {
    }

    template<typename ArgumentType>
    LogShiftChainHelper& operator<<(ArgumentType const aValue) noexcept {
      if(mAppender.isValid()) {
        append(mAppender, mNextFormat, aValue);
        mNextFormat.base = 0u;
      }
      else { // nothing to do
      }
      return *this;
    }

    LogShiftChainHelper& operator<<(LogFormat const aFormat) noexcept {
      mNextFormat = aFormat;
      return *this;
    }

    void operator<<(LogShiftChainMarker const) noexcept {
      if(mAppender.isValid()) {
        mAppender.flush();
      }
      else { // nothing to do
      }
    }
  }; // class LogShiftChainHelper
  friend class LogShiftChainHelper;

public:
  static LogShiftChainHelper i(TaskIdType const aTaskId = cLocalTaskId) noexcept {
    TaskIdType const taskId = tInterface::getCurrentTaskId(aTaskId);
    Appender& appender = sShiftChainingAppenders[taskId];
    appender.startWithTaskId(taskId);
    startSend(appender);
    return LogShiftChainHelper(appender);
  }

  static LogShiftChainHelper i(LogTopicType const aTopic, TaskIdType const aTaskId = cLocalTaskId) noexcept {
    TaskIdType const taskId = tInterface::getCurrentTaskId(aTaskId);
    Appender& appender = sShiftChainingAppenders[taskId];
    appender.startWithTaskId(taskId);
    startSend(appender, aTopic);
    return LogShiftChainHelper(appender);
  }

  static LogShiftChainHelper n(TaskIdType const aTaskId = cLocalTaskId) noexcept {
    TaskIdType const taskId = tInterface::getCurrentTaskId(aTaskId);
    Appender& appender = sShiftChainingAppenders[taskId];
    appender.startWithTaskId(taskId);
    startSendNoHeader(appender);
    return LogShiftChainHelper(appender);
  }

  static LogShiftChainHelper n(LogTopicType const aTopic, TaskIdType const aTaskId = cLocalTaskId) noexcept {
    TaskIdType const taskId = tInterface::getCurrentTaskId(aTaskId);
    Appender& appender = sShiftChainingAppenders[taskId];
    appender.startWithTaskId(taskId);
    startSendNoHeader(appender, aTopic);
    return LogShiftChainHelper(appender);
  }

private:
  /// Converts the number to string in a stack buffer and uses append(char
  /// const ch) to send it character by character. If the conversion fails
  /// (due to invalid base or too small buffer on stack) a # will be appended
  /// instead. For non-decimal numbers 0b or 0x is prepended.
  /// tValueType should be int32_t, uint32_t, int64_t or uint64_t to avoid too many template instantiations.
  /// @param aValue the number to convert
  /// @param aBase of the number system to use
  /// @param aFill number of digits to use at least. Shorter numbers will be
  /// filled using cNumericFill.
  /// @return the return aValue of the last append(char const ch) call.
  template<typename tValueType>
  static void append(Appender &aAppender, tValueType const aValue, tValueType const aBase, uint8_t const aFill) noexcept {
    tValueType tmpValue = aValue;
    uint8_t tmpFill = aFill;
    if((aBase != NumericSystem::cBinary) && (aBase != NumericSystem::cDecimal) && (aBase != NumericSystem::cHexadecimal)) {
      aAppender.push(cNumericError);
      return;
    }
    else { // nothing to do
    }
    if(sConfig->appendBasePrefix && (aBase == NumericSystem::cBinary)) {
      aAppender.push(cNumericFill);
      aAppender.push(cNumericMarkBinary);
    }
    else { // nothing to do
    }
    if(sConfig->appendBasePrefix && (aBase == NumericSystem::cHexadecimal)) {
      aAppender.push(cNumericFill);
      aAppender.push(cNumericMarkHexadecimal);
    }
    else { // nothing to do
    }
    char tmpBuffer[tAppendStackBufferLength];
    uint8_t where = 0u;
    bool negative = aValue < 0;
    do {
      tValueType mod = tmpValue % aBase;
      if(mod < 0) {
        mod = -mod;
      }
      else { // nothing to do
      }
      tmpBuffer[where] = cDigit2char[mod];
      ++where;
      tmpValue /= aBase;
    } while((tmpValue != 0) && (where < tAppendStackBufferLength));
    if(where >= tAppendStackBufferLength) {
      aAppender.push(cNumericError);
      return;
    }
    else { // nothing to do
    }
    if(negative) {
      aAppender.push(cMinus);
    }
    else if(sConfig->alignSigned && (aFill > 0u)) {
      aAppender.push(cSpace);
    }
    else { // nothing to do
    }
    if(tmpFill > where) {
      tmpFill -= where;
      while(tmpFill > 0u) {
        aAppender.push(cNumericFill);
        --tmpFill;
      }
    }
    for(--where; where > 0u; --where) {
      aAppender.push(tmpBuffer[where]);
    }
    aAppender.push(tmpBuffer[0]);
  }

  static void append(Appender& aAppender, long double const aValue, uint8_t const aDigitsNeeded) noexcept {
    if(std::isnan(aValue)) {
      append(aAppender, cNan);
      return;
    } else if(std::isinf(aValue)) {
      append(aAppender, cInf);
      return;
    } else if(aValue == 0.0l) {
      aAppender.push(cNumericFill);
      return;
    }
    else {
      long double value = aValue;
      if(value < 0.0l) {
          value = -value;
          aAppender.push(cMinus);
      }
      else if(sConfig->alignSigned) {
        aAppender.push(cSpace);
      }
      else { // nothing to do
      }
      long double exponent = floor(log10(value));
      long double normalized = value / pow(10.0l, exponent);
      int32_t firstDigit;
      for(uint8_t i = 1u; i < aDigitsNeeded; i++) {
        firstDigit = static_cast<int>(normalized);
        if(firstDigit > 9) {
          firstDigit = 9;
        }
        else { // nothing to do
        }
        aAppender.push(cDigit2char[firstDigit]);
        normalized = 10.0 * (normalized - firstDigit);
        if(i == 1u) {
          aAppender.push(cFractionDot);
        }
        else { // nothing to do
        }
      }
      firstDigit = static_cast<int>(round(normalized));
      if(firstDigit > 9) {
        firstDigit = 9;
      }
      else { // nothing to do
      }
      aAppender.push(cDigit2char[firstDigit]);
      aAppender.push(cScientificE);
      if(exponent >= 0) {
        aAppender.push(cPlus);
      }
      else { // nothing to do
      }
      append(aAppender, static_cast<int32_t>(exponent), static_cast<int32_t>(10), 0u);
    }
  }
};// class Log

} // namespace nowtech::log

#endif // NOWTECH_LOG_INCLUDED
