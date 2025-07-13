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
