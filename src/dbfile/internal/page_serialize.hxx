#ifndef TINYDB_DBFILE_INTERNAL_PAGE_SERIALIZE_HXX
#define TINYDB_DBFILE_INTERNAL_PAGE_SERIALIZE_HXX

#include "general/modules.hxx"
#ifndef ENABLE_MODULES
#include "general/sizes.hxx"
#include "dbfile/internal/page_base.hxx"
#include "dbfile/internal/page_meta.hxx"
#include <cassert>
#include <bit>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <optional>
#include <print>
#include <utility>
#endif // !ENABLE_MODULES

EXPORT
namespace tinydb::dbfile::internal {

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

template <>
auto read_from<FreePageMeta>(page_ptr_t t_pg_num, std::istream& t_in)
    -> FreePageMeta;

template <>
auto read_from<BTreeLeafMeta>(page_ptr_t t_pg_num, std::istream& t_in)
    -> BTreeLeafMeta;

template <>
auto read_from<HeapMeta>(page_ptr_t t_pg_num, std::istream& t_in) -> HeapMeta;

void write_to(const FreePageMeta& t_meta, std::ostream& t_out);

void write_to(const BTreeLeafMeta& t_meta, std::ostream& t_out);

void write_to(const BTreeInternalMeta& t_meta, std::ostream& t_out);

void write_to(const HeapMeta& t_meta, std::ostream& t_out);

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


#endif // !TINYDB_DBFILE_INTERNAL_PAGE_SERIALIZE_HXX
