#pragma once

#include <Geode/Geode.hpp>
struct OffsetData {
    std::map<int, int> m_offsets = std::map<int, int>();
};

template<>
struct matjson::Serialize<OffsetData> {
    static geode::Result<OffsetData> fromJson(Value const& value) {
        std::map<int, int> offsets;
        for (const auto& e : value) {
            auto k = e.getKey();
            if (!k.has_value()) {
                continue;
            }
            GEODE_UNWRAP_INTO(int levelID, geode::utils::numFromString<int>(k.value()));
            GEODE_UNWRAP_INTO(int offset, e.asInt());
            offsets[levelID]=offset;
        }
        return geode::Ok(OffsetData{offsets});
    }

    static Value toJson(OffsetData const& value) {
        auto obj = Value();
        for (const auto& [key, val] : value.m_offsets) {
            obj.set(geode::utils::numToString(key), val);
        }
        return obj;
    }
};