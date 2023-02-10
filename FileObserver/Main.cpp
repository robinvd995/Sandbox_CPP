#include <iostream>

#include "FileObserver.h"
#include <thread>

void created(const std::string& filepath) { 
    std::cout << "Created: " << filepath << std::endl;
}

void changed(const std::string& filepath) {
    std::cout << "Changed: " << filepath << std::endl;
}

void deleted(const std::string& filepath) {
    std::cout << "Deleted: " << filepath << std::endl;
}

FileObserver observer(1000, true);
bool running = true;

const char* observerErrorToStr(FileObserverError error) {
    switch(error) {
        case FileObserverError::None: return "No error";
        case FileObserverError::DirectoryNotFound: return "Directory not found";
        case FileObserverError::EntryAlreadyExists: return "Entry already exists";
        case FileObserverError::EntryNotFound: return "Entry not found";
        case FileObserverError::FileNotFound: return "File not found";
    }
}

void run() {
    std::cout << "Initializing FileObserver thread..." << std::endl;

    std::cout << "\nAdd Files:\n";
    std::cout << observerErrorToStr(observer.addFile("file_observable.txt")) << std::endl;
    std::cout << observerErrorToStr(observer.addFile("file_observable.txt")) << std::endl;
    std::cout << observerErrorToStr(observer.addFile("file_removeable.txt")) << std::endl;
    std::cout << observerErrorToStr(observer.addFile("file_not_exists.txt")) << std::endl;
    std::cout << observerErrorToStr(observer.addFile("dir_observable/dir_file_independent.txt")) << std::endl;
    std::cout << observerErrorToStr(observer.addFile("dir_observable/dir_file_removable.txt")) << std::endl;

    std::cout << "\nRemove Files:\n";
    std::cout << observerErrorToStr(observer.removeFile("file_removeable.txt")) << std::endl;
    std::cout << observerErrorToStr(observer.removeFile("file_not_exists.txt")) << std::endl;

    std::cout << "\nAdd Dirs:\n";
    std::cout << observerErrorToStr(observer.addDirectory("dir_observable")) << std::endl;
    std::cout << observerErrorToStr(observer.addDirectory("dir_observable")) << std::endl;
    std::cout << observerErrorToStr(observer.addDirectory("dir_removeable")) << std::endl;
    std::cout << observerErrorToStr(observer.addDirectory("dir_not_exists")) << std::endl;

    std::cout << "\nRemove Dirs:\n";
    std::cout << observerErrorToStr(observer.removeDirectory("dir_removeable")) << std::endl;
    std::cout << observerErrorToStr(observer.removeDirectory("dir_not_exists")) << std::endl;
    std::cout << observerErrorToStr(observer.removeFile("dir_observable/dir_file_removable.txt")) << std::endl;

    std::cout << "\nAdd Ignores:\n";
    std::cout << observerErrorToStr(observer.addIgnore("dir_observable/file_ignored.txt")) << std::endl;

    observer.setFileCreatedCallback(created);
    observer.setFileChangedCallback(changed);
    observer.setFileDeletedCallback(deleted);

    std::cout << "\nStarting FileObserver loop..." << std::endl;
    while(running) {
        observer.observe();
    }
    std::cout << "Ending FileObserver loop..." << std::endl;
}

int main()
{
    std::thread t(run);

    std::string cmd = "";

    while(true) {
        std::getline(std::cin, cmd);
        if (cmd == "end") break;
        if(cmd.rfind("remove_dir", 0) == 0) {
            int index = -1;
            for(int i = 0; i < cmd.size(); i++) 
                if (cmd[i] == ' ') { index = i; break; }

            if (index == -1) continue;
            std::string arg = cmd.substr(index + 1);

            std::cout << observerErrorToStr(observer.removeDirectory(arg)) << std::endl;
        }

    }
    


    running = false;
    t.join();
}