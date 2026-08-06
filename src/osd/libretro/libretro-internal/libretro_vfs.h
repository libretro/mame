#ifndef LIBRETRO_VFS_H
#define LIBRETRO_VFS_H

#include "osdfile.h"

#include <cstdint>
#include <string>
#include <system_error>

#define RETRO_VFS_HIGHEST_REQUESTED_VERSION 4

extern struct retro_vfs_interface *retro_vfs_interface_ptr;
extern uint32_t retro_vfs_interface_version;

void libretro_vfs_init(void);
bool libretro_vfs_file_exists(const char *path);
bool libretro_vfs_read_whole_file(const char *path, std::string &out);

bool libretro_vfs_active() noexcept;
uint32_t libretro_vfs_version() noexcept;

std::error_condition libretro_vfs_open(std::string const &path, std::uint32_t openflags,
	osd_file::ptr &file, std::uint64_t &filesize) noexcept;

std::error_condition libretro_vfs_remove(std::string const &filename) noexcept;

osd::directory::entry::ptr libretro_vfs_stat(std::string const &path);

#endif // LIBRETRO_VFS_H
