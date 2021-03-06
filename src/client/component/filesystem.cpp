#include <std_include.hpp>
#include "../loader/component_loader.hpp"

#include <utils/hook.hpp>

#include "filesystem.hpp"

namespace filesystem {
namespace {
const char* sys_default_install_path_stub() {
  static auto current_path = std::filesystem::current_path().string();
  return current_path.data();
}
} // namespace

file::file(std::string name, game::FsThread thread) : name_(std::move(name)) {
  assert(!this->name_.empty());

  void* handle;
  const auto len =
      game::FS_FOpenFileReadForThread(name_.data(), &handle, thread);

  if (!handle) {
    this->valid_ = false;
    return;
  }

  auto* buf = static_cast<char*>(game::Hunk_AllocateTempMemory(len + 1));
  game::FS_Read(buf, len, handle);
  buf[len] = '\0';

  game::FS_FCloseFile(handle);

  this->valid_ = true;
  this->buffer_.append(buf, len);
}

bool file::exists() const { return this->valid_; }

const std::string& file::get_buffer() const { return this->buffer_; }

const std::string& file::get_name() const { return this->name_; }

class component final : public component_interface {
public:
  void post_load() override {
    utils::hook(0x465B90, sys_default_install_path_stub, HOOK_CALL)
        .install()
        ->quick(); // Sys_CreateFile
  }
};
} // namespace filesystem

REGISTER_COMPONENT(filesystem::component)
