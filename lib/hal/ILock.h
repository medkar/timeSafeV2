#pragma once
namespace tsafe {
class ILock {
public:
    virtual ~ILock() {}
    virtual void forceLock() = 0; // état par défaut, appelé au boot
    virtual void unlock() = 0;    // retire le pêne
};
} // namespace tsafe
