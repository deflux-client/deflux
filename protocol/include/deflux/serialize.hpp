#ifndef SERIALIZE_HPP
#define SERIALIZE_HPP

#include <nlohmann/json.hpp>

#define DEFLUXD_JSON_TO(name) deflux::detail::serialize(nlohmann_json_j, nlohmann_json_t.name, #name);
#define DEFLUXD_JSON_FROM(name) deflux::detail::deserialize(nlohmann_json_j, nlohmann_json_t.name, #name);
#define DEFLUXD_SERIALIZABLE(Type, ...)                                                                                \
    friend void to_json(nlohmann::json& nlohmann_json_j, const Type& nlohmann_json_t)                                  \
    {                                                                                                                  \
        NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(DEFLUXD_JSON_TO, __VA_ARGS__))                                        \
    }                                                                                                                  \
    friend void from_json(const nlohmann::json& nlohmann_json_j, Type& nlohmann_json_t)                                \
    {                                                                                                                  \
        NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(DEFLUXD_JSON_FROM, __VA_ARGS__))                                      \
    }

namespace deflux {

template <typename T>
concept Serializable = requires(T a) {
    {
        nlohmann::json(a)
    } -> std::same_as<nlohmann::json>;
} || requires(T a) {
    {
        nlohmann::json(*a) // for std::optional
    } -> std::same_as<nlohmann::json>;
};

// helper functions that avoid serializing `std::optional`s if they are `std::nullopt`
// when using `NLOHMANN_DEFINE_TYPE_*`, the deserialization function throws if a key is absent from the json
// (even if optional), and the serialization function inserts a key with null. If that's not the behaviour
// you want, you have to write the functions manually, which i'd prefer not to.
// `DEFLUXD_SERIALIZABLE` does the same thing as `NLOHMANN_DEFINE_TYPE_INTRUSIVE`, but without throwing or including
// a null value
namespace detail {

    template <Serializable T>
    void serialize(nlohmann::json& json, const T& to_serialize, const std::string& serialized_name)
    {
        json[serialized_name] = to_serialize;
    }

    template <Serializable T>
    void serialize(nlohmann::json& json, const std::optional<T>& to_serialize, const std::string& serialized_name)
    {
        if (to_serialize)
            json[serialized_name] = *to_serialize;
    }

    template <Serializable T>
    void deserialize(const nlohmann::json& json, T& to_deserialize, const std::string& serialized_name)
    {
        to_deserialize = json.at(serialized_name);
    }

    template <Serializable T>
    void deserialize(const nlohmann::json& json, std::optional<T>& to_deserialize, const std::string& serialized_name)
    {
        if (json.contains(serialized_name) && !json[serialized_name].is_null())
            to_deserialize = json[serialized_name].get<T>();
        else
            to_deserialize = std::nullopt;
    }

}

}

#endif // SERIALIZE_HPP
