#pragma once

#include "Types.hpp"
#include "Yaml.hpp"

namespace Yaml
{
    namespace impl {
        template<>
        struct StringConverter<u32>
        {
            static u32 Get(const std::string & data)
            {
                return std::stoul(data, nullptr, 0);
            }

            static u32 Get(const std::string & data, const u32 & defaultValue)
            {
                if(data.size() == 0)
                {
                    return defaultValue;
                }
                return std::stoul(data, nullptr, 0);
            }
        };
    }
}