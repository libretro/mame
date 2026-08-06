#include "libretro_vfs.h"

#include "libretro.h"
#include "libretro_shared.h"

#include <cassert>
#include <cstring>
#include <new>

struct retro_vfs_interface *retro_vfs_interface_ptr;
uint32_t retro_vfs_interface_version;

namespace
{
class libretro_vfs_file final : public osd_file
{
public:
	libretro_vfs_file(libretro_vfs_file const &) = delete;
	libretro_vfs_file(libretro_vfs_file &&) = delete;
	libretro_vfs_file &operator=(libretro_vfs_file const &) = delete;
	libretro_vfs_file &operator=(libretro_vfs_file &&) = delete;

	explicit libretro_vfs_file(struct retro_vfs_file_handle *handle) noexcept
		: m_handle(handle)
	{
		assert(m_handle);
	}

	virtual ~libretro_vfs_file() override
	{
		if (m_handle && retro_vfs_interface_ptr && retro_vfs_interface_ptr->close)
			retro_vfs_interface_ptr->close(m_handle);
	}

	virtual std::error_condition read(void *buffer, std::uint64_t offset, std::uint32_t length, std::uint32_t &actual) noexcept override
	{
		actual = 0;

		if (!retro_vfs_interface_ptr || !retro_vfs_interface_ptr->seek || !retro_vfs_interface_ptr->read)
			return std::errc::not_supported;

		if (retro_vfs_interface_ptr->seek(m_handle, std::int64_t(offset), RETRO_VFS_SEEK_POSITION_START) < 0)
			return std::errc::io_error;

		if (!length)
			return std::error_condition();

		std::int64_t const result = retro_vfs_interface_ptr->read(m_handle, buffer, length);
		if (result < 0)
			return std::errc::io_error;

		actual = std::uint32_t(result);
		return std::error_condition();
	}

	virtual std::error_condition write(void const *buffer, std::uint64_t offset, std::uint32_t length, std::uint32_t &actual) noexcept override
	{
		actual = 0;

		if (!retro_vfs_interface_ptr || !retro_vfs_interface_ptr->seek || !retro_vfs_interface_ptr->write)
			return std::errc::not_supported;

		if (retro_vfs_interface_ptr->seek(m_handle, std::int64_t(offset), RETRO_VFS_SEEK_POSITION_START) < 0)
			return std::errc::io_error;

		if (!length)
			return std::error_condition();

		std::int64_t const result = retro_vfs_interface_ptr->write(m_handle, buffer, length);
		if (result < 0)
			return std::errc::io_error;

		actual = std::uint32_t(result);
		return std::error_condition();
	}

	virtual std::error_condition truncate(std::uint64_t offset) noexcept override
	{
		if (!retro_vfs_interface_ptr || !retro_vfs_interface_ptr->truncate)
			return std::errc::not_supported;

		if (retro_vfs_interface_ptr->truncate(m_handle, std::int64_t(offset)) < 0)
			return std::errc::io_error;

		return std::error_condition();
	}

	virtual std::error_condition flush() noexcept override
	{
		if (!retro_vfs_interface_ptr || !retro_vfs_interface_ptr->flush)
			return std::error_condition(); // nothing to flush, not an error

		if (retro_vfs_interface_ptr->flush(m_handle) < 0)
			return std::errc::io_error;

		return std::error_condition();
	}

private:
	struct retro_vfs_file_handle *m_handle;
};

} // anonymous namespace

void libretro_vfs_init(void)
{
	struct retro_variable var = {0};
	var.key = CORE_NAME "_vfs_enabled";
	var.value = NULL;

	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
	{
		if (strcmp(var.value, "enabled") != 0)
			return;
	}

	struct retro_vfs_interface_info vfs_iface_info;
	int version;

	retro_vfs_interface_ptr = NULL;
	retro_vfs_interface_version = 0;

	vfs_iface_info.required_interface_version = RETRO_VFS_HIGHEST_REQUESTED_VERSION;
	vfs_iface_info.iface = NULL;

	if (environ_cb(RETRO_ENVIRONMENT_GET_VFS_INTERFACE, &vfs_iface_info) &&
		vfs_iface_info.iface)
	{
		retro_vfs_interface_ptr     = vfs_iface_info.iface;
		retro_vfs_interface_version = (uint32_t)version;
	}
}

bool libretro_vfs_active() noexcept
{
	return retro_vfs_interface_ptr != nullptr;
}

uint32_t libretro_vfs_version() noexcept
{
	return retro_vfs_interface_version;
}

bool libretro_vfs_file_exists(const char *path)
{
   int64_t size  = 0;
   int     flags;

   if (!retro_vfs_interface_ptr || !retro_vfs_interface_ptr->stat)
      return false;

   flags = retro_vfs_interface_ptr->stat_64(path, &size);
   return (flags & RETRO_VFS_STAT_IS_VALID) &&
          !(flags & RETRO_VFS_STAT_IS_DIRECTORY);
}

bool libretro_vfs_read_whole_file(const char *path, std::string &out)
{
   struct retro_vfs_file_handle *fh;
   int64_t size;
   int64_t nread;

   out.clear();

   if (!retro_vfs_interface_ptr || !retro_vfs_interface_ptr->open)
      return false;

   fh = retro_vfs_interface_ptr->open(path, RETRO_VFS_FILE_ACCESS_READ,
         RETRO_VFS_FILE_ACCESS_HINT_NONE);
   if (!fh)
      return false;

   size = retro_vfs_interface_ptr->size(fh);
   if (size < 0)
   {
      retro_vfs_interface_ptr->close(fh);
      return false;
   }

   out.resize((size_t)size);
   nread = (size > 0)
      ? retro_vfs_interface_ptr->read(fh, &out[0], (uint64_t)size)
      : 0;
   retro_vfs_interface_ptr->close(fh);

   if (nread != size)
   {
      out.clear();
      return false;
   }
}

std::error_condition libretro_vfs_open(std::string const &path, std::uint32_t openflags, osd_file::ptr &file, std::uint64_t &filesize) noexcept
{
	if (!retro_vfs_interface_ptr || !retro_vfs_interface_ptr->open)
		return std::errc::not_supported;

	unsigned mode = 0;

	if (openflags & OPEN_FLAG_READ)
		mode |= RETRO_VFS_FILE_ACCESS_READ;

	if (openflags & OPEN_FLAG_WRITE)
	{
		mode |= RETRO_VFS_FILE_ACCESS_WRITE;

		if (!(openflags & OPEN_FLAG_CREATE))
			mode |= RETRO_VFS_FILE_ACCESS_UPDATE_EXISTING;
	}
	if (!mode)
		return std::errc::invalid_argument;

	struct retro_vfs_file_handle *handle = retro_vfs_interface_ptr->open(
			path.c_str(), mode, RETRO_VFS_FILE_ACCESS_HINT_NONE);

	if (!handle && (openflags & OPEN_FLAG_WRITE) && (openflags & OPEN_FLAG_CREATE))
	{
		mode &= ~unsigned(RETRO_VFS_FILE_ACCESS_UPDATE_EXISTING);
		handle = retro_vfs_interface_ptr->open(path.c_str(), mode, RETRO_VFS_FILE_ACCESS_HINT_NONE);
	}

	if (!handle)
		return std::errc::no_such_file_or_directory;

	std::int64_t size = retro_vfs_interface_ptr->size ? retro_vfs_interface_ptr->size(handle) : -1;
	if (size < 0)
		size = 0;

	osd_file::ptr result(new (std::nothrow) libretro_vfs_file(handle));
	if (!result)
	{
		retro_vfs_interface_ptr->close(handle);
		return std::errc::not_enough_memory;
	}

	file = std::move(result);
	filesize = std::uint64_t(size);
	return std::error_condition();
}


std::error_condition libretro_vfs_remove(std::string const &filename) noexcept
{
	if (!retro_vfs_interface_ptr || !retro_vfs_interface_ptr->remove)
		return std::errc::not_supported;

	if (retro_vfs_interface_ptr->remove(filename.c_str()) != 0)
		return std::errc::io_error;

	return std::error_condition();
}


osd::directory::entry::ptr libretro_vfs_stat(std::string const &path)
{
	if (!retro_vfs_interface_ptr || !retro_vfs_interface_ptr->stat_64)
		return nullptr;

	std::int64_t size = 0;
	int const flags = retro_vfs_interface_ptr->stat_64(path.c_str(), &size);

	if (!(flags & RETRO_VFS_STAT_IS_VALID))
		return nullptr;

	auto const result = reinterpret_cast<osd::directory::entry *>(
		::operator new(
			sizeof(osd::directory::entry) + path.length() + 1,
			std::align_val_t(alignof(osd::directory::entry)),
			std::nothrow));

	if (!result)
		return nullptr;
	new (result) osd::directory::entry;

	auto const resultname = reinterpret_cast<char *>(result) + sizeof(*result);
	std::strcpy(resultname, path.c_str());
	result->name = resultname;
	result->type = (flags & RETRO_VFS_STAT_IS_DIRECTORY)
			? osd::directory::entry::entry_type::DIR
			: osd::directory::entry::entry_type::FILE;
	result->size = std::uint64_t(std::uint32_t(size));
	result->last_modified = std::chrono::system_clock::time_point{};

	return osd::directory::entry::ptr(result);
}
