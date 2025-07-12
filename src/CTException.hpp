#pragma once

#include <exception>
#include <string_view>
#include <meta>

namespace jscheme {

class CTException : public std::exception {
public:
    template<size_t N>
    consteval CTException(const char(&message)[N])
    :_message{std::define_static_string(message)}
    {
    }

    constexpr const char* what() const noexcept override {
        return _message.data();
    }

private:
    std::string_view _message;
};

}