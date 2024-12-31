#ifndef TINYDB_DBFILE_PAGE_HXX
#define TINYDB_DBFILE_PAGE_HXX

#include <cassert>
#include <cstdint>
#include <iosfwd>
#include <type_traits>
namespace tinydb::dbfile {

constexpr uint16_t PAGESIZ = 4096;

/**
 * @brief A flag that each page has.
 * Determines how a page is formatted.
 * */
enum class PageType : uint8_t {
    Free = 0,
    BTreeLeaf,
    BTreeInternal,
    Heap,
};

enum class PageReadError : uint8_t {
    WrongPageType = 1,
};

// PageType's underlying number type.
using pt_num = std::underlying_type_t<PageType>;
using err_num = std::underlying_type_t<PageReadError>;

// NOLINTBEGIN(*-special-member-functions*)
/**
 * @class PageMeta
 * @brief Contains metadata about a page.
 * This is an abstract class.
 *
 * Probably gonna make this an interface.
 */
class PageMeta {
    // NOLINTEND(*-special-member-functions*)
  public:
    // TODO: implement page metadata.
    //
    // I will probably use inheritance for this one. There are a few `PageMeta`
    // types, as listed in the `PageType` enum.

    /**
     * @brief Your usual virtual destructor.
     */
    virtual ~PageMeta() = default;
    /**
     * @brief Write the content of this page into the specified file.
     *
     * @param path The path to the specified file.
     */
    virtual void write_to(std::ostream& t_out) = 0;

    // both write_to and read_from are dealing with I/O, so the cost of virual
    // is, virtually, nothing.

    /**
     * @brief Reads the page data from the specified file.
     *
     * @param t_path The path to the specified file.
     */
    virtual void read_from(std::istream& t_in) = 0;
    /**
     * @return The page number of this page.
     */
    auto get_page_num() -> uint32_t { return m_pagenum; }

  protected:
    /**
     * @brief This is declared as protected so that the end-user cannot create
     * this shit.
     *
     * @param t_pagenum
     */
    explicit PageMeta(uint32_t t_pagenum) : m_pagenum(t_pagenum) {
        // the 0th page is the metadata header.
        assert(t_pagenum > 0);
    }
    // NOLINTBEGIN(*-private-member-variables-*)
    uint32_t m_pagenum;
    // NOLINTEND(*-private-member-variables-*)
};

// NOLINTBEGIN(*-special-member-functions*)

/**
 * @class FreePageMeta
 * @brief Contains metadata about free page.
 * @detail For now, this is a "page" that can be more than 1 page big. A
 * one-page-each implementation should be simpler though, I may reconsider.
 *
 * With this implementation, I don't have a reason to use more than 1 page.
 * Maybe I will remove that "feature".
 *
 */
class FreePageMeta : public PageMeta {
    // NOLINTEND(*-special-member-functions*)
  public:
    FreePageMeta(uint32_t t_pagenum, uint32_t t_p_next_page)
        : PageMeta(t_pagenum), m_p_next_page(t_p_next_page) {}
    /**
     * @brief Initializes a placeholder page meta object. One should call
     * read_from after this.
     *
     * @param t_pagenum
     * @return
     */
    static inline auto placeholder(uint32_t t_pagenum) -> FreePageMeta {
        return FreePageMeta{t_pagenum};
    }
    void write_to(std::ostream& t_out) override;
    void read_from(std::istream& t_in) override;
    ~FreePageMeta() override = default;

  private:
    explicit FreePageMeta(uint32_t t_pagenum)
        : PageMeta(t_pagenum), m_p_next_page(0) {}
    // Format:
    // offset 0: 1-byte set to 0 (indicating free page)
    //   If number is 0, this page and anything below it is free.
    // offset 1: 4-byte pointer to the next free page.
    // the rest: raw bytes of nothingness.
    uint32_t m_p_next_page;
};

/**
 * @class BTreeLeafMeta
 * @brief Contains metadata about a BTree leaf page.
 *
 */
class BTreeLeafMeta : public PageMeta {
  public:
    BTreeLeafMeta(uint32_t t_n_page, uint16_t t_n_cols, uint32_t t_p_next)
        : PageMeta(t_n_page), m_n_cols(t_n_cols), m_p_next(t_p_next) {}

    void read_from(std::istream& t_in) override;

    void write_to(std::ostream& t_out) override;

    /**
     * @brief Initializes a placeholder BTree leaf meta object. One should call
     * read_from after this.
     *
     * @param t_pagenum
     * @return A new `BTreeLeafMeta`.
     */
    static inline auto placeholder(uint32_t t_n_page) -> BTreeLeafMeta {
        return BTreeLeafMeta{t_n_page};
    }

  private:
    explicit BTreeLeafMeta(uint32_t t_n_page)
        : PageMeta(t_n_page), m_n_cols(0), m_p_next(0) {}
    // Format (planned):
    // offset 0: 1-byte set to 1 (indicating B Tree leaf).
    // offset 1: 2-byte, number of rows currently recorded in this page.
    // offset 3: 4-byte, pointer to the next leaf page (0 means this is the last
    // leaf page). offset 7 onwards: the data rows.

    uint16_t m_n_cols;
    uint32_t m_p_next;
    // TODO: format data rows
};

/**
 * @class BTreeInternalMeta
 * @brief Contains metadata about a BTree internal page.
 *
 */
class BTreeInternalMeta : public PageMeta {
  public:
    void read_from(std::istream& t_in) override;
    void write_to(std::ostream& t_out) override;

  private:
    // Format:
    // offset 0: 1-byte set to 2 (indicating B Tree internal).
    // offset 1: 2-byte, number of keys currently in this page.
    //   The number of keys is always at least 1.
    // offset 3 onwards: the list of keys stored in this page.
    uint16_t m_n_pairs;
};

/**
 * @class HeapMeta
 * @brief Contains metadata about a heap page.
 *
 */
class HeapMeta : public PageMeta {
  public:
  private:
    // Format:
    // offset 0: 1-byte set to 3 (indicating heap).
    // offset 1: 4-byte, pointer to the next heap page.
    //   If this number is 0, this is the last heap page in the heap list. If we
    //   need more heap page, we need to get one from the free page.
    // offset 5: 2-byte, pointer to first free fragment.
    // offset 7: 2-byte, the size of the largest fragment.
    //   If this number is the page size minus 9, which is assumed to be
    //   4096 - 9 = 4087, this page is freed and added to the freelist.
    //
    // Free fragments form a linked list.
    // Free fragment format:
    // offset 0: 2-byte, pointer to the next fragment.
    //   We take 0 as "this is the last free fragment in the list".
    // offset 2: 1-byte; 0 if free, 1 if currently in use.
    // offset 3: 2-byte, size of the fragment.
    //
    //   This implementation suffers from memory fragmentation.
    //
    //   To avoid memory fragmentation, we need a way to merge 2 or more free
    //   fragments that lie next to each other into a bigger one.
    //
    //   I'm thinking of, making the linked list sorted in ascending memory
    //   position. That is, the first free fragment is the one with the smallest
    //   offset from the beginning of the page, and so on.
    //
    //   So, if a heap fragment is freed, we can traverse the linked list until
    //   the next pointer is either 0 or larger than this fragment's pointer.
    //
    // Any poitner to a fragment should be, in 6 bytes:
    //   offset 0: 4-byte, pointer to the heap page.
    //   offset 4: 2-byte, the offset from the beginning of that heap page.
    //     Note, the offset points to the actual data in the heap fragment. What
    //     that means is, the number 2 bytes before this number is the size of
    //     the fragment.
    //
    //   Since I lock the data size to be 1-byte, the maximum byte size of any
    //   data is 255 bytes. So, if anything uses more than 255 bytes, some of
    //   its data must be on heap.
    //
    //   Say, the first 249 bytes are at where it should be, and the next 6 is
    //   the fragment pointer.
    //
    //   Then grab as much memory as needed in a heap page. The maximum to be
    //   allocated is 4087 bytes. If more than 4087 is still needed, 4081 is
    //   used for the actual data, and the next 6 is, again, a fragment pointer.
    //
    //   Rinse and repeat until we fit all the data in.
    //
    //   The worst case of this is pretty terrible (looping through all free
    //   page, every single time we need more data).
    //
    //   Alternative: red-black tree and a best-fit allocation scheme.
    //   I will cook this later, if I ever decide on this.
};

} // namespace tinydb::dbfile

#endif // !TINYDB_DBFILE_PAGE_HXX
