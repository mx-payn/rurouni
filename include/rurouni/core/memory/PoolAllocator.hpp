#ifndef RUROUNI_POOLALLOCATOR_HPP
#define RUROUNI_POOLALLOCATOR_HPP

#include "rr_pch.h"

namespace Rurouni
{
    /*!
     * \brief An equally sized partition of a memory allocation
     *
     * A PoolChunk is a single part of the allocation space, that is equal in size to all
     * other chunks in that allocation space. More simple: The entire allocation space
     * of the allocator gets divided into chunks of equal size.
     * The chunk-union holds: Either the class object of type T or a Pointer to the next
     * chunk. If a chunk is allocated, it writes the class object and is freed from
     * the chunk-chain.
     *
     * @tparam T    the class of type T the chunk holds, or the type-parameter for the
     *              next chunk
     */
    template<typename T>
    union PoolChunk
    {
        T value;                        //!< The actual constructed object the chunk holds
        PoolChunk<T>* nextPoolChunk;    //!< pointer to the next chunk in the chunk-chain

        PoolChunk()  = default;
        ~PoolChunk() = default;
    };

    /*!
     * Allocates memory for given amount of chunks from the parent allocator and divides it into
     * equally sized chunks, which then again can be allocated by either the game or
     * another allocator (with the PoolAllocator being its parent-allocator.
     * Chunks are managed in a chain like structure, where free chunks point to each
     * other in sequence. By using a union PoolChunk, allocated chunks are no longer
     * part of the chain until the are freed and reinserted into the chain at the
     * head-pointer.
     * A chunk is always as big to fit an object of type T inside it.
     *
     * @tparam T            The classes typename to allocate memory and construct in place.
     * @tparam Allocator    The parent allocator to allocate the available memory from.
     */
    template<typename T, typename Allocator = std::allocator<PoolChunk<T>>>
    class PoolAllocator
    {
    public:

        /*!
         * Allocates memory from parent allocator and divides it equally in sizeof(T) portions.
         * chunks are pushed into array m_data by pointer. The last chunk holds the nullpointer,
         * so NO loop of chunks.
         * Castings to other types are obviously mute, ergo explicit.
         *
         * @param size              amount of chunks.
         * @param parentAllocator   the allocator to allocate base memory from.
         */
        explicit PoolAllocator(size_t     size            = POOL_ALLOCATOR_DEFAULT_SIZE,
                               Allocator* parentAllocator = nullptr)
                : m_size(size), m_parentAllocator(parentAllocator)
        {
            if (parentAllocator == nullptr)     // Use std allocator
                m_parentAllocator = new Allocator();

            // allocate "chunkable" memory of given size
            m_data = m_parentAllocator->allocate(m_size);
            m_head = m_data;
            m_poolBegin = m_data;
            m_poolEnd = &m_data[m_size - 1];

            // iterate all chunks and set next chunk (i+=sizeof(T) in pointer arrithmic! so this is chunking!)
            for(size_t i = 0; i < m_size - 1; i++)
            {
                m_data[i].nextPoolChunk = std::addressof(m_data[i + 1]);
            }
            // last chunk has no next chunk
            m_data[m_size - 1].nextPoolChunk = nullptr;
        }

        ~PoolAllocator()
        {
            RR_ASSERT_EQ(m_allocateCount, 0, "all PoolChunks have to be deallocated before Allocator destruction")

            m_parentAllocator->deallocate(m_data, m_size);
            if (m_parentAllocator != nullptr)   //TODO: This check is always true? parent always set in constructor?
                delete m_parentAllocator;

            m_data = nullptr;
            m_head = nullptr;
        }

        /*!
         * allocates a chunk from m_data and returns an in place constructed object pointer of type T.
         *
         * @tparam arguments    argument types of the T objects constructor.
         * @param args          arguments to be forwarded to the T objects constructor.
         * @return              a pointer on an in place constructed object of type T.
         */
        template<typename... arguments>
        T* allocate(arguments&&... args)
        {
            RR_ASSERT_NOT_EQ(m_head, nullptr, "there are no more chunks available")

            PoolChunk<T>* poolChunk = m_head;   // allocate chunk
            m_head = m_head->nextPoolChunk;     // move head
            T* retVal = new (std::addressof(poolChunk->value)) T(std::forward<arguments>(args)...); // construct T
            m_allocateCount++;

            return retVal;
        }

        /*!
         * deallocates the chunk and puts it back into the list of free chunks.
         * checks beforehand whether the passed chunk was actually allocated by this allocator.
         * on deallocation, a passed object gets cast back into a chunk (as it originally was).
         *
         * @param data  the object to be deallocated.
         */
        void deallocate(T* data)
        {

            data->~T();     // deconstruct
            // this reinterpret_cast is in fact allowed and valid, because we can guarantee that data was
            // a PoolChunk before construction. Also the c++ standard states, that our PoolChunk will have
            // the same address whatever we construct and cast on it. Therefore the Chunk turns into a constructed
            // object on alloc and is casted back to PoolChunk on dealloc, all on the same address.
            auto* poolChunk = reinterpret_cast<PoolChunk<T>*>(data);

            // checking whether the address of data is inside the total pool address space.
            // if not, data was not allocated by this allocator and should NEVER be freed here.
            RR_ASSERT_TRUE((poolChunk >= m_poolBegin) && (poolChunk <= m_poolEnd),
                           "address of data has to be in range of the pool address space")

                           poolChunk->nextPoolChunk = m_head;
            m_head = poolChunk;
            m_allocateCount--;
        }

        // Rule of five:
        // Because the presence of a user-defined destructor, copy-constructor, or copy-assignment operator
        // prevents implicit definition of the move constructor and the move assignment operator, any class
        // for which move semantics are desirable, has to declare all five special member functions.
        //
        // It may be possible to allocate pools on an defragment-able allocator, so we should also provide
        // the move constructors as stated in the Rule of five. Else the Rule of three (only copy constr)
        // would have been sufficient.
        // TODO: implement those!
        PoolAllocator(const PoolAllocator& other)               = delete; // Copy Constructor
        PoolAllocator(PoolAllocator&& other)                    = delete; // Move Constructor
        PoolAllocator& operator=(const PoolAllocator& other)    = delete; // Copy assignment Constructor
        PoolAllocator& operator=(PoolAllocator&& other)         = delete; // Move assignment Constructor

    private:
//  --== Member Variables ==--
        size_t        m_size            = 0;           //!< the amount of chunks to allocate
        size_t        m_allocateCount   = 0;           //!< amount of current active allocations
        PoolChunk<T>* m_data            = nullptr;     //!< array of all chunks
        PoolChunk<T>* m_head            = nullptr;     //!< pointer to the next chunk to allocate
        PoolChunk<T>* m_poolBegin       = nullptr;     //!< beginning address of allocated memory (first element AT INITIALIZATION)
        PoolChunk<T>* m_poolEnd         = nullptr;     //!< ending address of allocated memory (last element AT INITIALIZATION)
        Allocator*    m_parentAllocator = nullptr;     //!< the allocator to allocate memory from (Allocateception)

        static const size_t POOL_ALLOCATOR_DEFAULT_SIZE = 1024; //!< default size of chunks
    };
}

#endif //RUROUNI_POOLALLOCATOR_HPP
