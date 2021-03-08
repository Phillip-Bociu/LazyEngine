#pragma once
#include <stdlib.h>
#include <string.h>

namespace lzy
{
    template <typename T>
    struct fvector
    {
        T *_begin;
        T *_end;

        T* begin()
        {
            return _begin;
        }

        T* end ()
        {
            return _end;
        }

        const T *begin() const
        {
            return _begin;
        }

        const T *end() const
        {
            return _end;
        }

        std::size_t size() const
        {
            return _end - _begin;
        }

    };

    namespace fVector
    {
        template <typename T>
        void allocate(fvector<T> *vec, std::size_t count)
        {
            vec->_begin = (T *)malloc(count * sizeof(T));
            vec->_end = vec->_begin + count;
        }

        template <typename T>
        void reallocate(fvector<T> *vec, std::size_t newCount)
        {
            const size_t oldCount = vec->_end - vec->_begin;
            fvector<T> newVec;
            allocate(&newVec, newCount);
            T *pNewBegin = newVec._begin;
            T *pOldBegin = vec->_begin;

            if (oldCount > newCount)
            {
                memcpy(pNewBegin, pOldBegin, newCount * sizeof(T));
            }
            else
                memcpy(pNewBegin, pOldBegin, oldCount * sizeof(T));

            free(vec->_begin);
            *vec = *newVec;
        }

        template <typename T>
        std::size_t size(const fvector<T> *vec)
        {
            return vec->_end - vec->_begin;
        }

    }

}