# Steps to be done next in order

- [x] (De)serialization of lists as range
- [ ] (De)serialization of objects via uncustomizable reflection of structs
    - [x] Reading
    - [ ] Writing
- [ ] Maybe deserialize_invoke, as preparation for (de)composition?
    - Simple, uncustomizable rules for now: single parameter = deserialize the type of the parameter. Multiple parameters = deserialize object with the corresponding fields.
- [ ] Customization by (de)composition
- [ ] (De)serialization of variants of basic types
    - This will need a way for ANY deserializable type to be able to tell what kind of json-type is expected. For types with their own readers, the readers themselves will provide that information somehow. For composable types the information can be inferred by reducing them down to object, array, or readable type.
    - A type may have multiple json-types (ie: `variant<string, int>` has 2)
    - Only unambiguous variants are supported; if 2 elements have an overlap in json types we get a compiletime error
- [ ] jser::Any, which can hold any json object (`struct Any: std::variant<int, bool, string, vector<Any>, map<string, Any>> {...};`, with custom (de)composition from/into the underlying variant)

# Steps at some point in the future

- [ ] Support heterogenous lists as tuples

# Desired Features

- Fully custom (de)serialization with iterators
    - Use-case: big number class, which can handle arbitrarily large integers or even floats. This can NOT be implemented with custom serializer that's expected to just translate things into types with builtin support
- (MAYBE OBSOLETE, see smart deserialization further down) Custom (de)serialization from/to object and array without having to create temporary values.
    - IE: I don't want to have to create a `vector<int>` to then pass it to my custom `constexpr static Foo deserialize(const vector<int>&)`. Rather, I'd like to instead expect some sort of adapter that I can read the values out of on the fly: `constexpr static Foo deserialize(jser::ArrayDeserializationContext& ctx)`. I can then do `ctx.next<int>()` etc to deserialize the next value. Same thing for objects, for which I already have `ObjectSerializationContext`, but the optimization is not yet supported for custom deserialization.
    - Same thing for serialize: `class Foo { constexpr vector<int> serialize(); };` is bad; we're creating a vector, then jser serializes it. Blergh. Better: `class Foo { constexpr void serialize(ArraySerializationContext& ctx); }`, with `ctx.append(<...>)` to write values right into the output!
- Support for tuple as json array
- Support for variant as single element of dynamic type
- Support for smart custom deserialization: if I create `Person deserialize(string name, int [[=jser::name("age_years")]] age)`, then jser should try to parse one string field with key "name" and one int field with key "age_years", then pass those values to deserialize to get the Person. This is proper reflection usage for deserialization! Use the parameter types and names for smart behavior!!! This (along with smart deserialization for arrays) might make the deserialization contexts obsolete!
    - Similar stuff for deserializing from arrays
- Come up with what smart serialization might look like because I don't know yet.

# Design considerations

## Field annotations vs custom field serializer

Currently default field formatter options are individual annotations, with a fully custom field formatter being another annotation.
This allows something like [[=jser::name("asd"), =jser::custom_field(<formatter>)]], which doesn't make sense because the jser::name will be ignored!
I mean, arguably, the user could be writing a custom field formatter that also respects the name, embed, omit, ... annotations... but then why not use the default field formatter in the first place?
Different idea is: have _ONE_ annotation that can be used to configure field serialization. That annotation can _either_ be options for the default serializer _OR_ be a custom serializer (also with options). Syntax is hard here, because the default stuff must be ergonomic, but for example:
[[=field(jser::Field {.omit = true, .name="asd",})]] int field;
[[=field(jser::CustomField<Serializer> { .foo = "bar" })]] int field;
-> kinda obvious that we should not have more than 1 field annotation!

### Decision

Won't be done.

This makes the default annotations and everything more complicated, as well as the option annotations more verbose!
custom fields should be an absolute exception anyway.
They're there so users that need super special things have a way to do them.
But if there's ever a wide spread use case where everyone starts copying the same custom field serializer, I'll just add equivalent functionality into the lib, so they can stop using it.
No need to spend to much time on making the custom field serializer more ergonomic! Just keep the jser::name, jser::embed etc annotations.

## Formatters (Basic, Pretty, maybe even BSON)

### Decision

Doesn't have to be done right away, might be a good way to go later.

But we will not go for the idea sketched out in src/utopia/crazy_formatters, because that one will lead to lots of template instantiations and if we need more caching and runtime options (ie pretty print or even optimized bson) we have to rely on the compiler to elide tons and tons of copies/moves, or it will be very expensive.

Best leave the formatter interface simple, even if that makes it "misusable". It's only used within the library. Only 0.01% of users will ever need to think about implementing a custom formatter interface. It will probably even stay in the detail namespace to signify that its stability is not guaranteed. So just make it simple! The concept in src/utopia/Utopia.hpp is a decent first draft.
