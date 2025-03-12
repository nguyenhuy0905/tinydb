/**
 * @file heap.cxx
 * @brief Definitions for declarations in heap.hxx
 *
 * And some helper functions.
 */

#ifdef ENABLE_MODULES
module;
#include "general/offsets.hxx"
#include "general/sizes.hxx"
#include "general/utils.hxx"
#include <cassert>
#ifndef IMPORT_STD
#include <bit>
#include <cmath>
#include <iostream>
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>
#endif
export module tinydb.dbfile.internal.heap;
export import tinydb.dbfile.internal.heap_base;
import tinydb.dbfile.coltype;
import tinydb.dbfile.internal.page;
import tinydb.dbfile.internal.freelist;
#ifdef IMPORT_STD
import std;
#endif // IMPORT_STD
#else
#include "dbfile/internal/freelist.hxx"
#include "dbfile/internal/heap_base.hxx"
#include "dbfile/internal/page_base.hxx"
#include "dbfile/internal/page_meta.hxx"
#include "general/offsets.hxx"
#include "general/sizes.hxx"
#include "general/utils.hxx"
#include <bit>
#include <cassert>
#include <iostream>
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>
#endif // ENABLE_MODULES

#include "dbfile/internal/heap.hxx"

namespace tinydb::dbfile::internal {

void write_ptr_to(const Ptr& t_pos, const Ptr& t_ptr, std::ostream& t_out);

/**
 * @brief Gets the pointer at the specified position.
 *
 * @param t_pos The pointer to the position to read the pointer.
 * @param t_in The stream to read from.
 */
auto read_ptr_from(const Ptr& t_pos, std::istream& t_in) -> Ptr;

/**
 * @brief Writes the data inside the fragment into the stream.
 * This function performs no checking before writing data.
 *
 * @param t_frag The fragment.
 * @param t_out The write-only stream.
 */
void write_frag_to(const Fragment& t_frag, std::ostream& t_out);

/**
 * @brief Reads the fragment at the position pointed to from the stream.
 *
 * @param t_pos Pointer to the position of the fragment.
 * @param t_in The read-only stream.
 * @return The fragment read.
 *   - Exception if there's a read error.
 */
auto read_frag_from(const Ptr& t_pos, std::istream& t_in) -> Fragment;

struct FindFragRetVal {
  Fragment ret_frag;
  Fragment next;
  Fragment prev;
  page_off_t ret_off{};
};

/**
 * @brief Find the first-fit fragment. If the first-fit fragment is the
 * largest fragment in the page, update the max-pair of the page. Similarly
 * for smallest fragment and min-pair.
 *
 * @details Since min_pair and max_pair may be in an indeterminate state
 * before and after calling this function, meaning we can't really call
 * update_min_pair or update_max_pair without an error, we need to take their
 * pairs by reference.
 *
 * This function also formats the returned fragment's extra into the right
 * variant.
 *
 * @param t_size The allocation size.
 * @param is_chained Whether the fragment to be allocated is chained.
 * @param t_max_pair First value must be larger than t_size (plus
 * CHAINED_FRAG_HEADER_SIZE if is_chained is true).
 * @param t_min_pair
 * @param t_meta
 * @param t_io
 * @return
 */
auto find_first_fit_frag(page_off_t t_size, bool is_chained, HeapMeta& t_meta,
                         std::istream& t_io) -> FindFragRetVal;

/**
 * @brief Calculates the difference between the fragment's size and the size
 * requested. If the difference is large enough (larger than
 * FREE_FRAG_HEADER_SIZE), perform a fragmentation, creating a new free
 * fragment.
 *
 * @param t_old_frag The fragment to try break. This fragment will be allocated
 * to the user.
 * @param t_next_frag The next fragment of `t_old_frag`. Can be in an
 * indeterminate state.
 * @param t_old_frag_size The requested size.
 * @param heap_meta The heap meta of the heap page containing `t_old_frag` (and
 * consequently t_next_frag). Taken by reference. In case `t_old_frag` is the
 * first free fragment, update `heap_meta`'s first free.
 * @param t_io The stream to read/write.
 * @return True if a new fragment is created, false otherwise.
 */
auto try_break_frag(Fragment& t_old_frag, Fragment& t_next_frag,
                    page_off_t t_old_frag_size, HeapMeta& heap_meta,
                    std::iostream& t_io) -> bool;

/**
 * @brief Updates all the parameters passed in if needed before writing them
 * into the stream.
 *
 * @param t_heap_meta Needs to be updated if max_pair and min_pair changes.
 * @param t_next_frag Will not be updated. The updating is already done in
 * `try_break_frag.`
 * @param t_max_pair Updated if there's a larger fragment (not happening in
 * `malloc`), or if it's a .
 * @param t_min_pair Updated if there's a smaller fragment.
 * @param t_io The read/write stream.
 */
void update_write_heap_pg(HeapMeta& t_heap_meta, const Fragment& t_next_frag,
                          std::pair<page_off_t, page_off_t>& t_max_pair,
                          std::iostream& t_io);

/**
 * @brief Gets the 2 neighboring free fragments of the fragment passed in.
 *
 * The previous neighbor is the closest free fragment to `t_curr_frag` whose
 * `pos.offset` is smaller than that of `t_curr_frag`.
 * Similarly for next neighbor, except this time `pos.offset` is larger.
 *
 * @param t_curr_frag The current fragment.
 * @param t_in The read stream.
 * @return A pair of `Fragment`s whose:
 * - `first` is the previous neighbor.
 * - `second` is the next neighbor.
 */
auto get_neighboring_free_frags(Fragment& t_curr_frag, HeapMeta& t_meta,
                                std::istream& t_in)
    -> std::pair<std::optional<Fragment>, std::optional<Fragment>>;

/**
 * @brief Try coalesce `t_frag` and its 2 closest neighbors.
 *
 * @param t_frag
 * @param t_in
 */
auto coalesce(Fragment&& t_frag, HeapMeta& t_meta, std::iostream& t_io)
    -> Fragment;

/**
 * @brief Reads from a stream and gets the heap.
 *
 * @param t_in The input stream.
 */
auto read_heap_from(std::istream& t_in) -> Heap {
  t_in.seekg(HEAP_OFF);
  page_ptr_t first_heap{};
  t_in.rdbuf()->sgetn(std::bit_cast<char*>(&first_heap), sizeof(first_heap));
  return Heap{first_heap};
}

void write_ptr_to(const Ptr& t_pos, const Ptr& t_ptr, std::ostream& t_out) {
  t_out.seekp(t_pos.pagenum * SIZEOF_PAGE + t_pos.offset);
  auto& rdbuf{*t_out.rdbuf()};
  rdbuf.sputn(std::bit_cast<const char*>(&t_ptr.pagenum),
              sizeof(t_ptr.pagenum));
  rdbuf.sputn(std::bit_cast<const char*>(&t_ptr.offset), sizeof(t_ptr.offset));
}

auto read_ptr_from(const Ptr& t_pos, std::istream& t_in) -> Ptr {
  t_in.seekg(t_pos.pagenum * SIZEOF_PAGE + t_pos.offset);
  auto& rdbuf{*t_in.rdbuf()};
  Ptr ret{};
  rdbuf.sgetn(std::bit_cast<char*>(&ret.pagenum), sizeof(ret.pagenum));
  rdbuf.sgetn(std::bit_cast<char*>(&ret.offset), sizeof(ret.offset));
  return ret;
}

void write_frag_to(const Fragment& t_frag, std::ostream& t_out) {
  t_out.seekp(t_frag.pos.pagenum * SIZEOF_PAGE + t_frag.pos.offset);
  auto& rdbuf{*t_out.rdbuf()};
  // type
  rdbuf.sputc(
      static_cast<std::underlying_type_t<decltype(t_frag.type)>>(t_frag.type));
  // size
  rdbuf.sputn(std::bit_cast<const char*>(&t_frag.size), sizeof(t_frag.size));
  std::visit(
      overload{[&](const Fragment::FreeFragExtra& t_free_extra) {
                 rdbuf.sputn(std::bit_cast<const char*>(&t_free_extra.next),
                             sizeof(t_free_extra.next));
               },
               [&](const Fragment::UsedFragExtra&) {},
               [&](const Fragment::ChainedFragExtra& t_chain_extra) {
                 write_ptr_to(Ptr{.pagenum = t_frag.pos.pagenum,
                                  .offset = static_cast<page_off_t>(
                                      t_frag.pos.offset + sizeof(t_frag.type) +
                                      sizeof(t_frag.size))},
                              t_chain_extra.next, t_out);
               }},
      t_frag.extra);
}

auto read_frag_from(const Ptr& t_pos, std::istream& t_in) -> Fragment {
  t_in.seekg(t_pos.pagenum * SIZEOF_PAGE + t_pos.offset);
  auto& rdbuf{*t_in.rdbuf()};

  Fragment::FragType type{
      static_cast<std::underlying_type_t<Fragment::FragType>>(rdbuf.sbumpc())};

  page_off_t size{};
  rdbuf.sgetn(std::bit_cast<char*>(&size), sizeof(size));

  Fragment::FragExtra extra{Fragment::UsedFragExtra{}};
  switch (Fragment::FragType{type}) {
    using enum Fragment::FragType;
  case Free: {
    page_off_t next{};
    rdbuf.sgetn(std::bit_cast<char*>(&next), sizeof(next));
    extra = Fragment::FreeFragExtra{.next = next};
    break;
  }
  case Used:
    break;
  case Chained: {
    Ptr next{read_ptr_from(Ptr{.pagenum = t_pos.pagenum,
                               .offset = static_cast<page_off_t>(
                                   t_pos.offset + Fragment::HEADER_SIZE)},
                           t_in)};
    extra = Fragment::ChainedFragExtra{.next = next};
    break;
  }
  default:
    std::unreachable();
  }

  return {.pos{t_pos},
          .extra{extra},
          .size = size,
          .type = Fragment::FragType{type}};
}

void Heap::write_heap_to(std::ostream& t_out) {
  t_out.seekp(HEAP_OFF);
  t_out.rdbuf()->sputn(std::bit_cast<const char*>(&m_first_heap_page),
                       sizeof(m_first_heap_page));
}

[[nodiscard]] auto Heap::malloc(page_off_t t_size, bool is_chained,
                                FreeList& t_fl, std::iostream& t_io)
    -> std::pair<Fragment, page_off_t> {
  // chained fragment is free fragment plus a pointer, so naturally it
  // needs Ptr::SIZE (6) more bytes.
  auto header_size{(is_chained ? Fragment::CHAINED_FRAG_HEADER_SIZE
                               : Fragment::USED_FRAG_HEADER_SIZE)};
  [[maybe_unused]]
  auto actual_size{static_cast<page_off_t>(header_size + t_size)};

  assert(actual_size <= SIZEOF_PAGE - HeapMeta::DEFAULT_FREE_OFF);

  auto heap_meta{find_first_fit_heap_pg(t_size, is_chained, t_fl, t_io)};
  auto [ret_frag, next_frag, prev_frag,
        ret_off]{find_first_fit_frag(t_size, is_chained, heap_meta, t_io)};
  auto max_pair{heap_meta.get_max_pair()};

  try_break_frag(ret_frag, next_frag, t_size, heap_meta, t_io);
  write_frag_to(ret_frag, t_io);
  update_write_heap_pg(heap_meta, next_frag, max_pair, t_io);

  return std::make_pair(ret_frag, ret_off);
}

void Heap::chain(Fragment& t_to_chain, const Fragment& t_next_frag,
                 std::ostream& t_out) {
  assert(std::get_if<Fragment::ChainedFragExtra>(&t_to_chain.extra) != nullptr);
  assert(std::get_if<Fragment::ChainedFragExtra>(&t_next_frag.extra) !=
         nullptr);
  std::get<Fragment::ChainedFragExtra>(t_to_chain.extra).next = t_next_frag.pos;
  write_frag_to(t_to_chain, t_out);
}

void Heap::free(Fragment&& t_frag, [[maybe_unused]] FreeList& t_fl,
                std::iostream& t_io) {
  assert(t_frag.type != Fragment::FragType::Free);
  auto heap_meta{read_from<HeapMeta>(t_frag.pos.pagenum, t_io)};
  if (t_frag.type == Fragment::FragType::Chained) {
    t_frag.size = static_cast<page_off_t>(t_frag.size +
                                          Fragment::CHAINED_FRAG_HEADER_SIZE -
                                          Fragment::FREE_FRAG_HEADER_SIZE);
  } else {
    t_frag.size =
        static_cast<page_off_t>(t_frag.size + Fragment::USED_FRAG_HEADER_SIZE -
                                Fragment::FREE_FRAG_HEADER_SIZE);
  }
  t_frag.type = Fragment::FragType::Free;
  t_frag.extra = Fragment::FreeFragExtra{};
  auto coalesced_frag{coalesce(std::move(t_frag), heap_meta, t_io)};

  bool heap_meta_changed{false};
  auto max_pair{heap_meta.get_max_pair()};
  if (max_pair.first < coalesced_frag.size || max_pair.first == 0) {
    heap_meta.update_max_pair(coalesced_frag.size, coalesced_frag.pos.offset);
    heap_meta_changed = true;
  }
  auto first_free_off{heap_meta.get_first_free_off()};
  if (first_free_off > coalesced_frag.pos.offset ||
      first_free_off == Fragment::NULL_FRAG_PTR) {
    heap_meta.update_first_free(coalesced_frag.pos.offset);
    heap_meta_changed = true;
  }

  if (heap_meta_changed) {
  }
  write_to(heap_meta, t_io);

  write_frag_to(std::move(coalesced_frag), t_io);
}

auto find_first_fit_frag(page_off_t t_size, bool is_chained, HeapMeta& t_meta,
                         std::istream& t_io) -> FindFragRetVal {
  auto header_size{is_chained ? Fragment::CHAINED_FRAG_HEADER_SIZE
                              : Fragment::USED_FRAG_HEADER_SIZE};
  auto search_size{t_size + header_size - Fragment::FREE_FRAG_HEADER_SIZE};
  // first-fit scheme. You know, a database is meant to be read from more
  // than update. If one wants frequent updating, he/she would probably
  // use a giant-hashtable type of database. If it's meant to be more
  // rarely updated, I can afford some fragmentation. Fragmentation is
  // only terrible in the case of "you have a bunch of heap-allocated
  // strings that are all relatively short (say, <1000 bytes) and their
  // sizes vary wildly". For giant strings that take one or more pages to
  // allocate, this isn't an issue really. The number of fragments over
  // the total amount of heap memory allocated is smaller.

  auto pagenum{t_meta.get_pg_num()};
  auto& max_pair{t_meta.get_max_pair()};

  auto ret_frag{read_frag_from(
      Ptr{.pagenum = pagenum, .offset = t_meta.get_first_free_off()}, t_io)};
  Fragment prev_frag{};
  while (ret_frag.size < search_size) {
    assert(std::get_if<Fragment::FreeFragExtra>(&ret_frag.extra) != nullptr);
    assert(ret_frag.get_free_extra().next != Fragment::NULL_FRAG_PTR);

    prev_frag = ret_frag;
    ret_frag = read_frag_from(
        Ptr{.pagenum = pagenum, .offset = ret_frag.get_free_extra().next},
        t_io);
  }
  // from this point, ret_frag's position will not be modified.
  assert(ret_frag.type == Fragment::FragType::Free);
  // set max to some placeholder values, if max happens to
  // point to the same fragment that will be allocated.
  if (max_pair.second == ret_frag.pos.offset) {
    // max_pair = {static_cast<page_off_t>(0), Fragment::NULL_FRAG_PTR};
    t_meta.update_max_pair(static_cast<page_off_t>(0), Fragment::NULL_FRAG_PTR);
  }
  // record the 2 "neighbor" free fragments (neighbor in the sense of
  // "closest together" here) for update later. Of course, only update if
  // there exists those 2.
  Fragment next_frag{prev_frag};
  if (ret_frag.get_free_extra().next != Fragment::NULL_FRAG_PTR) {
    next_frag = read_frag_from(
        Ptr{.pagenum = pagenum, .offset = ret_frag.get_free_extra().next},
        t_io);
  }
  if (!prev_frag.is_invalid()) {
    auto next_off{ret_frag.get_free_extra().next};
    if (next_off != Fragment::NULL_FRAG_PTR) {
      next_frag =
          read_frag_from(Ptr{.pagenum = pagenum, .offset = next_off}, t_io);
    }
  }
  // if the user needs to chain the fragments, they must have another
  // `Chained` fragment ready.
  ret_frag.type = [&]() {
    if (is_chained) {
      ret_frag.extra = Fragment::ChainedFragExtra{NullPtr};
      return Fragment::FragType::Chained;
    }
    ret_frag.extra = Fragment::UsedFragExtra{};
    return Fragment::FragType::Used;
  }();
  ret_frag.size = static_cast<page_off_t>(
      ret_frag.size + Fragment::FREE_FRAG_HEADER_SIZE - header_size);
  return {.ret_frag = ret_frag,
          .next = next_frag,
          .prev = prev_frag,
          .ret_off = header_size};
}

auto Heap::find_first_fit_heap_pg(page_off_t t_size, bool is_chained,
                                  FreeList& t_fl, std::iostream& t_io)
    -> HeapMeta {
  auto header_size{(is_chained) ? Fragment::CHAINED_FRAG_HEADER_SIZE
                                : Fragment::USED_FRAG_HEADER_SIZE};
  auto search_size{t_size + header_size - Fragment::FREE_FRAG_HEADER_SIZE};
  assert(search_size <= SIZEOF_PAGE - HeapMeta::DEFAULT_FREE_OFF -
                            Fragment::FREE_FRAG_HEADER_SIZE);
  auto alloc_new_heap_pg{[&]() {
    auto ret = t_fl.allocate_page<HeapMeta>(t_io);
    // initialize the only fragment, which is 4076 bytes long (including
    // the header stuff).
    Fragment write_to{
        .pos{.pagenum = ret.get_pg_num(), .offset = HeapMeta::DEFAULT_FREE_OFF},
        .extra{Fragment::FreeFragExtra{.next = Fragment::NULL_FRAG_PTR}},
        // When this thing is used, it is converted into
        // an used fragment anyways.
        .size =
            static_cast<page_off_t>(SIZEOF_PAGE - HeapMeta::DEFAULT_FREE_OFF -
                                    Fragment::FREE_FRAG_HEADER_SIZE),
        .type = Fragment::FragType::Free};
    write_frag_to(write_to, t_io);
    return ret;
  }};
  auto heap_meta{[&]() {
    if (m_first_heap_page == NULL_PAGE) {
      auto ret = alloc_new_heap_pg();
      m_first_heap_page = ret.get_pg_num();
      return ret;
    } else {
      return read_from<HeapMeta>(m_first_heap_page, t_io);
    }
  }()};
  // search for the first heap page that has a large enough fragment.
  auto max_pair{heap_meta.get_max_pair()};
  auto pagenum{heap_meta.get_pg_num()};
  {
    // this one is to update the heap page later on
    // this name is terrible. I should have created a struct that has
    // fields size and offset.
    while (max_pair.first < search_size && pagenum != NULL_PAGE) {
      pagenum = heap_meta.get_next_pg();
      if (pagenum == NULL_PAGE) {
        break;
      }
      heap_meta = read_from<HeapMeta>(pagenum, t_io);
      max_pair = heap_meta.get_max_pair();
    }
    // if we don't even have a large enough fragment to use, ask the
    // freelist for a new heap page. Pretty much "inventing" `mmap()`
    // here.
    if (pagenum == NULL_PAGE) {
      auto prev_heap{heap_meta};
      heap_meta = alloc_new_heap_pg();
      pagenum = heap_meta.get_pg_num();
      prev_heap.update_next_pg(pagenum);
      write_to(prev_heap, t_io);
    }
  }
  // return {.heap_pg = heap_meta, .max_pair = max_pair};
  return heap_meta;
}

auto try_break_frag(Fragment& t_old_frag, Fragment& t_next_frag,
                    page_off_t t_old_frag_size, HeapMeta& heap_meta,
                    std::iostream& t_io) -> bool {
  auto header_size{t_old_frag.header_size()};

  // if the returned fragment still has room for another free fragment,
  // break it down.
  auto new_frag_size{[&]() {
    auto ret =
        t_old_frag.size - t_old_frag_size - Fragment::FREE_FRAG_HEADER_SIZE;
    if (ret > 0) {
      return static_cast<page_off_t>(ret);
    }
    return static_cast<page_off_t>(0);
  }()};
  if (new_frag_size > 0) {
    auto new_frag_off{static_cast<page_off_t>(t_old_frag.pos.offset +
                                              header_size + t_old_frag_size)};
    Fragment new_frag{
        .pos{.pagenum = t_old_frag.pos.pagenum, .offset = new_frag_off},
        .extra{Fragment::FreeFragExtra{.next = Fragment::NULL_FRAG_PTR}},
        // basically, we only care about the size of an used fragment.
        .size = static_cast<page_off_t>(new_frag_size),
        .type = Fragment::FragType::Free};
    t_old_frag.size = t_old_frag_size;
    // Update the neighboring fragment, particularly the next fragment.
    if (!t_next_frag.is_invalid()) {
      assert(std::get_if<Fragment::FreeFragExtra>(&t_next_frag.extra) !=
             nullptr);
      new_frag.get_free_extra().next = t_next_frag.pos.offset;
    } else {
      t_next_frag = new_frag;
    }
    // then the heap page
    if (heap_meta.get_first_free_off() == t_old_frag.pos.offset) {
      heap_meta.update_first_free(new_frag_off);
    }
    // TODO: move the writes back to malloc.

    write_frag_to(new_frag, t_io);
    return true;
  }
  if (heap_meta.get_first_free_off() == t_old_frag.pos.offset) {
    heap_meta.update_first_free(Fragment::NULL_FRAG_PTR);
  }
  return false;
}

void update_write_heap_pg(HeapMeta& heap_meta, const Fragment& next_frag,
                          std::pair<page_off_t, page_off_t>& max_pair,
                          std::iostream& t_io) {
  auto pagenum{heap_meta.get_pg_num()};
  auto curr_update_frag{next_frag};
  // Force update. I'm a bit paranoid.
  if (max_pair.second == curr_update_frag.pos.offset) {
    // if first remains 0 after the for loop, it's basically the same as
    // "this page is out of memory".
    max_pair = {static_cast<page_off_t>(0), static_cast<page_off_t>(0)};
  }
  // Traverse the entire page to see which free fragments remaining are
  // the biggest and smallest.
  for (auto o{heap_meta.get_first_free_off()}; o != Fragment::NULL_FRAG_PTR;
       o = curr_update_frag.get_free_extra().next) {
    assert(std::get_if<Fragment::FreeFragExtra>(&curr_update_frag.extra) !=
           nullptr);
    if (max_pair.first < curr_update_frag.size) {
      max_pair = {curr_update_frag.size, curr_update_frag.pos.offset};
    }
    curr_update_frag =
        read_frag_from(Ptr{.pagenum = pagenum, .offset = o}, t_io);
  }
  // if min_pair is not updated at all, meaning there is no fragment left.
  // first = 0 is basically the same as "this page is out of memory".
  if (max_pair.first == 0) {
    max_pair.second = 0;
  }

  heap_meta.update_max_pair(max_pair.first, max_pair.second);
  write_to(heap_meta, t_io);
}

auto get_neighboring_free_frags(Fragment& t_curr_frag, HeapMeta& heap_meta,
                                std::istream& t_in)
    -> std::pair<std::optional<Fragment>, std::optional<Fragment>> {
  assert(t_curr_frag.type == Fragment::FragType::Free);
  // this disgusting piece of template means "the return type of this
  // function".
  // Also, ret is declared here to abuse NRVO. Of course, in the debug build
  // with no optimization whatsoever, copy assignment will probably be called
  // anyways.
  std::invoke_result_t<decltype(&get_neighboring_free_frags), Fragment&,
                       HeapMeta&, std::istream&>
      ret;

  // Fuck C++, where is my labeled return?
  [&]() {
    // auto heap_meta{read_from<HeapMeta>(t_curr_frag.pos.pagenum, t_in)};
    auto first_free{heap_meta.get_first_free_off()};
    if (first_free == Fragment::NULL_FRAG_PTR) {
      // return {std::nullopt, std::nullopt};
      return;
    }

    Fragment prev{read_frag_from(
        {.pagenum = t_curr_frag.pos.pagenum, .offset = first_free}, t_in)};
    assert(prev.type == Fragment::FragType::Free);
    if (prev.pos.offset > t_curr_frag.pos.offset) {
      // then `prev` should have been `next`.
      // return {std::nullopt, prev};
      t_curr_frag.get_free_extra().next = prev.pos.offset;
      ret = {std::nullopt, prev};
      return;
    }
    if (prev.get_free_extra().next == Fragment::NULL_FRAG_PTR) {
      // there's nothing "next".
      if (prev.pos.offset == t_curr_frag.pos.offset) {
        // return {std::nullopt, std::nullopt};
        ret = {std::nullopt, std::nullopt};
        return;
      }
      // return {prev, std::nullopt};
      prev.get_free_extra().next = t_curr_frag.pos.offset;
      ret = {prev, std::nullopt};
      return;
    }

    // a potential "next"
    Fragment next{read_frag_from({.pagenum = t_curr_frag.pos.pagenum,
                                  .offset = prev.get_free_extra().next},
                                 t_in)};
    assert(next.type == Fragment::FragType::Free);
    while (next.pos.offset <= t_curr_frag.pos.offset) {
      assert(next.type == Fragment::FragType::Free);
      if (next.get_free_extra().next == Fragment::NULL_FRAG_PTR) {
        // at this point, `next` is still the previous neighbor of
        // `t_curr_frag`
        // return {next, std::nullopt};
        if (prev.get_free_extra().next == t_curr_frag.pos.offset) {
          prev.get_free_extra().next = t_curr_frag.pos.offset;
          ret = {prev, std::nullopt};
          return;
        }
        next.get_free_extra().next = t_curr_frag.pos.offset;
        ret = {next, std::nullopt};
        return;
      }
      if (next.pos.offset < t_curr_frag.pos.offset) {
        prev = next;
      }
      next = read_frag_from({.pagenum = t_curr_frag.pos.pagenum,
                             .offset = next.get_free_extra().next},
                            t_in);
    }
    prev.get_free_extra().next = t_curr_frag.pos.offset;
    t_curr_frag.get_free_extra().next = next.pos.offset;
    ret = {prev, next};
  }();

  return ret;
}

auto coalesce(Fragment&& t_frag, HeapMeta& t_meta, std::iostream& t_io)
    -> Fragment {
  assert(t_frag.type == Fragment::FragType::Free);
  Fragment ret;

  [&]() {
    auto [prev_frag,
          next_frag]{get_neighboring_free_frags(t_frag, t_meta, t_io)};
    if (!prev_frag && !next_frag) {
      ret = std::move(t_frag);
      return;
    }

    // if lhs is left next to of rhs, returns true, otherwise false.
    //
    // Eg, lhs is a free fragment size 10 (which makes the fragment 15 bytes)
    // large. Then if rhs's offset is exactly 15 more than that of lhs, the
    // 2 are next to each other.
    static constexpr auto is_next_to{
        [](const Fragment& lhs, const Fragment& rhs) noexcept {
          auto lhs_tail = lhs.pos.offset + lhs.header_size() + lhs.size;
          // print debugging, I know. But it's helpful when you have tracked
          // down that one troublesome function.
          //
          // std::println("Fragment (off: {}, header: {}, size: {}) neighbors "
          //              "(off: {}, header: {}, size: {}): {}",
          //              lhs.pos.offset, lhs.header_size(), lhs.size,
          //              rhs.pos.offset, rhs.header_size(), rhs.size,
          //              (lhs_tail == rhs.pos.offset) ? "true" : "false");
          return lhs_tail == rhs.pos.offset;
        }};
    // Merges 2 fragments. Assumption: `is_next_to(lhs, rhs)` returns `true`.
    //
    // The move is just to explicitly tell "don't touch these variables again".
    static constexpr auto merge{[](Fragment&& lhs,
                                   Fragment&& rhs) noexcept -> Fragment {
      // damn, you don't need to capture static lambdas?
      // assert(is_next_to(lhs, rhs));
      return Fragment{
          .pos{lhs.pos},
          .extra{Fragment::FreeFragExtra{
              .next = rhs.get_free_extra().next,
          }},
          .size = static_cast<page_off_t>(
              std::move(lhs).size + rhs.header_size() + std::move(rhs).size),
          .type = Fragment::FragType::Free,
      };
    }};

    bool prev_merged{false};
    auto left_merge = [&]() {
      // merge with prev_frag if prev_frag exists, and return the newly created
      // fragment. Otherwise, just return t_frag.
      auto ret_opt =
          prev_frag
              .transform([&](Fragment& prev) noexcept {
                // we only care about t_frag.
                if (!is_next_to(prev, t_frag)) {
                  return t_frag;
                }
                if (t_meta.get_first_free_off() > prev.pos.offset ||
                    t_meta.get_first_free_off() == Fragment::NULL_FRAG_PTR) {
                  t_meta.update_first_free(prev.pos.offset);
                }
                prev_merged = true;
                return merge(std::move(prev), std::move(t_frag));
              })
              .or_else([&t_frag]() noexcept { return std::optional{t_frag}; });
      assert(ret_opt.has_value());
      return *ret_opt;
    }();

    ret = [&]() {
      // merge left_merge with next_frag if next_frag exists, or just return
      // left_merge.
      // If prev_frag has a value but cannot be merged (due to not being
      // neighbors) but t_frag and next_frag can still be merged, update
      // prev_frag's next pointer.
      auto ret_opt = next_frag
                         .transform([&](Fragment& next) noexcept {
                           if (!is_next_to(left_merge, next)) {
                             return left_merge;
                           }
                           if (!prev_merged && prev_frag.has_value()) {
                             prev_frag->get_free_extra().next =
                                 left_merge.pos.offset;
                             write_frag_to(*prev_frag, t_io);
                           }
                           return merge(std::move(left_merge), std::move(next));
                         })
                         .or_else([&left_merge]() noexcept {
                           return std::optional{left_merge};
                         });
      assert(ret_opt.has_value());
      return ret_opt.value();
    }();
  }();

  // std::println("Resulting fragment: (page: {}, offset: {}, header: {}, size:
  // {})",
  //              ret.pos.pagenum, ret.pos.offset, ret.header_size(), ret.size);
  return ret;
}

} // namespace tinydb::dbfile::internal
