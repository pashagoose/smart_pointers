#pragma once

#include <exception>

class BadWeakPtr : public std::exception {};

template <typename T>
class SharedPtr;

template <typename T>
class WeakPtr;

template <typename T>
class EnableSharedFromThis;

class EnableSharedFromThisBase;

struct BaseAnyPtr {
    virtual void Delete() = 0;
    virtual void Destruct() = 0;
    virtual void Release() = 0;
    virtual ~BaseAnyPtr() = default;
};

template <class T>
struct AnyPtr final : public BaseAnyPtr {

    template <class X>
    AnyPtr(X* pointer) noexcept : ptr(static_cast<T*>(pointer)) {
    }

    void Delete() override {
        delete ptr;
    }

    void Destruct() override {
        ptr->~T();
    }

    void Release() override {
        ptr = nullptr;
    }

    ~AnyPtr() override = default;

    T* ptr;
};

struct ControllBlock {
    std::size_t strong = 0;
    std::size_t weak = 0;
    BaseAnyPtr* ptr = nullptr;
    bool created_from_make_shared = false;

    ControllBlock(std::size_t st, std::size_t we) noexcept : strong(st), weak(we) {
    }

    template <class X>
    ControllBlock(X* ptr) noexcept
        : strong(1), weak(0), ptr(new AnyPtr<X>(ptr)) {  // this ctor is only for SharedPtr
    }

    ControllBlock(BaseAnyPtr* pointer) noexcept
        : ptr(pointer),
          created_from_make_shared(true) {  // this ctor is for SharedPtr created via MakeShared
    }

    bool DecreaseStrong() {
        /* Return true if we can destruct controll block, otherwise - false */
        if (strong == 1) {
            if (ptr && !created_from_make_shared) {
                ptr->Delete();
                delete ptr;
            } else if (ptr && created_from_make_shared) {
                ptr->Destruct();
                ptr->~BaseAnyPtr();
            }
        }
        --strong;
        return (strong == 0 && weak == 0);
    }

    void IncreaseStrong() {
        ++strong;
    }

    void IncreaseWeak() {
        ++weak;
    }

    bool DecreaseWeak() {
        /* Returns true if we can destruct controll block, otherwise - false */
        --weak;
        return (strong == 0 && weak == 0);
    }

    ~ControllBlock() = default;
};
