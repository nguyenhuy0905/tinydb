#ifndef TINYDB_ARGS_HXX
#define TINYDB_ARGS_HXX

#include <expected>
#include <fstream>
#include <optional>

namespace tinydb {

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

} // namespace tinydb

#endif // !TINYDB_ARGS_HXX
