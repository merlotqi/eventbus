/**
 * @file event.h
 * @brief Type-safe event container using std::variant
 *
 * This file provides the Event class which is a type-safe container for
 * different event types using std::variant. It allows storing and retrieving
 * events of different types in a type-safe manner.
 *
 * Key features:
 * - Compile-time type safety using std::variant
 * - Efficient storage with minimal overhead
 * - Support for in-place construction
 * - Visit pattern for type-safe event processing
 */

#pragma once

#include <type_traits>
#include <utility>
#include <variant>

namespace eventbus {

template <typename T>
struct is_event_payload : std::conjunction<std::is_destructible<T>, std::negation<std::is_abstract<T>>,
                                           std::negation<std::is_reference<T>>> {};

template <typename T>
inline constexpr bool is_event_payload_v = is_event_payload<T>::value;

/**
 * @brief Type-safe event container using std::variant
 *
 * The Event class provides a type-safe container for different event types
 * using std::variant. It allows storing events of different types and provides
 * methods to check and access the stored event type.
 *
 * @tparam Ts Variadic template parameters listing all supported event types
 *
 * Example usage:
 * @code
 * // Define event types
 * struct UserLoggedIn { std::string username; };
 * struct OrderPlaced { int order_id; double amount; };
 *
 * // Create event container
 * Event<UserLoggedIn, OrderPlaced> event;
 *
 * // Store an event
 * event = UserLoggedIn{"alice"};
 *
 * // Check and access
 * if (event.is<UserLoggedIn>()) {
 *     // Event contains UserLoggedIn
 * }
 * @endcode
 */
template <typename... Ts>
class Event {
 public:
  using Variant = std::variant<std::monostate, Ts...>;

  /**
   * @brief Default constructor creating an empty event
   *
   * Creates an event containing std::monostate, indicating no event data.
   */
  constexpr Event() : data_(std::monostate{}) {}

  /**
   * @brief Constructor accepting any supported event type
   *
   * @tparam T The event type to store (must be in Ts...)
   * @param value The event value to store
   */
  template <typename T, typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, Event> &&
                                                    (std::is_same_v<std::decay_t<T>, Ts> || ...)>>
  constexpr Event(T &&value) noexcept(std::is_nothrow_constructible_v<Variant, T>) : data_(std::forward<T>(value)) {}

  /**
   * @brief In-place constructor for event types
   *
   * Constructs an event of type T in-place using the provided arguments.
   *
   * @tparam T The event type to construct (must be in Ts...)
   * @tparam Args Constructor argument types
   * @param args Arguments to forward to T's constructor
   */
  template <typename T, typename... Args, typename = std::enable_if_t<(std::is_same_v<T, Ts> || ...)>>
  explicit constexpr Event(std::in_place_type_t<T>, Args &&...args)
      : data_(std::in_place_type<T>, std::forward<Args>(args)...) {}

  /**
   * @brief Factory method for creating events
   *
   * @tparam T The event type to create
   * @tparam Args Constructor argument types
   * @param args Arguments to forward to T's constructor
   * @return Event containing the constructed T instance
   */
  template <typename T, typename... Args>
  static constexpr Event make(Args &&...args) {
    return Event(std::in_place_type<T>, std::forward<Args>(args)...);
  }

  /**
   * @brief Get access to the underlying std::variant
   *
   * @return Const reference to the internal std::variant
   */
  [[nodiscard]] constexpr const Variant &variant() const noexcept { return data_; }

  /**
   * @brief Check if the event is empty (contains no data)
   *
   * @return true if the event contains std::monostate, false otherwise
   */
  [[nodiscard]] constexpr bool empty() const noexcept { return std::holds_alternative<std::monostate>(data_); }

  /**
   * @brief Check if the event contains a specific type
   *
   * @tparam T The type to check for
   * @return true if the event contains type T, false otherwise
   */
  template <typename T>
  [[nodiscard]] constexpr bool is() const noexcept {
    return std::holds_alternative<T>(data_);
  }

 private:
  Variant data_;
};

/**
 * @brief Visit the event with a visitor function
 *
 * Applies the visitor to the event's internal variant. This enables type-safe
 * event processing where the visitor can handle different event types.
 *
 * @tparam Visitor The visitor callable type (function, lambda, functor)
 * @tparam Ts The event types supported by the Event
 * @param visitor The visitor to apply to the event
 * @param ev The event to visit
 * @return The result of applying the visitor to the event data
 *
 * Example usage:
 * @code
 * Event<UserLoggedIn, OrderPlaced> event = UserLoggedIn{"alice"};
 *
 * visit([](const auto& e) {
 *     using T = std::decay_t<decltype(e)>;
 *     if constexpr (std::is_same_v<T, UserLoggedIn>) {
 *         std::cout << "User logged in: " << e.username << std::endl;
 *     } else if constexpr (std::is_same_v<T, OrderPlaced>) {
 *         std::cout << "Order placed: " << e.order_id << std::endl;
 *     }
 * }, event);
 * @endcode
 */
template <typename Visitor, typename... Ts>
constexpr decltype(auto) visit(Visitor &&visitor, const Event<Ts...> &ev) {
  return std::visit(std::forward<Visitor>(visitor), ev.variant());
}

}  // namespace eventbus
