#pragma once

#include "messages.h"

namespace messages::core {
struct connection_added {};
struct connected {};
struct disconnected {};

using exported = type_list<connection_added, connected, disconnected>;

}  // namespace messages::core

generate_typeid_for(messages::core::connection_added);
generate_typeid_for(messages::core::connected);
generate_typeid_for(messages::core::disconnected);

