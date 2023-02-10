#include "FileObserver.h"

#include <vector>

FileObserver::FileObserver(uint32_t delay, bool auto_erase) :
	m_Delay(delay), m_AutoErase(auto_erase) 
{}

void FileObserver::setFileCreatedCallback(FileWatcherCallback file_created_func) {
	m_FileCreatedCallback = file_created_func;
}

void FileObserver::setFileChangedCallback(FileWatcherCallback file_changed_func) {
	m_FileChangedCallback = file_changed_func;
}

void FileObserver::setFileDeletedCallback(FileWatcherCallback file_deleted_func) {
	m_FileDeletedCallback = file_deleted_func;
}

FileObserverError FileObserver::addFile(const std::string& filepath) {
	FileObserverError error = FileObserverError::None;
	m_ObserverMutex.lock();

	fs::path p(filepath);
	error = addFileInternal(p, false);

	m_ObserverMutex.unlock();
	return error;
}

FileObserverError FileObserver::removeFile(const std::string& filepath) {
	FileObserverError error = FileObserverError::None;
	m_ObserverMutex.lock();

	fs::path p(filepath);
	error = removeFileInternal(p, false);

	m_ObserverMutex.unlock();
	return error;
}

FileObserverError FileObserver::addFileInternal(const fs::path& filepath, bool dirAdd) {
	if (!std::filesystem::exists(filepath)) return FileObserverError::FileNotFound;

	auto it = m_TimeChangedMap.find(filepath);
	if (it != m_TimeChangedMap.end()) {
		if (!dirAdd && !it->second.independent) it->second.independent = true;
		else return FileObserverError::EntryAlreadyExists;
	}
	else m_TimeChangedMap.insert({ filepath, { std::filesystem::last_write_time(filepath), !dirAdd } });
	return FileObserverError::None;
}

FileObserverError FileObserver::removeFileInternal(const fs::path& filepath, bool dirRemove) {
	auto it = m_TimeChangedMap.find(filepath);
	if (it == m_TimeChangedMap.end()) return FileObserverError::EntryNotFound;
	else {
		if (it->second.independent && dirRemove) return FileObserverError::None;
		m_TimeChangedMap.erase(it);
	}
	return FileObserverError::None;
}

FileObserverError FileObserver::addDirectory(const std::string& directory) {
	FileObserverError error = FileObserverError::None;
	m_ObserverMutex.lock();

	if (m_DirectorySet.find(directory) != m_DirectorySet.end()) error = FileObserverError::EntryAlreadyExists;
	else if (!std::filesystem::exists(directory)) error = FileObserverError::DirectoryNotFound;
	else {
		m_DirectorySet.insert(directory);
		for (auto& file : std::filesystem::recursive_directory_iterator(directory)) {
			if (m_IgnoredSet.find(file.path()) != m_IgnoredSet.end()) continue;
			addFileInternal(file.path(), true);
		}
	}
	m_ObserverMutex.unlock();
	return error;
}

FileObserverError FileObserver::removeDirectory(const std::string& directory) {
	FileObserverError error = FileObserverError::None;
	m_ObserverMutex.lock();

	auto it = m_DirectorySet.find(directory);
	if (it == m_DirectorySet.end()) error = FileObserverError::EntryNotFound;
	else {
		for (auto& file : std::filesystem::recursive_directory_iterator(directory)) {
			removeFileInternal(file.path(), true);
		}

		m_DirectorySet.erase(it);
	}

	m_ObserverMutex.unlock();
	return error;
}

FileObserverError FileObserver::addIgnore(const std::string& filepath) {
	FileObserverError error = FileObserverError::None;
	m_ObserverMutex.lock();

	fs::path pth = fs::path(filepath);
	auto it = m_IgnoredSet.find(pth);
	if (it != m_IgnoredSet.end()) error = FileObserverError::EntryAlreadyExists;
	else {
		m_IgnoredSet.insert(pth);
		auto fileit = m_TimeChangedMap.find(fs::path(pth));
		if (fileit != m_TimeChangedMap.end()) m_TimeChangedMap.erase(fileit);
	}

	m_ObserverMutex.unlock();
	return error;
}

FileObserverError FileObserver::removeIgnore(const std::string& file) {
	FileObserverError error = FileObserverError::None;
	m_ObserverMutex.lock();

	auto it = m_IgnoredSet.find(file);
	if (it == m_IgnoredSet.end()) error = FileObserverError::IgnoredNoEntry;
	else m_IgnoredSet.erase(it);

	m_ObserverMutex.unlock();
	return error;
}

void FileObserver::observe() {
	std::this_thread::sleep_for(std::chrono::milliseconds(m_Delay));

	m_ObserverMutex.lock();
	// observe directories for newly created files
	for (auto& directory : m_DirectorySet) {
		for (auto& file : std::filesystem::recursive_directory_iterator(directory)) {
			fs::path pth = file.path();
			if (m_IgnoredSet.find(pth) != m_IgnoredSet.end()) continue;
			auto it = m_TimeChangedMap.find(pth);
			if (it == m_TimeChangedMap.end()) {
				m_TimeChangedMap.insert({ pth, { std::filesystem::last_write_time(pth), true } });
				if (m_FileCreatedCallback) m_FileCreatedCallback(pth.string());
			}
		}
	}

	// observe files
	std::vector<fs::path> deletedFiles;
	for (auto it = m_TimeChangedMap.begin(); it != m_TimeChangedMap.end(); it++) {

		if(!std::filesystem::exists(it->first)) {
			if(m_FileDeletedCallback) m_FileDeletedCallback(it->first.string());
			if(m_AutoErase) deletedFiles.push_back(it->first);
			continue;
		}

		auto curTime = std::filesystem::last_write_time(it->first);
		if(it->second != curTime) {
			if(m_FileChangedCallback) m_FileChangedCallback(it->first.string());
			it->second.timeChanged = curTime;
		}
	}

	if(!deletedFiles.empty()) {
		for(auto& file : deletedFiles) {
			m_TimeChangedMap.erase(file);
		}
	}
	m_ObserverMutex.unlock();
}
