#ifndef TINYDB_ARGS_HXX
#define TINYDB_ARGS_HXX

#include <expected>
#include <fstream>
#include <optional>

namespace tinydb::cli {

/**
 * @class Cli
 * @brief The REPL.
 *
 */
class Cli {
  public:
    Cli();
    /**
     * @brief Runs the REPL.
     */
    void run();

  private:
    /**
     * @brief Database file
     */
    std::optional<std::fstream> file;
};

} // namespace tinydb::cli

#endif // !TINYDB_ARGS_HXX
