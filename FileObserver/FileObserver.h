#pragma once

#include <map>
#include <string>
#include <chrono>
#include <filesystem>
#include <mutex>
#include <set>
#include <filesystem>

// C++17 or newer is required to use the filesystem
namespace fs = std::filesystem;

enum class FileObserverError {
	None = 0,
	FileNotFound = 1,
	DirectoryNotFound = 2,
	EntryAlreadyExists = 3,
	EntryNotFound = 4,
	IgnoredNoEntry = 5
};

struct TimeChangedEntry {
	fs::file_time_type timeChanged;
	bool independent = false;

	TimeChangedEntry() = delete;
	TimeChangedEntry(fs::file_time_type p_timeChanged, bool p_independent) :
		timeChanged(p_timeChanged), independent(p_independent)
	{}
	TimeChangedEntry(const TimeChangedEntry&) = default;
	~TimeChangedEntry() = default;

	friend bool operator== (const TimeChangedEntry& lh, const TimeChangedEntry& rh) { return lh.timeChanged == rh.timeChanged; }
	friend bool operator== (const TimeChangedEntry& lh, const fs::file_time_type& rh) { return lh.timeChanged == rh; }
	friend bool operator!= (const TimeChangedEntry& lh, const TimeChangedEntry& rh) { return lh.timeChanged != rh.timeChanged; }
	friend bool operator!= (const TimeChangedEntry& lh, const fs::file_time_type& rh) { return lh.timeChanged != rh; }
};

class FileObserver {

public:

	typedef void (*FileWatcherCallback)(const std::string& entry);

	FileObserver() = delete;
	FileObserver(const FileObserver&) = delete;
	FileObserver(uint32_t delay = 1000, bool auto_erase = true);
	~FileObserver() = default;

	void setFileCreatedCallback(FileWatcherCallback file_created_func);
	void setFileChangedCallback(FileWatcherCallback file_changed_func);
	void setFileDeletedCallback(FileWatcherCallback file_deleted_func);

	FileObserverError addFile(const std::string& filepath);
	FileObserverError removeFile(const std::string& filepath);
	FileObserverError addDirectory(const std::string& directory);
	FileObserverError removeDirectory(const std::string& directory);
	FileObserverError addIgnore(const std::string& filepath);
	FileObserverError removeIgnore(const std::string& filepath);

	void observe();

private:
	FileObserverError addFileInternal(const fs::path& filepath, bool dirAdd);
	FileObserverError removeFileInternal(const fs::path& filepath, bool dirRemove);

private:
	std::map<fs::path, TimeChangedEntry> m_TimeChangedMap;

	template <class Key, class Compare = std::less<Key>>
	using set = std::set<Key, Compare, std::pmr::polymorphic_allocator<Key>>;
	set<fs::path> m_DirectorySet;
	set<fs::path> m_IgnoredSet;

	std::mutex m_ObserverMutex;

	FileWatcherCallback m_FileCreatedCallback = nullptr;
	FileWatcherCallback m_FileChangedCallback = nullptr;
	FileWatcherCallback m_FileDeletedCallback = nullptr;

	uint32_t m_Delay;
	bool m_AutoErase;
};
