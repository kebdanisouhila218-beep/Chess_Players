#pragma once

class IObserver {
public:
    virtual ~IObserver() = default;
    virtual void onStateChanged() = 0;
};