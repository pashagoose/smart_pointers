#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>

template <typename T, size_t I, bool EnableEBO = std::is_empty_v<T> && !std::is_final_v<T>>
struct CompressedPairElement {
    CompressedPairElement() : value_() {
    }

    template <typename X>
    CompressedPairElement(X&& element) : value_(std::forward<X>(element)) {
    }

    T& Get() {
        return value_;
    }

    const T& Get() const {
        return value_;
    }

    T value_;
};

template <typename T, size_t I>
struct CompressedPairElement<T, I, true> : public T {
    CompressedPairElement() : T() {
    }

    template <typename X>
    CompressedPairElement(X&& element) : T(std::forward<X>(element)) {
    }

    CompressedPairElement<T, I, true>& Get() {
        return *this;
    }

    const CompressedPairElement<T, I, true>& Get() const {
        return *this;
    }
};

template <typename F, typename S>
class CompressedPair : private CompressedPairElement<F, 0>, private CompressedPairElement<S, 1> {
public:
    CompressedPair() = default;

    template <typename X, typename Y>
    CompressedPair(X&& first, Y&& second)
        : CompressedPairElement<F, 0>(std::forward<X>(first)),
          CompressedPairElement<S, 1>(std::forward<Y>(second)) {
    }

    F& GetFirst() {
        return CompressedPairElement<F, 0>::Get();
    }

    const F& GetFirst() const {
        return CompressedPairElement<F, 0>::Get();
    }

    S& GetSecond() {
        return CompressedPairElement<S, 1>::Get();
    }

    const S& GetSecond() const {
        return CompressedPairElement<S, 1>::Get();
    };
};