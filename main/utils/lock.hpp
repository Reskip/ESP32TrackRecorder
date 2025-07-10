#ifndef ESP_RWLOCK_HPP
#define ESP_RWLOCK_HPP

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

class RWlock {
public:
    RWlock() {
        mutex_ = xSemaphoreCreateMutex();
        resource_lock_ = xSemaphoreCreateBinary();
        xSemaphoreGive(resource_lock_); // 初始化为可用状态
        readers_count_ = 0;
    }

    ~RWlock() {
        vSemaphoreDelete(mutex_);
        vSemaphoreDelete(resource_lock_);
    }

    // 禁止拷贝和赋值
    RWlock(const RWlock&) = delete;
    RWlock& operator=(const RWlock&) = delete;

    // 读锁持有者类（RAII）
    class read_guard {
    public:
        explicit read_guard(RWlock& rwlock) : rwlock_(rwlock) {
            rwlock_.lock_read();
        }
        ~read_guard() {
            rwlock_.unlock_read();
        }
    private:
        RWlock& rwlock_;
    };

    // 写锁持有者类（RAII）
    class write_guard {
    public:
        explicit write_guard(RWlock& rwlock) : rwlock_(rwlock) {
            rwlock_.lock_write();
        }
        ~write_guard() {
            rwlock_.unlock_write();
        }
    private:
        RWlock& rwlock_;
    };

    // 获取/释放读锁
    void lock_read() {
        xSemaphoreTake(mutex_, portMAX_DELAY);
        if (readers_count_ == 0) {
            xSemaphoreTake(resource_lock_, portMAX_DELAY);
        }
        readers_count_++;
        xSemaphoreGive(mutex_);
    }

    void unlock_read() {
        xSemaphoreTake(mutex_, portMAX_DELAY);
        readers_count_--;
        if (readers_count_ == 0) {
            xSemaphoreGive(resource_lock_);
        }
        xSemaphoreGive(mutex_);
    }

    // 获取/释放写锁
    void lock_write() {
        xSemaphoreTake(resource_lock_, portMAX_DELAY);
    }

    void unlock_write() {
        xSemaphoreGive(resource_lock_);
    }

private:
    SemaphoreHandle_t mutex_;         // 保护读者计数器
    SemaphoreHandle_t resource_lock_; // 保护共享资源
    int readers_count_;               // 当前读者数量
};

#endif // ESP_RWLOCK_HPP    