#include "offsets.hxx"
#include <bit>
#ifndef ENABLE_MODULE
#include "dbfile/internal/freelist.hxx"
#include "dbfile/internal/heap.hxx"
#include "general/sizes.hxx"
#include <array>
#include <gtest/gtest.h>
#include <print>
#include <ranges>
#include <sstream>
#include <utility>
#endif // !ENABLE_MODULE

TEST(default_heap, test) {
    using namespace tinydb;
    using namespace tinydb::dbfile;
    using namespace tinydb::dbfile::internal;

    // these are still invalid pointers, but I just need something good enough
    // to test out.
    Heap default_init_heap{};
    auto bins = default_init_heap.get_bins();
    for (const auto& ptr : bins) {
        // AFAIK, std::array is always default-initialized if no argument is
        // specified. Default-initialized for Ptr should be all zeros.
        ASSERT_EQ(ptr, NullPtr);
        // way too careful here, but, to make sure operator== doesn't change the
        // values.
        ASSERT_EQ(ptr, NullPtr);
    }
    std::stringstream test_stream{std::string(SIZEOF_PAGE * 3, '\0')};
    write_heap_to(default_init_heap, test_stream);
    auto read_heap = read_heap_from(test_stream);
    auto read_bins = read_heap.get_bins();
    // TLDR: basically compares bin[0] to read_bins[0], bin[1] to read_bins[1]
    // and so on.
    for (const auto [lhs, rhs] :
         // so, if the cast isn't here, I suspect that this thing gives me an
         // overflow. ASan reported some memory bug.
         std::views::iota(static_cast<uint8_t>(0), read_bins.size()) |
             std::views::transform([&](auto num) {
                 return std::pair(bins[num], read_bins[num]);
             })) {
        ASSERT_EQ(lhs, rhs);
        // way too careful here, but, to make sure operator== doesn't change the
        // values.
        ASSERT_EQ(lhs, NullPtr);
        ASSERT_EQ(rhs, NullPtr);
    }
}

TEST(initialized_heap, test) {
    using namespace tinydb;
    using namespace tinydb::dbfile;
    using namespace tinydb::dbfile::internal;

    // just some test stuff
    std::array<Ptr, Heap::SIZEOF_BIN> inits{
        Ptr{.pagenum = 0, .offset = 0},
        Ptr{.pagenum = 1, .offset = 4},
        Ptr{.pagenum = 0, .offset = 0},
        Ptr{.pagenum = 2, .offset = 3},
        Ptr{.pagenum = 0, .offset = 0},
        Ptr{.pagenum = 3, .offset = 2},
        Ptr{.pagenum = 0, .offset = 0},
        Ptr{.pagenum = 4, .offset = 1},
        Ptr{.pagenum = 4, .offset = 4},
    };
    Heap init_heap{inits.begin(), inits.end()};
    auto bins = init_heap.get_bins();

    for (auto [inits_ptr, bins_ptr] :
         std::views::iota(static_cast<uint8_t>(0), bins.size()) |
             std::views::transform([&](auto num) {
                 return std::make_pair(inits.at(num), bins[num]);
             })) {
        ASSERT_EQ(inits_ptr, bins_ptr);
        // way too careful here, but, to make sure operator== doesn't change the
        // values.
        ASSERT_EQ(inits_ptr, bins_ptr);
    }

    std::stringstream test_stream{std::string(SIZEOF_PAGE * 3, '\0')};
    write_heap_to(init_heap, test_stream);
    auto read_heap = read_heap_from(test_stream);
    auto read_bins = read_heap.get_bins();
    // TLDR: basically compares bin[0] to read_bins[0], bin[1] to read_bins[1]
    // and so on.
    for (const auto [lhs, rhs, init] :
         // so, if the cast isn't here, I suspect that this thing gives me an
         // overflow. ASan reported some memory bug.
         std::views::iota(static_cast<uint8_t>(0), read_bins.size()) |
             std::views::transform([&](auto num) {
                 return std::make_tuple(bins[num], read_bins[num],
                                        inits.at(num));
             })) {
        ASSERT_EQ(lhs, rhs);
        // way too careful here, but, to make sure operator== doesn't change the
        // values.
        ASSERT_EQ(lhs, init);
        ASSERT_EQ(rhs, init);
    }
}

TEST(malloc, test) {
    using namespace tinydb;
    using namespace tinydb::dbfile;
    using namespace tinydb::dbfile::internal;

    std::array<Ptr, Heap::SIZEOF_BIN> inits{
        Ptr{.pagenum = 0, .offset = 0},
        Ptr{.pagenum = 1, .offset = 4},
        Ptr{.pagenum = 0, .offset = 0},
        Ptr{.pagenum = 2, .offset = 3},
        Ptr{.pagenum = 0, .offset = 0},
        Ptr{.pagenum = 3, .offset = 2},
        NullPtr,
        NullPtr,
        NullPtr,
    };
    Heap init_heap{inits.begin(), inits.end()};
    // lazy patch to record database file size.
    const uint32_t dbfile_size = 2;
    std::stringstream test_stream{std::string(SIZEOF_PAGE * 4, '\0')};
    test_stream.seekp(DBFILE_SIZE_OFF);
    test_stream.rdbuf()->sputn(std::bit_cast<const char*>(&dbfile_size),
                               sizeof(dbfile_size));
    auto fl = FreeList::default_init(1, test_stream);
    write_heap_to(init_heap, test_stream);
    auto ptr = init_heap.malloc(SIZEOF_PAGE / 4 + 1, fl, test_stream);
    auto ptr2 = init_heap.malloc((SIZEOF_PAGE / 4) / 2 + 1, fl, test_stream);
    // The print function hammers the compile speed.
    std::println("ptr: pagenum: {}, offset: {}", ptr.pagenum, ptr.offset);
    std::println("ptr2: pagenum: {}, offset: {}", ptr2.pagenum, ptr2.offset);
}
