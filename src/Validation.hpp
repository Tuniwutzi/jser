#pragma once

namespace jscheme {

namespace validation {

template<typename T>
struct InRange {
    T min;
    T max;
};

consteval auto in_range(auto min, auto max) {
    return InRange {
        .min = min,
        .max = max,
    }; 
}

}

}