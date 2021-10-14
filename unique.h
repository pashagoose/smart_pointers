#pragma once

#include "compressed_pair.h"
#include <memory>
#include <cstddef>  // std::nullptr_t

// Primary template
template <typename T, typename Deleter = std::default_delete<T>>
class UniquePtr {
public:
    template <typename X, typename OtherDeleter>
    friend class UniquePtr;
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    template <typename X = T>
    explicit UniquePtr(X* ptr = nullptr) : gut_(static_cast<T*>(ptr), Deleter()) {
    }

    template <typename PointerType, typename DeleterType>
    UniquePtr(PointerType ptr, DeleterType&& deleter)
        : gut_(static_cast<T*>(ptr), std::forward<DeleterType>(deleter)) {
    }

    template <typename X, typename DeleterType>
    UniquePtr(UniquePtr<X, DeleterType>&& other) noexcept {
        gut_.GetFirst() = static_cast<T*>(other.gut_.GetFirst());
        gut_.GetSecond() = std::move(other.gut_.GetSecond());
        other.gut_.GetFirst() = nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        gut_.GetSecond()(gut_.GetFirst());
        gut_.GetFirst() = static_cast<T*>(other.gut_.GetFirst());
        gut_.GetSecond() = std::forward<Deleter>(other.gut_.GetSecond());
        other.gut_.GetFirst() = nullptr;
        return *this;
    }

    UniquePtr& operator=(std::nullptr_t) {
        gut_.GetSecond()(gut_.GetFirst());
        gut_.GetFirst() = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        gut_.GetSecond()(gut_.GetFirst());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        T* return_val = gut_.GetFirst();
        gut_.GetFirst() = nullptr;
        return return_val;
    }

    void Reset(T* ptr = nullptr) {
        if (ptr == gut_.GetFirst()) {
            return;
        }
        std::swap(gut_.GetFirst(), ptr);
        gut_.GetSecond()(ptr);
    }

    void Swap(UniquePtr& other) {
        std::swap(other.gut_.GetFirst(), gut_.GetFirst());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return gut_.GetFirst();
    }

    Deleter& GetDeleter() {
        return gut_.GetSecond();
    }

    const Deleter& GetDeleter() const {
        return gut_.GetSecond();
    }

    explicit operator bool() const {
        return gut_.GetFirst() != nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    std::add_lvalue_reference_t<T> operator*() const {
        return *gut_.GetFirst();
    }

    T* operator->() const {
        return gut_.GetFirst();
    }

private:
    CompressedPair<T*, Deleter> gut_;
};

// Specialization for arrays
template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
public:
    template <typename X, typename OtherDeleter>
    friend class UniquePtr;
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    template <typename X = T>
    explicit UniquePtr(X* ptr = nullptr) : gut_(static_cast<T*>(ptr), Deleter()) {
    }

    template <typename PointerType, typename DeleterType>
    UniquePtr(PointerType ptr, DeleterType&& deleter)
        : gut_(static_cast<T*>(ptr), std::forward<DeleterType>(deleter)) {
    }

    template <typename X, typename DeleterType>
    UniquePtr(UniquePtr<X, DeleterType>&& other) noexcept {
        gut_.GetFirst() = static_cast<T*>(other.gut_.GetFirst());
        gut_.GetSecond() = std::move(other.gut_.GetSecond());
        other.gut_.GetFirst() = nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        gut_.GetSecond()(gut_.GetFirst());
        gut_.GetFirst() = static_cast<T*>(other.gut_.GetFirst());
        gut_.GetSecond() = std::move(other.gut_.GetSecond());
        other.gut_.GetFirst() = nullptr;
        return *this;
    }

    UniquePtr& operator=(std::nullptr_t) {
        gut_.GetSecond()(gut_.GetFirst());
        gut_.GetFirst() = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        gut_.GetSecond()(gut_.GetFirst());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        T* return_val = gut_.GetFirst();
        gut_.GetFirst() = nullptr;
        return return_val;
    }

    void Reset(T* ptr = nullptr) {
        if (ptr == gut_.GetFirst()) {
            return;
        }
        std::swap(gut_.GetFirst(), ptr);
        gut_.GetSecond()(ptr);
    }

    void Swap(UniquePtr& other) {
        std::swap(other.gut_.GetFirst(), gut_.GetFirst());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return gut_.GetFirst();
    }

    Deleter& GetDeleter() {
        return gut_.GetSecond();
    }

    const Deleter& GetDeleter() const {
        return gut_.GetSecond();
    }

    explicit operator bool() const {
        return gut_.GetFirst() != nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Muttiple-object dereference operators

    T* operator->() const {
        return gut_.GetFirst();
    }

    T& operator[](size_t index) {
        return gut_.GetFirst()[index];
    }

    const T& operator[](size_t index) const {
        return gut_.GetFirst()[index];
    }

private:
    CompressedPair<T*, Deleter> gut_;
};
