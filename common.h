#pragma once

#include <memory>
#include <new>

struct NonCopyable
{
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;

protected:
    NonCopyable()
    { }
};

template<typename T>
struct TypedGrowableBuffer : private NonCopyable
{
    explicit TypedGrowableBuffer(size_t buf_size) : buf_size_(buf_size)
    {
        buf_.reset(new char[buf_size]);
    }

    TypedGrowableBuffer(TypedGrowableBuffer<T>&& rhs) : buf_size_(rhs.buf_size_)
    {
        buf_.reset(rhs.buf_.release());
    }

    inline void* Address()
    {
        return buf_.get();
    }

    inline const T& Data() const
    {
        return *(T*)buf_.get();
    }

    inline size_t Size() const
    {
        return buf_size_;
    }

    void Grow(size_t new_size = 0)
    {
        if (new_size > buf_size_)
        {
            buf_size_ = new_size;
        }
        else
        {
            buf_size_ *= 2;
        }
        
        auto buf = buf_.release();
        buf = (char*)realloc(buf, buf_size_);
        if (!buf)
        {
            throw std::bad_alloc();
        }
        buf_.reset(buf);
    }

private:
    std::unique_ptr<char> buf_;
    size_t buf_size_;
};
