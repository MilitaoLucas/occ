#include <cstdlib>
#include <occ/core/data_directory.h>

namespace occ {

namespace {
static std::string data_directory_override{""};
}

void set_data_directory(const std::string &s) { data_directory_override = s; }

const char *get_data_directory() {
  if (!data_directory_override.empty()) {
    return data_directory_override.c_str();
  }
  const char *env = std::getenv("OCC_DATA_PATH");
  return env ? env : "";
}

} // namespace occ
