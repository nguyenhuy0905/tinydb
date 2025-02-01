module;
#include "general/sizes.hxx"
#include <cassert>
#ifndef IMPORT_STD
#include <bit>
#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <print>
#include <utility>
#endif
export module tinydb.dbfile.internal.page:serialize;
import :base;
import :meta;
#ifdef IMPORT_STD
import std;
#endif

export namespace tinydb::dbfile::internal {

/**
 * @brief Specialize this method for each serializable type. This includes page
 * metadata or the actual data of a page.
 * returned must be the exact same as the one used to call write_to.
 *
 * @tparam T
 * @param t_pg_num
 * @param t_in
 * @return
 */
template <typename T>
auto read_from(page_ptr_t t_pg_num, std::istream& t_in) -> T;

void write_to(const FreePageMeta& t_meta, std::ostream& t_out) {
    t_out.seekp(t_meta.get_pg_num() * SIZEOF_PAGE);
    t_out.rdbuf()->sputc(static_cast<pt_num_t>(PageType::Free));

    auto next_pg = t_meta.get_next_pg();
    t_out.rdbuf()->sputn(std::bit_cast<const char*>(&next_pg), sizeof(next_pg));
    std::println("Write free page: (pagenum: {}, next: {})",
                 t_meta.get_pg_num(), next_pg);
}

template <>
auto read_from<FreePageMeta>(page_ptr_t t_pg_num, std::istream& t_in)
    -> FreePageMeta {
    t_in.seekg(t_pg_num * SIZEOF_PAGE);
    [[maybe_unused]]
    auto pagetype = t_in.rdbuf()->sbumpc();
    assert(pagetype == static_cast<pt_num_t>(PageType::Free));
    // TODO: think of a new checking mechanism

    // auto pagetype = static_cast<uint8_t>(t_in.rdbuf()->sbumpc());
    // if (pagetype != static_cast<pt_num>(PageType::Free)) {
    //     // I should return something more meaningful here.
    //     return;
    // }

    page_ptr_t next_pg{0};
    t_in.rdbuf()->sgetn(std::bit_cast<char*>(&next_pg), sizeof(next_pg));
    std::println("Read free page: (pagenum: {}, next: {})", t_pg_num, next_pg);
    return {t_pg_num, next_pg};
}

void write_to(const BTreeLeafMeta& t_meta, std::ostream& t_out) {
    t_out.seekp(t_meta.get_pg_num() * SIZEOF_PAGE);
    t_out.rdbuf()->sputc(static_cast<pt_num_t>(PageType::BTreeLeaf));
    auto nrows = t_meta.get_n_rows();
    t_out.rdbuf()->sputn(std::bit_cast<const char*>(&nrows), sizeof(nrows));
    auto first_free = t_meta.get_first_free();
    t_out.rdbuf()->sputn(std::bit_cast<const char*>(&first_free),
                         sizeof(first_free));
}

template <>
auto read_from<BTreeLeafMeta>(page_ptr_t t_pg_num, std::istream& t_in)
    -> BTreeLeafMeta {
    t_in.seekg(t_pg_num * SIZEOF_PAGE);

    [[maybe_unused]]
    auto pagetype = t_in.rdbuf()->sbumpc();
    assert(pagetype == static_cast<pt_num_t>(PageType::BTreeLeaf));
    // if (pagetype != static_cast<pt_num>(PageType::BTreeLeaf)) {
    //     // I should return something more meaningful here.
    //     return;
    // }

    uint16_t nrows{0};
    t_in.rdbuf()->sputn(std::bit_cast<char*>(&nrows), sizeof(nrows));
    uint16_t first_free{0};
    t_in.rdbuf()->sputn(std::bit_cast<char*>(&first_free), sizeof(first_free));
    return {t_pg_num, nrows, first_free};
}

void write_to(const BTreeInternalMeta& t_meta, std::ostream& t_out) {
    t_out.seekp(t_meta.get_pg_num() * SIZEOF_PAGE);
    t_out.rdbuf()->sputc(static_cast<pt_num_t>(PageType::BTreeInternal));
    auto nkeys = t_meta.get_n_keys();
    t_out.rdbuf()->sputn(std::bit_cast<const char*>(&nkeys), sizeof(nkeys));
    auto first_free = t_meta.get_first_free_off();
    t_out.rdbuf()->sputn(std::bit_cast<const char*>(&first_free),
                         sizeof(first_free));
}

template <>
auto read_from<BTreeInternalMeta>(page_ptr_t t_pg_num, std::istream& t_in)
    -> BTreeInternalMeta {
    t_in.seekg(t_pg_num * SIZEOF_PAGE);
    [[maybe_unused]]
    auto pagetype = t_in.rdbuf()->sbumpc();
    assert(pagetype == static_cast<pt_num_t>(PageType::BTreeInternal));
    // auto pagetype = static_cast<uint8_t>(t_in.rdbuf()->sbumpc());
    // if (pagetype != static_cast<pt_num>(PageType::BTreeInternal)) {
    //     // I should return something more meaningful here.
    //     return;
    // }
    uint16_t nkeys{0};
    t_in.rdbuf()->sputn(std::bit_cast<char*>(&nkeys), sizeof(nkeys));
    uint16_t first_free{0};
    t_in.rdbuf()->sputn(std::bit_cast<char*>(&first_free), sizeof(first_free));
    return {t_pg_num, nkeys, first_free};
}

// I should really reorganize these stuff
// NOLINTBEGIN

/**
 * @class HeapFragMeta
 * @brief Forms a free list inside a heap page.
 *
 */
struct HeapFragMeta {
    // offset 0: 2-byte, offset of previous heap fragment.
    // If there's no fragment before this, the value is nullopt, and is written
    // as a 0 on disk.
    // Next fragment is actually easy to get, given we know the size.
    std::optional<uint16_t> prev;
    // offset 2: 2-byte, size of this heap, not including the header.
    // If the offset of this heap plus size equals 4095, this is the last heap
    // fragment.
    uint16_t size;
    // offset 4: 1-byte, if this fragment is free.
    bool is_free;
};
// NOLINTEND

void write_to(const HeapMeta& t_meta, std::ostream& t_out) {
    t_out.seekp(t_meta.get_pg_num() * SIZEOF_PAGE);
    auto& rdbuf = *t_out.rdbuf();
    rdbuf.sputc(static_cast<char>(PageType::Heap));
    auto nextpg = t_meta.get_next_pg();
    rdbuf.sputn(std::bit_cast<const char*>(&nextpg), sizeof(nextpg));
    auto prevpg = t_meta.get_prev_pg();
    rdbuf.sputn(std::bit_cast<const char*>(&prevpg), sizeof(prevpg));
    auto first_free = t_meta.get_first_free_off();
    rdbuf.sputn(std::bit_cast<const char*>(&first_free), sizeof(first_free));
    auto max_pair = t_meta.get_max_pair();
    rdbuf.sputn(std::bit_cast<const char*>(&max_pair.first),
                sizeof(max_pair.first));
    rdbuf.sputn(std::bit_cast<const char*>(&max_pair.second),
                sizeof(max_pair.second));
}

template <>
auto read_from<HeapMeta>(page_ptr_t t_pg_num, std::istream& t_in) -> HeapMeta {
    t_in.seekg(t_pg_num * SIZEOF_PAGE);
    auto& rdbuf = *t_in.rdbuf();
    // rdbuf.sbumpc();

    [[maybe_unused]]
    auto pagetype = rdbuf.sbumpc();
    assert(pagetype == static_cast<pt_num_t>(PageType::Heap));
    // if (pagetype != static_cast<pt_num_t>(PageType::Heap)) {
    //     // I should return something more meaningful here.
    //     return;
    // }

    page_ptr_t nextpg{0};
    rdbuf.sgetn(std::bit_cast<char*>(&nextpg), sizeof(nextpg));
    page_ptr_t prevpg{0};
    rdbuf.sgetn(std::bit_cast<char*>(&prevpg), sizeof(prevpg));
    page_off_t first_free{0};
    rdbuf.sgetn(std::bit_cast<char*>(&first_free), sizeof(first_free));
    auto max_pair =
        std::make_pair(static_cast<page_off_t>(0), static_cast<page_off_t>(0));
    rdbuf.sgetn(std::bit_cast<char*>(&max_pair.first), sizeof(max_pair.first));
    rdbuf.sgetn(std::bit_cast<char*>(&max_pair.second),
                sizeof(max_pair.second));
    return {t_pg_num, nextpg, prevpg, first_free, max_pair};
}

// technically, write_to could be a templated function, relying on template
// specialization.

template <typename Pg>
concept PageSerializable =
    requires(Pg page, page_ptr_t pagenum, std::iostream stream) {
        {
            // namespace is specified, so that no random read_from's and
            // write_to's qualify. Must be a template specialization of
            // read_from.
            tinydb::dbfile::internal::read_from<Pg>(pagenum, stream)
        } -> std::convertible_to<Pg>;
        tinydb::dbfile::internal::write_to(page, stream);
    };

static_assert(PageSerializable<FreePageMeta>);
static_assert(PageSerializable<BTreeLeafMeta>);
static_assert(PageSerializable<BTreeInternalMeta>);
static_assert(PageSerializable<HeapMeta>);

/**
 * @class PageSerializer
 * @brief Type-erased page metadata, useful for writing stuff into a stream.
 * Pretty useless otherwise.
 *
 */
class PageSerializer {
  public:
    explicit PageSerializer(PageSerializable auto&& t_page)
        : m_impl{new PageModel<std::remove_cvref_t<decltype(t_page)>>(
              std::forward<decltype(t_page)>(t_page))} {}
    /**
     * @brief Reads the data from the specified input stream, starting at the
     * specified page number (usually 4096 * page number bytes).
     *
     * @tparam T The page type to be constructed.
     * @param t_in The specified input stream.
     * @param t_pg_num The page number.
     * @return The PageMeta object containing the page constructed.
     *   Or, an exception if the stream is configured to throw, and a read error
     *   occurs.
     */
    template <PageSerializable T>
    static auto construct_from(std::istream& t_in, page_ptr_t t_pg_num)
        -> PageSerializer {
        PageSerializer ret{T{t_pg_num}};
        ret.do_read_from(t_in);
        return ret;
    }
    /**
     * @brief Reads the page metadata from the given stream.
     *   The content read is written to the page held under this `PageMeta`.
     *
     * @param t_in The given stream.
     *   The stream must inherit from `std::istream`.
     */
    void do_read_from(std::istream& t_in) { m_impl->do_read_from(t_in); }
    /**
     * @brief Writes the content of the page this `PageMeta` holds into the
     * given stream.
     *
     * @param t_out The given stream.
     *   The stream must inherit from `std::ostream`
     */
    void do_write_to(std::ostream& t_out) { m_impl->do_write_to(t_out); }

    // special members

    PageSerializer(PageSerializer&&) = default;
    auto operator=(PageSerializer&&) -> PageSerializer& = default;
    PageSerializer(const PageSerializer& t_meta)
        : m_impl{t_meta.m_impl->clone()} {}
    auto operator=(const PageSerializer& t_meta) -> PageSerializer& {
        m_impl = t_meta.m_impl->clone();
        return *this;
    }
    ~PageSerializer() = default;

  private:
    // NOLINTBEGIN(*special-member*)

    /**
     * @class PageConcept
     * @brief Standardized name.
     * @details A pure virtual struct.
     *
     */
    struct PageConcept {
        virtual void do_read_from(std::istream& t_in) = 0;
        virtual void do_write_to(std::ostream& t_out) const = 0;
        virtual auto clone() -> std::unique_ptr<PageConcept> = 0;
        // gcc got a segfault trying to digest with `= default`
        virtual ~PageConcept() = default;
    };
    // NOLINTEND(*special-member*)

    // NOLINTBEGIN(*special-member*)

    /**
     * @class PageModel
     * @tparam T Must inherit from PageMixin.
     * @brief The inner type that holds the actual page metadata.
     *
     */
    template <PageSerializable T> struct PageModel : PageConcept {
        template <typename Tp>
            requires std::convertible_to<Tp, T>
        explicit PageModel(Tp&& t_page) : m_page(std::forward<Tp>(t_page)) {}
        void do_read_from(std::istream& t_in) override {
            // it seems to not be possible to just name this `read_from` due to
            // conflicting names.
            m_page = read_from<T>(m_page.get_pg_num(), t_in);
        }
        void do_write_to(std::ostream& t_out) const override {
            // write_to(m_page, t_out);
            write_to(m_page, t_out);
        }
        auto clone() -> std::unique_ptr<PageConcept> override {
            return std::make_unique<PageModel>(*this);
        }
        ~PageModel() override = default;
        T m_page;
    };
    // NOLINTEND(*special-member*)

    std::unique_ptr<PageConcept> m_impl;
};

} // namespace tinydb::dbfile::internal
