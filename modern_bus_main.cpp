#include <algorithm>
#include <any>
#include <cassert>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include "core_messages.hpp"
#include "statistics_messages.hpp"

template <class T>
struct spell_type;

template <class Msg>
class type_erased_handler {
 private:
  template <class M>
  class base {
   public:
    virtual ~base() = default;
    virtual void operator()(const M&) = 0;
  };

  template <class T, class M>
  class derived : public base<M> {
    std::shared_ptr<T> t;

   public:
    derived(std::shared_ptr<T> t) : t(std::move(t)) {}
    virtual void operator()(const M& m) override { (*t)(m); }
  };

 public:
  template <class T>
  explicit type_erased_handler(std::shared_ptr<T> t)
      : m_base_ptr(std::make_shared<derived<T, Msg>>(t)) {}
  void operator()(const Msg& m) { (*m_base_ptr)(m); }

 private:
  std::shared_ptr<base<Msg>> m_base_ptr;
};

template <class... Msgs>
struct message_bus_t {
  template <class Msg, class H>
  void register_one(Msg, std::shared_ptr<H> h_ptr) {
    // TODO: static assert that Msg in Msgs.
    if constexpr (std::is_invocable_v<H, Msg>) {
      auto h_typeid = typeid_for<Msg>::get();
      type_erased_handler<Msg> wrapped_h(h_ptr);
      m_handlers[h_typeid].push_back(wrapped_h);
    } else {
      auto h_typeid = typeid_for<Msg>::get();
    }
  }

  template <class H>
  void register_handler(std::shared_ptr<H> h) {
    (register_one(Msgs{}, h), ...);
  }

  template <class Msg>
  void raise(const Msg& m) const {
    // TODO: static assert that Msg in Msgs.
    auto h_typeid = typeid_for<Msg>::get();
    // std::cout << "raising, type id: " << h_typeid << "\n";
    if (auto it = m_handlers.find(h_typeid); it != m_handlers.end()) {
      for (auto& h : it->second) {
        auto h_casted = std::any_cast<type_erased_handler<Msg>>(h);
        h_casted(m);
      }
    }
  }

  std::map<typeid_t, std::list<std::any>> m_handlers;
};

template <class... Msgs>
auto make_message_bus_for_types(type_list<Msgs...>) {
  return message_bus_t<Msgs...>();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//      END OF MAGIC, NOW USAGE:

using all_messages =
    type_list_cat_t<messages::core::exported, messages::statistics::exported>;
using message_bus_type = decltype(make_message_bus_for_types(all_messages{}));

// some 3rd party handler which combines both statistics and core features.
class statistics_and_core_handler {
  message_bus_type& m_bus_ref;
  int m_connected_count = 0;

 public:
  statistics_and_core_handler(message_bus_type& bus_ref) : m_bus_ref(bus_ref) {}
  void operator()(const messages::core::connected&) {
    std::cout << __PRETTY_FUNCTION__ << "\n";
    if (++m_connected_count == 2) {
      std::cout << "connected twice, raising message2\n";
      m_bus_ref.raise(messages::statistics::message2{});
    }
  }

  void operator()(const messages::statistics::message1&) {
    std::cout << __PRETTY_FUNCTION__ << "\n";
  }
};

class core_messages_handler {
 public:
  void operator()(const messages::core::connection_added& m) {
    std::cout << __PRETTY_FUNCTION__ << "\n";
  }

  void operator()(const messages::core::connected& m) {
    std::cout << __PRETTY_FUNCTION__ << "\n";
  }

  void operator()(const messages::core::disconnected& m) {
    std::cout << __PRETTY_FUNCTION__ << "\n";
  }
};

// Example of handler in functional style.
template <typename... Ts>
struct overload : Ts... {
  using Ts::operator()...;
};
template <class... Ts>
overload(Ts...) -> overload<Ts...>;

auto functional_handler() {
  return overload{[](const messages::core::connected&) {
                    std::cout << __PRETTY_FUNCTION__ << "\n";
                  },
                  [](const messages::statistics::message1&) {
                    std::cout << __PRETTY_FUNCTION__ << "\n";
                  },
                  [](const messages::statistics::message2&) {
                    std::cout << __PRETTY_FUNCTION__ << "\n";
                  }};
}

int main() {
  auto bus = make_message_bus_for_types(all_messages{});

  // spell_type<all_messages> _all_messages;

  auto h1 = std::make_shared<core_messages_handler>();
  auto h2 = std::make_shared<statistics_and_core_handler>(bus);
  auto h3_obj = functional_handler();
  auto h3 = std::make_shared<decltype(h3_obj)>(std::move(h3_obj));

  bus.register_handler(h1);
  bus.register_handler(h2);
  bus.register_handler(h3);

  bus.raise(messages::core::disconnected{});
  bus.raise(messages::core::connected{});
  bus.raise(messages::statistics::message1{});
  bus.raise(messages::core::connected{});
}
