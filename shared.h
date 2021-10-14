#pragma once

#include "sw_fwd.h"  // Forward declaration

#include <cstddef>  // std::nullptr_t

// https://en.cppreference.com/w/cpp/memory/shared_ptr
template <typename T>
class SharedPtr {
public:
    template <typename X>
    friend class SharedPtr;

    template <typename X>
    friend class WeakPtr;

    template <typename X>
    friend class EnableSharedFromThis;

    template <class X, class... Args>
    friend SharedPtr<X> MakeShared(Args&&... args);
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    SharedPtr() : ptr_(nullptr), controll_(new ControllBlock(1, 0)) {
    }

    SharedPtr(std::nullptr_t) : SharedPtr() {
    }

    template <class X = T>
    explicit SharedPtr(X* ptr) : ptr_(static_cast<T*>(ptr)), controll_(new ControllBlock(ptr)) {
        if constexpr (std::is_base_of_v<EnableSharedFromThisBase, X>) {
            SetLinkToOuterSharedPtr(ptr);
        }
    }

    SharedPtr(const SharedPtr& other) noexcept
        : ptr_(static_cast<T*>(other.ptr_)), controll_(other.controll_) {
        controll_->IncreaseStrong();
    }

    template <class X>
    SharedPtr(const SharedPtr<X>& other) noexcept
        : ptr_(static_cast<T*>(other.ptr_)), controll_(other.controll_) {
        controll_->IncreaseStrong();
    }

    SharedPtr(SharedPtr&& other) : ptr_(static_cast<T*>(other.ptr_)), controll_(other.controll_) {
        other.ptr_ = nullptr;
        other.controll_ = new ControllBlock(1, 0);
    }

    template <class X>
    SharedPtr(SharedPtr<X>&& other)
        : ptr_(static_cast<T*>(other.ptr_)), controll_(other.controll_) {
        other.ptr_ = nullptr;
        other.controll_ = new ControllBlock(1, 0);
    }

    // Aliasing constructor
    // #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    template <class X, class Y>
    SharedPtr(const SharedPtr<X>& other, Y* ptr)
        : ptr_(static_cast<T*>(ptr)), controll_(other.controll_) {
        controll_->IncreaseStrong();
    }

    // Promote `WeakPtr`
    // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    explicit SharedPtr(const WeakPtr<T>& other) {
        if (other.Expired()) {
            throw BadWeakPtr();
        } else {
            *this = other.Lock();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    SharedPtr& operator=(const SharedPtr& other) {
        if (Get() == other.Get()) {
            return *this;
        }

        UnlinkWithControllBlock();

        ptr_ = static_cast<T*>(other.ptr_);
        controll_ = other.controll_;
        controll_->IncreaseStrong();
        return *this;
    }

    template <class X>
    SharedPtr& operator=(const SharedPtr<X>& other) {
        if (Get() == other.Get()) {
            return *this;
        }

        UnlinkWithControllBlock();

        ptr_ = static_cast<T*>(other.ptr_);
        controll_ = other.controll_;
        controll_->IncreaseStrong();
        return *this;
    }

    SharedPtr& operator=(SharedPtr&& other) {
        if (Get() == other.Get()) {
            return *this;
        }

        UnlinkWithControllBlock();

        controll_ = other.controll_;
        ptr_ = static_cast<T*>(other.ptr_);
        other.ptr_ = nullptr;
        other.controll_ = nullptr;
        return *this;
    }

    template <class X>
    SharedPtr& operator=(SharedPtr<X>&& other) {
        if (Get() == other.Get()) {
            return *this;
        }

        UnlinkWithControllBlock();

        controll_ = other.controll_;
        ptr_ = static_cast<T*>(other.ptr_);
        other.ptr_ = nullptr;
        other.controll_ = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~SharedPtr() {
        UnlinkWithControllBlock();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        if (controll_->DecreaseStrong()) {
            delete controll_;
        }
        controll_ = new ControllBlock(1, 0);
        ptr_ = nullptr;
    }

    template <class X>
    void Reset(X* ptr) {
        UnlinkWithControllBlock();

        controll_ = new ControllBlock(ptr);
        ptr_ = static_cast<T*>(ptr);
    }

    void Swap(SharedPtr& other) {
        std::swap(other.ptr_, ptr_);
        std::swap(controll_, other.controll_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return ptr_;
    }

    T& operator*() const {
        return *ptr_;
    }

    T* operator->() const {
        return ptr_;
    }

    size_t UseCount() const {
        return (ptr_) ? controll_->strong : 0;
    }

    explicit operator bool() const {
        return ptr_;
    }

private:
    template <class X>  // private ctor for MakeShared and WeakPtr::Lock
    SharedPtr(X* pointer, ControllBlock* block) : ptr_(static_cast<T*>(pointer)), controll_(block) {
        controll_->IncreaseStrong();
        if constexpr (std::is_base_of_v<EnableSharedFromThisBase, T>) {
            SetLinkToOuterSharedPtr(pointer);
        }
    }

    void UnlinkWithControllBlock() {
        if (controll_ && controll_->DecreaseStrong()) {
            if (controll_->created_from_make_shared) {
                delete[](reinterpret_cast<char*>(controll_));
            } else {
                delete controll_;
            }
        }
    }

    template <class X>
    void SetLinkToOuterSharedPtr(EnableSharedFromThis<X>* base) {
        base->outer_shared_pointer_ = *this;
    }

private:
    T* ptr_ = nullptr;
    ControllBlock* controll_ = nullptr;
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right) noexcept {
    return left.Get() == right.Get();
}

// Allocate memory only once
template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {  // no warranty here, ctors might throw -> memory leak
    char* space = new char[sizeof(T) + sizeof(ControllBlock) + sizeof(AnyPtr<T>)];
    char* box_ptr = space + sizeof(ControllBlock);
    char* type_ptr = box_ptr + sizeof(AnyPtr<T>);
    new (reinterpret_cast<void*>(type_ptr)) T(std::forward<Args>(args)...);
    new (reinterpret_cast<void*>(box_ptr)) AnyPtr<T>(reinterpret_cast<T*>(type_ptr));
    new (reinterpret_cast<void*>(space)) ControllBlock(reinterpret_cast<BaseAnyPtr*>(box_ptr));
    return SharedPtr<T>(reinterpret_cast<T*>(type_ptr), reinterpret_cast<ControllBlock*>(space));
}

// Look for usage examples in tests

struct EnableSharedFromThisBase {};

template <typename T>
struct EnableSharedFromThis : public EnableSharedFromThisBase {

    SharedPtr<T> SharedFromThis() {
        return SharedPtr<T>(outer_shared_pointer_);
    }

    SharedPtr<const T> SharedFromThis() const {
        return SharedPtr<const T>(outer_shared_pointer_);
    }

    WeakPtr<T> WeakFromThis() noexcept {
        return WeakPtr<T>(outer_shared_pointer_);
    }

    WeakPtr<const T> WeakFromThis() const noexcept {
        return WeakPtr<const T>(outer_shared_pointer_);
    }

public:
    WeakPtr<T> outer_shared_pointer_;
};
