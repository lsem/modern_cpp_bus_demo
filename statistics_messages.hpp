#pragma once

#include "messages.h"

namespace messages::statistics {
struct message1 {};
struct message2 {};

using exported = type_list<message1, message2>;
}  // namespace messages::statistics

generate_typeid_for(messages::statistics::message1);
generate_typeid_for(messages::statistics::message2);
