
#pragma once
#include "noncopyable.h"
namespace utils
{
    template<typename T> 
    class unique_array : noncopyable
    {
    private:
        T * px;
        unique_array(unique_array const &);
        unique_array & operator=(unique_array const &);

        typedef unique_array<T> this_type;
    public:
        explicit unique_array( T * p = 0 ) noexcept : px( p )
        {
        }

        ~unique_array() noexcept
        {
            delete[] px;
        }

        void reset(T * p = 0) noexcept
        {
            this_type(p).swap(*this);
        }

        T & operator[](std::ptrdiff_t i) const noexcept
        {
            return px[i];
        }

        T * get() const noexcept
        {
            return px;
        }
    };

    template<typename T> 
    inline void swap(unique_array<T>& a, unique_array<T>& b) noexcept
    {
        a.swap(b);
    }

}