#pragma once

#include "sw_fwd.h"  // Forward declaration

// https://en.cppreference.com/w/cpp/memory/weak_ptr
template <typename T>
class WeakPtr {
public:
    template <typename X>
    friend class SharedPtr;

    template <typename X>
    friend class WeakPtr;

    template <typename X>
    friend class EnableSharedFromThis;
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    WeakPtr() : ptr_(nullptr), controll_(new ControllBlock(0, 1)) {
    }

    WeakPtr(const WeakPtr& other) : ptr_(static_cast<T*>(other.ptr_)), controll_(other.controll_) {
        controll_->IncreaseWeak();
    }

    template <class X>
    WeakPtr(const WeakPtr<X>& other)
        : ptr_(static_cast<T*>(other.ptr_)), controll_(other.controll_) {
        controll_->IncreaseWeak();
    }

    WeakPtr(WeakPtr&& other) : ptr_(static_cast<T*>(other.ptr_)), controll_(other.controll_) {
        other.ptr_ = nullptr;
        other.controll_ = nullptr;
    }

    template <class X>
    WeakPtr(WeakPtr<X>&& other) : ptr_(static_cast<T*>(other.ptr_)), controll_(other.controll_) {
        other.ptr_ = nullptr;
        other.controll_ = nullptr;
    }

    // Demote `SharedPtr`
    // #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr
    template <class X>
    WeakPtr(const SharedPtr<X>& other) : ptr_(other.ptr_), controll_(other.controll_) {
        controll_->IncreaseWeak();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    WeakPtr& operator=(const WeakPtr& other) {
        if (ptr_ == other.ptr_) {
            return *this;
        }

        UnlinkWithControllBlock();

        ptr_ = static_cast<T*>(other.ptr_);
        controll_ = other.controll_;
        controll_->IncreaseWeak();
        return *this;
    }

    template <class X>
    WeakPtr& operator=(const WeakPtr<X>& other) {
        if (ptr_ == other.ptr_) {
            return *this;
        }

        UnlinkWithControllBlock();

        ptr_ = static_cast<T*>(other.ptr_);
        controll_ = other.controll_;
        controll_->IncreaseWeak();
        return *this;
    }

    WeakPtr& operator=(WeakPtr&& other) {
        if (ptr_ == other.ptr_) {
            return *this;
        }

        UnlinkWithControllBlock();

        ptr_ = static_cast<T*>(other.ptr_);
        controll_ = other.controll_;
        other.ptr_ = nullptr;
        other.controll_ = nullptr;
        return *this;
    }

    template <class X>
    WeakPtr& operator=(WeakPtr<T>&& other) {
        if (ptr_ == other.ptr_) {
            return *this;
        }

        UnlinkWithControllBlock();

        ptr_ = static_cast<T*>(other.ptr_);
        controll_ = other.controll_;
        other.ptr_ = nullptr;
        other.controll_ = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~WeakPtr() {
        UnlinkWithControllBlock();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        UnlinkWithControllBlock();

        ptr_ = nullptr;
        controll_ = new ControllBlock(0, 1);
    }

    void Swap(WeakPtr& other) {
        std::swap(ptr_, other.ptr_);
        std::swap(controll_, other.controll_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    size_t UseCount() const {
        if (!controll_) {
            return 0;
        } else {
            return controll_->strong;
        }
    }

    bool Expired() const {
        return UseCount() == 0;
    }

    SharedPtr<T> Lock() const {
        return Expired() ? SharedPtr<T>() : SharedPtr<T>(ptr_, controll_);
    }

private:
    void UnlinkWithControllBlock() {
        if (controll_ && controll_->DecreaseWeak()) {
            if (controll_->created_from_make_shared) {
                delete[](reinterpret_cast<char*>(controll_));
            } else {
                delete controll_;
            }
        }
    }

private:
    T* ptr_ = nullptr;
    ControllBlock* controll_ = nullptr;
};
