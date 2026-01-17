/**
 * @file error.h
 * @brief Error handling system for EventBus operations
 *
 * This file provides a comprehensive error handling system using std::error_code
 * and std::error_category. It defines error codes for various failure conditions
 * that can occur during EventBus operations, along with human-readable error
 * messages.
 *
 * The error system integrates seamlessly with C++ standard library error handling
 * and can be used with exceptions or error codes as preferred.
 */

#pragma once

#include <string>
#include <string_view>
#include <system_error>

namespace eventbus {

/**
 * @brief Error codes for EventBus operations
 *
 * Defines all possible error conditions that can occur during EventBus
 * operations. These codes are used with std::error_code for type-safe
 * error handling.
 */
enum class ErrorCode {
  /** Operation completed successfully */
  Success = 0,

  /** Event queue is full and cannot accept more events */
  QueueFull,

  /** Event queue is closed and not accepting new events */
  QueueClosed,

  /** EventBus is shutting down and not accepting new operations */
  BusShutdown,

  /** Subscription is invalid or corrupted */
  InvalidSubscription,

  /** Subscription has expired or been cancelled */
  SubscriptionExpired,

  /** Error occurred in the dispatcher */
  DispatcherError,

  /** Error occurred during event handler execution */
  HandlerError,

  /** Event type is not supported by this EventBus */
  InvalidEventType,

  /** Operation timed out */
  Timeout,

  /** Unknown or unexpected error */
  UnknownError
};

/**
 * @brief Error category implementation for EventBus errors
 *
 * Provides human-readable error messages for EventBus error codes.
 * Implements the std::error_category interface to integrate with
 * standard C++ error handling facilities.
 */
class ErrorCategory : public std::error_category {
 public:
  /**
   * @brief Get the name of this error category
   * @return "eventbus"
   */
  const char *name() const noexcept override { return "eventbus"; }

  /**
   * @brief Get human-readable error message for an error code
   * @param ev The error code value
   * @return Human-readable error message
   */
  std::string message(int ev) const override {
    using namespace std::string_view_literals;

    std::string_view msg;
    switch (static_cast<ErrorCode>(ev)) {
      case ErrorCode::Success:
        msg = "Success"sv;
        break;
      case ErrorCode::QueueFull:
        msg = "Event queue is full"sv;
        break;
      case ErrorCode::QueueClosed:
        msg = "Event queue is closed"sv;
        break;
      case ErrorCode::BusShutdown:
        msg = "EventBus is shutting down"sv;
        break;
      case ErrorCode::InvalidSubscription:
        msg = "Invalid subscription"sv;
        break;
      case ErrorCode::SubscriptionExpired:
        msg = "Subscription expired"sv;
        break;
      case ErrorCode::DispatcherError:
        msg = "Dispatcher error"sv;
        break;
      case ErrorCode::HandlerError:
        msg = "Handler execution error"sv;
        break;
      case ErrorCode::InvalidEventType:
        msg = "Invalid event type"sv;
        break;
      case ErrorCode::Timeout:
        msg = "Operation timed out"sv;
        break;
      case ErrorCode::UnknownError:
        msg = "Unknown error"sv;
        break;
      default:
        msg = "Unknown error code"sv;
    }
    return std::string(msg);
  }
};

inline const std::error_category &eventbus_category() {
  static ErrorCategory instance;
  return instance;
}

inline std::error_code make_error_code(ErrorCode e) { return {static_cast<int>(e), eventbus_category()}; }

}  // namespace eventbus

namespace std {
template <>
struct is_error_code_enum<eventbus::ErrorCode> : true_type {};
}  // namespace std
