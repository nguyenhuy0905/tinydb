#ifndef TINYDB_DBFILE_INTERNAL_HEAP_HXX
#define TINYDB_DBFILE_INTERNAL_HEAP_HXX

#include <type_traits>
#ifndef ENABLE_MODULE
#include "dbfile/internal/page_base.hxx"
#include "dbfile/internal/page_meta.hxx"
#include <variant>
#endif // !ENABLE_MODULE

namespace tinydb::dbfile::internal {

/**
 * @class AllocPtr
 * @brief Represents a pointer to a heap-allocated block.
 *
 */
struct AllocPtr {
    // the page number.
    page_ptr_t page;
    // the offset relative to the page.
    page_off_t offset;
    // the size of this pointer.
    page_off_t size;
};

static_assert(std::is_trivial_v<AllocPtr>);

/**
 * @class HeapPage
 * @brief The data of the heap page, including the metadata.
 * Also manages the heap page's operations.
 *
 */
class HeapPage {
  public:
    explicit HeapPage(HeapMeta t_meta) : m_meta{t_meta} {}

  private:
    struct FreeDataFrag {};
    struct UsedDataFrag {
        page_off_t size;
    };

    using FragType = std::variant<FreeDataFrag, UsedDataFrag>;
    // basically a doubly-linked list
    struct DataFrag {
        page_off_t prev_frag;
        page_off_t next_frag;
        FragType frag;
    };

    HeapMeta m_meta;
};
// TODO: define read_from for HeapPage.
void write_to(const HeapPage& t_data, std::ostream& t_out);

/**
 * @class Heap
 * @brief Manages all the heap pages.
 *
 */
class Heap {
  // TODO: heap and heap page.
  public:
    void allocate_big(uint64_t t_size);
    auto allocate_small(page_off_t t_size) -> AllocPtr;

  private:
};

} // namespace tinydb::dbfile::internal

#endif // !TINYDB_DBFILE_INTERNAL_HEAP_HXX
