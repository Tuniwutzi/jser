#include "BasicFormatter.hpp"

int main() {
    // std::string serialized = jser::serialize(Person{}); // use basic formatter by default
    // std::string pretty_serialized = jser::serialize(Person{}, jser::formatters::Pretty {.indent = 4}); // use pretty formatter

    // Maybe, far in the future:
    // std::vector<uint8_t> bson_serialized = jser::serialize(Person{}, jser::formatters::BSON{}); // use bson formatter
    // But this opens a can of worms - why not XML? Why not X, Y and Z format?
    // BSON might be possible - ultimately all we need is to serialize/deserialize all 5 standard json types, that's it

    return 0;
}