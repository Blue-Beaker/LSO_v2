#pragma once

#include <Geode/Geode.hpp>
struct OffsetData {
    std::map<int, int> offsets = std::map<int, int>();
};

template<>
struct matjson::Serialize<OffsetData> {
    static geode::Result<OffsetData> fromJson(Value const& value) {
        std::map<int, int> offsets;
        for (auto const& e : value) {
            auto k = e.getKey();
            if (!k.has_value()) {
                continue;
            }
            GEODE_UNWRAP_INTO(int const levelID, geode::utils::numFromString<int>(k.value()));
            GEODE_UNWRAP_INTO(int const offset, e.asInt());
            offsets[levelID]=offset;
        }
        return geode::Ok(OffsetData{offsets});
    }

    static Value toJson(OffsetData const& value) {
        auto obj = Value();
        for (auto const& [key, val] : value.offsets) {
            obj.set(geode::utils::numToString(key), val);
        }
        return obj;
    }
};