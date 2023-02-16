#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cinttypes>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <cstdint>
#include <cstdio>
#include <limits.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


class PuppyMemory
{
  private:
	pid_t pid = 0;
	uint64_t Addr = 0;
	long address = 0;
  public:
	int Init(const char *name);
	uint64_t get_module_base(const char *module_name);
	void Patch(const char *module, long offset, std::string hex);
	void Restore();
};

int PuppyMemory::Init(const char *name)
{
	int id = -1;
	DIR *dir;
	char filename[32];
	char cmdline[256];
	struct dirent *entry;
	dir = opendir("/proc");
	while ((entry = readdir(dir)) != NULL)
	{
		id = atoi(entry->d_name);
		if (id != 0)
		{
			std::ifstream cmdlineFile;
			sprintf(filename, "/proc/%d/cmdline", id);
			cmdlineFile.open(filename);
			if (cmdlineFile.is_open())
			{
				cmdlineFile.getline(cmdline, sizeof(cmdline));
				cmdlineFile.close();
				if (strcmp(name, cmdline) == 0)
				{
					pid = id;
					closedir(dir);
					return id;
				}
			}
		}
	}
	closedir(dir);
	return -1;
}




uint64_t PuppyMemory::get_module_base(const char *module_name)
{
	char mapspath[128];
	std::sprintf(mapspath, "/proc/%d/maps", pid);
	std::ifstream ifs(mapspath);
	std::string line;
	while (std::getline(ifs, line))
	{
		uint64_t start, end;
		char flags[5], path[PATH_MAX];
		if (std::sscanf(line.c_str(), "%" PRIx64 "-%" PRIx64 " %4s %*x %*x:%*x %*u %s",
						&start, &end, flags, path) != 4)
		{
			continue;
		}
#if defined(__aarch64__)
		if (std::strstr(flags, "x") == nullptr)
		{
			continue;
		}
#endif
		if (std::strstr(path, module_name) != nullptr)
		{
			return start;
		}
	}
	return 0;
}

void PuppyMemory::Patch(const char *module, long offset, std::string hex)
{
	hex.erase(std::remove(hex.begin(), hex.end(), ' '), hex.end());
	size_t MemSize = hex.length() / 2;
	uint64_t value = std::stoul(hex, nullptr, 16);
	std::vector < uint8_t > bytes(MemSize);
	for (size_t i = 0; i < MemSize; ++i)
	{
		bytes[i] = (value >> (i * 8)) & 0xFF;
	}
	std::reverse(bytes.begin(), bytes.end());
	char lj[64];
	int handle;
	address = get_module_base(module) + offset;
	sprintf(lj, "/proc/%d/mem", pid);
	handle = open(lj, O_RDWR);
	lseek(handle, 0, SEEK_SET);
	if (Addr == 0)
	{
		pread64(handle, &Addr, MemSize, address);
	}
	pwrite64(handle, &bytes[0], MemSize, address);
	close(handle);
}

void PuppyMemory::Restore()
{
	if (Addr != 0 && address != 0)
	{
		char lj[64];
		int handle;
		sprintf(lj, "/proc/%d/mem", pid);
		handle = open(lj, O_RDWR);
		lseek(handle, 0, SEEK_SET);
		pwrite64(handle, &Addr, sizeof(Addr), address);
		close(handle);
	}


}
