#ifndef ENABLE_MODULE
#include "dbfile/internal/freelist.hxx"
#include "dbfile/internal/heap.hxx"
#include "general/sizes.hxx"
#include <gtest/gtest.h>
#include <print>
#include <sstream>
#endif

TEST(heap, init) {
    using namespace tinydb;
    using namespace tinydb::dbfile;
    using namespace tinydb::dbfile::internal;

    static constexpr page_ptr_t numpages = 4;
    static constexpr page_ptr_t first_pg = 1;
    std::stringstream test_stream{std::string(SIZEOF_PAGE * numpages, '\0')};
    test_stream.exceptions(std::stringstream::failbit);
    auto fl = FreeList::default_init(first_pg, test_stream);
    // fl.do_write_to(test_stream);
    Heap test_heap{0};
    // NOLINTBEGIN(*magic-number*)
    std::pair<Heap::Fragment, page_off_t> frag1{};
    frag1 = test_heap.malloc(4020, true, fl, test_stream);

    // the print functions are there so that the fragments aren't optimized out.
    std::println("frag1 - pagenum: {} - offset: {} - size: {}",
                 frag1.first.pos.pagenum, frag1.first.pos.offset,
                 frag1.first.size);
    auto frag2 = test_heap.malloc(50, false, fl, test_stream);
    std::println("frag2 - pagenum: {} - offset: {} - size: {}",
                 frag2.first.pos.pagenum, frag2.first.pos.offset,
                 frag2.first.size);
    [[maybe_unused]]
    auto rando_frag = test_heap.malloc(10, false, fl, test_stream);
    // to force another free page allocation
    auto frag3 = test_heap.malloc(4000, false, fl, test_stream);
    std::println("frag3 - pagenum: {} - offset: {} - size: {}",
                 frag3.first.pos.pagenum, frag3.first.pos.offset,
                 frag3.first.size);
    auto frag4 = test_heap.malloc(30, false, fl, test_stream);
    std::println("frag4 - pagenum: {} - offset: {} - size: {}",
                 frag4.first.pos.pagenum, frag4.first.pos.offset,
                 frag4.first.size);
    auto frag5 = test_heap.malloc(30, false, fl, test_stream);
    std::println("frag5 - pagenum: {} - offset: {} - size: {}",
                 frag5.first.pos.pagenum, frag5.first.pos.offset,
                 frag5.first.size);
    // NOLINTEND(*magic-number*)
}

// there used to be tests here.
// But I decided to rewrite Heap from the ground up.
// Again, yes.
// So, not until I at least finish writing Heap::malloc.
