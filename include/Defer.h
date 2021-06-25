//
// Created by carl on 20-4-3.
//

#pragma once

#include <functional>
#include <utility>

#define L_DEFER_COMBINE1(X, Y) X##Y
#define L_DEFER_COMBINE(X, Y) L_DEFER_COMBINE1(X, Y)

#define OnScopeExit ::sol2::Defer L_DEFER_COMBINE(_defer_, __LINE__) = [&]() -> void

namespace sol2 {

class Defer {
public:
    template <typename Callable>
    Defer(Callable&& defer) // NOLINT(google-explicit-constructor,hicpp-explicit-conversions)
        : defered_(std::forward<Callable>(defer))
    {
    }

    Defer()
        : defered_([]() {})
    {
    }
    Defer(Defer&& d) noexcept
    {
        defered_ = d.defered_;
        d.defered_ = []() {};
    }
    Defer(const Defer&& d) noexcept = delete;
    Defer& operator=(Defer&& d) noexcept
    {
        defered_ = d.defered_;
        d.defered_ = []() {};
        return *this;
    }

    ~Defer() { defered_(); }

    Defer(const Defer&) = delete;
    Defer& operator=(const Defer&) = delete;

    void Cancel()
    {
        defered_ = []() {};
    }

private:
    std::function<void()> defered_;
};

}