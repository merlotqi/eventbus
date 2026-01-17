#pragma once

#include <string>
#include <string_view>
#include <system_error>

namespace eventbus {

enum class ErrorCode {
  Success = 0,
  QueueFull,
  QueueClosed,
  BusShutdown,
  InvalidSubscription,
  SubscriptionExpired,
  DispatcherError,
  HandlerError,
  InvalidEventType,
  Timeout,
  UnknownError
};

class ErrorCategory : public std::error_category {
 public:
  const char *name() const noexcept override { return "eventbus"; }

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
