#pragma once

// Lets a service make several saves succeed or fail together,
// without knowing anything about SQL.
class TransactionManager {
public:
    virtual ~TransactionManager() = default;
    virtual void begin() = 0;
    virtual void commit() = 0;
    virtual void rollback() = 0;
};

// Starts a transaction; rolls it back automatically unless commit() is called.
class TransactionGuard {
public:
    explicit TransactionGuard(TransactionManager& tm) : tm_(tm) { tm_.begin(); }
    ~TransactionGuard() {
        if (!done_) { try { tm_.rollback(); } catch (...) {} }
    }
    TransactionGuard(const TransactionGuard&) = delete;
    TransactionGuard& operator=(const TransactionGuard&) = delete;
    void commit() { tm_.commit(); done_ = true; }
private:
    TransactionManager& tm_;
    bool done_ = false;
};
