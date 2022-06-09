#pragma once
#include <string>

template <class... Ts>
struct type_list {};

template <class L1, class L2>
struct type_list_cat;

template <class... L1, class... L2>
struct type_list_cat<type_list<L1...>, type_list<L2...>> {
  using result = type_list<L1..., L2...>;
};
template <class L1, class L2>
using type_list_cat_t = typename type_list_cat<L1, L2>::result;


using typeid_t = std::string;

template <class T>
struct typeid_for;

#define generate_typeid_for(X)              \
  template <>                               \
  struct typeid_for<X> {                    \
    static std::string get() { return #X; } \
  };
