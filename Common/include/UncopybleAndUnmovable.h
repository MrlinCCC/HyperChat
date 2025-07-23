
#pragma once

class UncopybleAndUnmovable
{
protected:
    UncopybleAndUnmovable() = default;
    ~UncopybleAndUnmovable() = default;
    UncopybleAndUnmovable(const UncopybleAndUnmovable &) = delete;
    UncopybleAndUnmovable &operator=(const UncopybleAndUnmovable &) = delete;
    UncopybleAndUnmovable(const UncopybleAndUnmovable &&) = delete;
    UncopybleAndUnmovable &operator=(const UncopybleAndUnmovable &&) = delete;
};