/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef MU_GLOBAL_ALLOCATOR_H
#define MU_GLOBAL_ALLOCATOR_H

#include <cstdint>
#include <vector>
#include <list>
#include <string>

namespace mu {
#define OBJECT_ALLOC(ClassName) \
public: \
    static ObjectAllocator& allocator() { \
        static ObjectAllocator a(#ClassName, ObjectAllocator::destroyer<ClassName>); \
        return a; \
    } \
    static void* operator new(size_t sz) { return allocator().alloc(sz); } \
    static void operator delete(void* ptr, size_t sz) { allocator().free(ptr, sz); } \
    static void* operator new[](size_t) { return allocator().not_supported("new[]"); } \
    static void* operator new(size_t, void*) { return allocator().not_supported("new(size_t, void*)"); } \
    static void operator delete[](void*, size_t) { allocator().not_supported("delete[]"); } \
private:

class ObjectAllocator
{
public:

    using destroyer_t = void (*)(void*);

    ObjectAllocator(const char* name, destroyer_t dtor);
    ~ObjectAllocator();

    static size_t DEFAULT_BLOCK_SIZE;

    const char* name() const;

    void* alloc(size_t size);
    void free(void* ptr, size_t size);
    void cleanup();

    template<class T>
    static void destroyer(void* ptr)
    {
        T* p = reinterpret_cast<T*>(ptr);
        p->~T();
    }

    void* not_supported(const char* info);

    struct Info
    {
        std::string name;
        size_t chunkSize = 0;
        size_t blockCount = 0;
        size_t totalChunks = 0;
        size_t freeChunks = 0;
        uint64_t allocatedBytes = 0;
    };

    Info dumpInfo() const;
    void dump() const;

private:
    struct Chunk {
        /**
         * When a chunk is free, the `next` contains the
         * address of the next chunk in a list.
         *
         * When it's allocated, this space is used by
         * the user.
         */
        Chunk* next = nullptr;
    };

    struct Block {
        Chunk* begin = nullptr;
        size_t chunkCount = 0;
        size_t chunkSize = 0;
    };

    Block allocateBlock(size_t chunkSize) const;

    const char* m_name = nullptr;
    size_t m_chunkSize = 0;
    destroyer_t m_dtor = nullptr;
    Chunk* m_free = nullptr;
    std::vector<Block> m_blocks;
};

class AllocatorsRegister
{
public:

    static AllocatorsRegister* instance()
    {
        static AllocatorsRegister r;
        return &r;
    }

    void reg(ObjectAllocator* a);
    void unreg(ObjectAllocator* a);

    void cleanupAll();

    void dump();

private:
    std::list<ObjectAllocator*> m_allocators;
};
}

#endif // MU_GLOBAL_ALLOCATOR_H
