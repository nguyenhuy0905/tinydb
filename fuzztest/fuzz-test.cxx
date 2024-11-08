#include <cstddef>
#include <cstdint>
#include <iostream>

extern "C" auto LLVMFuzzerTestOneInput(const std::uint8_t *data,
                                       std::size_t size) -> int {
  std::cout << "Data size = " << size << "\n";
  if (size > 0) {
    std::cout << "First byte = " << static_cast<unsigned short>(*data) << "\n";
  }

  return 0;
}
