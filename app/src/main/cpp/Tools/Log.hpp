#ifndef _ROBOT_LOG_HPP_
#define _ROBOT_LOG_HPP_

#include <cstdarg>
#include <mutex>
#include <chrono>

class Log {
public:
  /*** type define ***/
  enum Level {
    Warn = 0,
    Info,
    Debug,
    Verbose,
    Trace,
  };

  /*** static function and variable ***/
  static void SetLevel(enum Level level);
  static void F(const char* tag, const char* format, ...);
  static void E(const char* tag, const char* format, ...);
  static void W(const char* tag, const char* format, ...);
  static void I(const char* tag, const char* format, ...);
  static void D(const char* tag, const char* format, ...);
  static void V(const char* tag, const char* format, ...);
  static void T(const char* tag, const char* function, int line, const char* format, ...);
  static uint64_t Now();
  static uint64_t MilliNow();
  static uint64_t MicroNow();
  static uint64_t NanoNow();

  static constexpr const char *TAG = "ROBOT";

  /*** class function and variable ***/

private:
  /*** type define ***/

  /*** static function and variable ***/
  static inline void log(const char head, const char* tag, const char* format, ...);
  static inline void log(const char head, const char* tag, const char* format, va_list ap);

  static enum Level m_logLevel;
  static std::recursive_mutex m_mutex;
//  static const std::chrono::steady_clock::time_point m_baseTime;

#if defined(__ANDROID__)
  static inline int convPrio(const char head);
#endif

  /*** class function and variable ***/
  Log() { }
  virtual ~Log() = 0;
};

#endif /* _ROBOT_LOG_HPP_ */
