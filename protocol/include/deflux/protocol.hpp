/*
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * Part of the deflux torrenting client.
 * Copyright (c) 2024 @dodicidodici
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

#include <cstdint>
#include <variant>

#include "serialize.hpp"

namespace deflux {

enum class message_type : uint16_t {
    // types sent by the server
    success,
    failure,
    event,

    // types sent by clients
    shutdown,
    ping
};

enum class error_reason : uint16_t { invalid_request, no_such_method };

struct empty_t {
    friend void to_json(nlohmann::json& /*j*/, const empty_t& /*e*/)
    {
        // no-op
    }

    friend void from_json(const nlohmann::json& /*j*/, empty_t& /*e*/)
    {
        // no-op
    }
};

template <Serializable T>
    requires !std::same_as<T, error_reason>
             struct message_t {
    uint32_t tag;
    message_type type;
    std::variant<T, error_reason> data;

    [[nodiscard]] bool is_ok() const
    {
        return std::holds_alternative<T>(data);
    }

    /**
     * Convenience method for accessing the message data. Equivalent to `std::get<T>(message.data)`
     *
     * @throws std::bad_variant_access if `is_ok()` is false
     * @return message data
     */
    [[nodiscard]] T& get_data()
    {
        return std::get<T>(data);
    }

    /**
     * Convenience method for accessing the message's error reason. Equivalent to `std::get<error_reason>(message.data)`
     *
     * @throws std::bad_variant_access if `is_ok()` is true
     * @return error reason
     */
    [[nodiscard]] error_reason& get_error()
    {
        return std::get<error_reason>(data);
    }

    [[nodiscard]] const T& get_data() const
    {
        return std::get<T>(data);
    }

    [[nodiscard]] const error_reason& get_error() const
    {
        return std::get<error_reason>(data);
    }

    friend void to_json(nlohmann::json& j, const message_t& m)
    {
        detail::serialize(j, m.tag, "tag");
        detail::serialize(j, m.type, "type");

        if (m.is_ok())
            detail::serialize(j, m.get_data(), "data");
        else
            detail::serialize(j, m.get_error(), "data");
    }

    friend void from_json(const nlohmann::json& j, message_t& m)
    {
        detail::deserialize(j, m.tag, "tag");
        detail::deserialize(j, m.type, "type");

        if (m.is_ok())
            m.data = j["data"].get<T>();
        else
            m.data = j["data"].get<error_reason>();
    };
};

/**
 * Type alias for a `message` that holds unparsed data (represented as a json object)
 */
using unparsed_message_t = message_t<nlohmann::json>;

template <Serializable T>
message_t<T> make_response(const T& data, uint32_t tag = 0)
{
    return { tag, message_type::success, data };
}

inline message_t<empty_t> make_response(const error_reason& error, uint32_t tag = 0)
{
    return { tag, message_type::failure, error };
}

template <Serializable T>
message_t<T> make_request(const T& data, const message_type& method, uint32_t tag = 0)
{
    return { tag, method, data };
}

}

#endif // PROTOCOL_HPP
