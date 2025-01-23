#ifndef ENABLE_MODULE
#include "dbfile/internal/heap.hxx"
#include "general/sizes.hxx"
#include <array>
#include <gtest/gtest.h>
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

    std::array<Ptr, Heap::SIZEOF_BIN> inits{
        Ptr{.pagenum = 0, .offset = 0}, Ptr{.pagenum = 1, .offset = 4},
        Ptr{.pagenum = 0, .offset = 0}, Ptr{.pagenum = 2, .offset = 3},
        Ptr{.pagenum = 0, .offset = 0}, Ptr{.pagenum = 3, .offset = 2},
        Ptr{.pagenum = 0, .offset = 0}, Ptr{.pagenum = 4, .offset = 1},
    };
    Heap init_heap{inits.begin(), inits.end()};
    auto bins = init_heap.get_bins();

    for (auto [inits_ptr, bins_ptr] :
         std::views::iota(static_cast<uint8_t>(0), bins.size()) |
             std::views::transform([&](auto num) {
                 return std::make_pair(inits.at(num), bins[num]);
             })) {
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
                 return std::make_tuple(bins[num], read_bins[num], inits.at(num));
             })) {
        ASSERT_EQ(lhs, rhs);
        // way too careful here, but, to make sure operator== doesn't change the
        // values.
        ASSERT_EQ(lhs, init);
        ASSERT_EQ(rhs, init);
    }
}
