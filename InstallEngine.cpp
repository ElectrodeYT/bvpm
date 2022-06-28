#include <InstallEngine.h>
#include <DependencyEngine.h>
#include <iostream>
#include <cstring>
#include <filesystem>
#include <debug.h>
#include <sys/wait.h>
#include <human-readable.h>

namespace fs = std::filesystem;

bool InstallEngine::AddPackage(std::string package) {
    PRINT_DEBUG("adding package " << package << " to install engine list" << std::endl);
    std::cout << "\33[2K\rAdding package " << package;
    std::cout.flush();
    // TODO: add network package support
    PackageFile file;
    // TODO: read this from manifest
    file.name = package;
    file.path = package;
    file.a = archive_read_new();
    archive_read_support_filter_all(file.a);
    archive_read_support_format_all(file.a);
    if(archive_read_open_filename(file.a, package.c_str(), 1024) != ARCHIVE_OK) {
        std::cout << "error adding package " << package << " to install list: archive not ok" << std::endl;
        return false;
    }
    struct archive_entry* file_entry;
    bool has_manifest = false;
    bool has_owned_files = false;
    bool has_hashes = false;
    size_t file_size = fs::file_size(package);
    file.total_package_file_bytes = file_size;
    while(archive_read_next_header(file.a, &file_entry) == ARCHIVE_OK) {
        std::cout << "\33[2K\rAdding package " << package << ": " << humanSize(archive_filter_bytes(file.a, -1)) << "/" << humanSize(file_size);
        std::cout.flush();
        std::string name = archive_entry_pathname(file_entry);
        file.total_package_bytes += archive_entry_size(file_entry);
        if(name == "manifest") {
            has_manifest = true;
            // We also try to read the manifest now
            size_t manifest_size = archive_entry_size(file_entry);
            char* data = (char*)malloc(manifest_size + 2);
            memset(data, 0, manifest_size + 1);
            archive_read_data(file.a, data, manifest_size);
            file.manifest = Config::readFromData(data);
        }
        if(name == "owned-files") {
            has_owned_files = true;
            // We also try to read the manifest now
            size_t owned_files_size = archive_entry_size(file_entry);
            char* data = (char*)malloc(owned_files_size + 2);
            memset(data, 0, owned_files_size + 1);
            archive_read_data(file.a, data, owned_files_size);

            // Split the owned-files by newline
            std::istringstream ss(data);
            std::string line;
            while(std::getline(ss, line, '\n')) {
                file.owned_files.push_back(line);
            }
        }
        if(name == "sums") {
            has_hashes = true;
            // We also try to read the manifest now
            size_t sums_size = archive_entry_size(file_entry);
            char* data = (char*)malloc(sums_size + 2);
            memset(data, 0, sums_size + 1);
            archive_read_data(file.a, data, sums_size);

            // Split the sums by newline
            std::istringstream ss(data);
            std::string line;
            while(std::getline(ss, line, '\n')) {
                std::string hash = line.substr(0, line.find(' '));
                std::string file_str = line.substr(line.find(' '), line.size());

                // Sanitize file a bit
                while(file_str[0] == ' ') { file_str.erase(0, 1); }
                if(file_str[0] == '*') { file_str.erase(0, 1); }
                if(file_str[0] == '.') { file_str.erase(0, 1); }

                file.file_hashes[file_str] = hash;
            }
        }
        if(name == "afterinstall.sh") { file.has_after_install = true; }
        // We also make a record of all files and folders in root/
        if(name.rfind("root/", 0) == 0) {
            // Create a std::string and chop the root/ off
            std::string name_str(name.erase(0, strlen("root/")));
            if(name_str.empty()) { continue; }
            if(*(name.end()--) == '/') {
                //std::cout << "folder in root: " << name_str << std::endl;
                file.folders.push_back(name_str);
            } else {
                file.files.push_back(name_str);
            }
        }
        archive_read_data_skip(file.a);
    }
    std::cout << "\33[2K\rDone reading package " << package;
    std::cout.flush();
    archive_read_close(file.a);
    archive_read_free(file.a);
    if(has_manifest) {
        // We now parse the manifest (mostly to find the package name)
        if(file.manifest.values.find("PACKAGE") == file.manifest.values.end()) {
            std::cout << std::endl <<  "error adding package " << package << " to install list: manifest is missing package name" << std::endl;
            return false;
        } else {
            file.name = file.manifest.values["PACKAGE"];
        }
        if(file.manifest.values.find("VERSION") != file.manifest.values.end()) {
            file.version = file.manifest.values["VERSION"];
        }
        // We now also check if we even need to install this
        // If this package is installed, then a config read of /etc/bvpm/packages/x/manifest should work
        ConfigFile installed_manifest = Config::readConfigFile(install_root + "/etc/bvpm/packages/" + file.name + "/manifest");
        if(installed_manifest.values.find("failed") == installed_manifest.values.end()) {
            // This package is already installed
            // If the version is the same as this package
            // Exception: if this package has no version, we let it install
            if(installed_manifest.values.find("VERSION") != installed_manifest.values.end()) {
                if(installed_manifest.values["VERSION"] == file.version) {
                    std::cout << std::endl << "Package " << file.name << " of same version is already installed, skipping" << std::endl;
                    return true; // We return true here since this is not a fatal error
                }
            }
        }
    }
    if(!has_manifest || !has_owned_files) {
        std::cout << std::endl << "error adding package " << package << " to install list: archive is missing required files" << std::endl;
        return false;
    }
    if(!has_hashes) {
        std::cout << std::endl << "warning adding package " << package << " to install list: archive is missing hashes" << std::endl;
    }
    package_list.push_back(file);
    return true;
}

bool InstallEngine::VerifyIntegrity() {
    for(PackageFile package : package_list) {
        // We now reopen the archive
        package.a = archive_read_new();
        archive_read_support_format_all(package.a);
        archive_read_support_filter_all(package.a);
        if(archive_read_open_filename(package.a, package.path.c_str(), 1024) != ARCHIVE_OK) {
            std::cout << "error installing package " << package.name << ": archive not ok" << std::endl;
            continue;
        }
    }
    return true;
}

// I copied this from the libarchive examples
// TODO: replace this
static int copy_data(struct archive *ar, struct archive *aw) {
    int r;
    const void *buff;
    size_t size;
    la_int64_t offset;

    for (;;) {
        r = archive_read_data_block(ar, &buff, &size, &offset);
        if (r == ARCHIVE_EOF)
            return (ARCHIVE_OK);
        if (r < ARCHIVE_OK)
            return (r);
        r = archive_write_data_block(aw, buff, size, offset);
        if (r < ARCHIVE_OK) {
            fprintf(stderr, "%s\n", archive_error_string(aw));
            return (r);
        }
    }
}

bool InstallEngine::GetUserPermission() {
    if(empty()) { return false; }
    std::cout << "The following packages will be installed: " << std::endl;
    size_t total_size = 0;
    size_t total_file_size = 0;
    for(PackageFile package : package_list) {
        std::cout << "\t" << package.name << " (size: " << humanSize(package.total_package_bytes) << ", file size: " << humanSize(package.total_package_file_bytes) << ")" << std::endl;
        total_size += package.total_package_bytes;
        total_file_size += package.total_package_file_bytes;
    }
    std::cout << "Total size of packages: " << humanSize(total_size) << "\n";
    std::cout << "Total file size of packages: " << humanSize(total_file_size) << "\n";
    // Check if we have enough space
    if(fs::space(fs::path(install_root)).available <= total_size) {
        std::cout << "Error: Not enough space on filesystem (space available: " << humanSize(fs::space(fs::path(install_root)).available) << ")" << std::endl;
        return false;
    }
    std::cout << "Are you sure? [y/N] ";
    fflush(stderr);
    fflush(stdout);
    char response = getchar();
    if(response == 'y' || response == 'Y') { return true; }
    return false;
}

bool InstallEngine::Execute() {
    std::vector<PackageFile> afterinstall_script_list;
    for(PackageFile package : package_list) {
        std::cout << "Operating on " << package.name << '\r';
        std::cout.flush();
        // We mkdir all the folders first
        for(std::string folder : package.folders) {
            folder = install_root + "/" + folder;
            PRINT_DEBUG("creating folder " << folder << std::endl);
            if(!fs::exists(folder)) {
                fs::create_directories(folder);
            }
        }

        // We now copy the files
        // To do this we re-create the libarchive archive
        int flags = ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_PERM | ARCHIVE_EXTRACT_FFLAGS;
        package.a = archive_read_new();
        archive_read_support_format_all(package.a);
        archive_read_support_filter_all(package.a);
        struct archive* extract;

        // We create the libarchive extractor thing
        extract = archive_write_disk_new();
        archive_write_disk_set_options(extract, flags);
        archive_write_disk_set_standard_lookup(extract);
        if(archive_read_open_filename(package.a, package.path.c_str(), 1024) != ARCHIVE_OK) {
            std::cout << "error installing package " << package.name << ": archive not ok" << std::endl;
            continue;
        }

        // We now stream through the archive again
        struct archive_entry* file_entry;
        size_t copied_files = 0;
        while(archive_read_next_header(package.a, &file_entry) == ARCHIVE_OK) {
            const char* name = archive_entry_pathname(file_entry);
            if(strcmp(name, "manifest") == 0 || strcmp(name, "owned-files") == 0 || strcmp(name, "afterinstall.sh") == 0 || strcmp(name, "sums") == 0) {
                // We copy the manifest to a specific folder
                std::string name_str(name);
                name_str = "etc/bvpm/packages/" + package.name + "/" + name_str;
                PRINT_DEBUG("extracting in root: " << name_str << std::endl);
                struct archive_entry* extracted_entry;
                extracted_entry = archive_entry_clone(file_entry);

                // Create the pathname with the install root
                std::string path_string = install_root + "/" + name_str;

                archive_entry_set_pathname(extracted_entry, path_string.c_str());
                // We can now begin copying the data
                archive_write_header(extract, extracted_entry);
                copy_data(package.a, extract);
                archive_write_finish_entry(extract);
            }
            if(strncmp(name, "root/", strlen("root/")) == 0) {
                // Create a std::string and chop the root/ off
                std::string name_str(name + strlen("root/"));
                if(name_str.empty()) { continue; }
                if(name[strlen(name) - 1] == '/') {
                    // We dont care about folders anymore
                } else {
                   PRINT_DEBUG("extracting in root: " << name_str << std::endl);
                   struct archive_entry* extracted_entry;
                   extracted_entry = archive_entry_clone(file_entry);

                   // Create the pathname with the install root
                   std::string path_string = install_root + "/" + name_str;

                   archive_entry_set_pathname(extracted_entry, path_string.c_str());
                   // We can now begin copying the data
                   archive_write_header(extract, extracted_entry);
                   copy_data(package.a, extract);
                   archive_write_finish_entry(extract);
                   std::cout << "\33[2K\rOperating on " << package.name << ": " << ++copied_files << "/" << package.files.size() << '\r';
                   std::cout.flush();
                }
            }
        }
        archive_read_close(package.a);
        archive_read_free(package.a);
        archive_write_close(extract);
        archive_write_free(extract);

        // If this package has an after install script, we run it now
        if(package.has_after_install) {
            afterinstall_script_list.push_back(package);
        }
        std::cout << "\33[2K\rDone operating on " << package.name << std::endl;
    }
    if(afterinstall_script_list.empty()) { return true; }
    for(PackageFile package : afterinstall_script_list) {
        std::cout << "Running after install script for " << package.name << std::endl;
        std::cout.flush();
        // Generic fork/exec/wait
        pid_t pid = fork();
        if(pid > 0) {
            int status;
            waitpid(pid, &status, 0);
        } else {
            if(install_root != "/") {
                // We chroot if we need to
                if(chroot(install_root.c_str())) {
                    perror("failed to chroot to fakeroot");
                    exit(-1);
                } else {
                    PRINT_DEBUG("chrooted to " << install_root << std::endl);
                }
            }
            std::string path("/");
            path = path + "etc/bvpm/packages/" + package.name + "/afterinstall.sh";
            PRINT_DEBUG("trying to execute " << path << " as after install script" << std::endl);
            chroot("/");
            chdir("/");
            execl(path.c_str(), path.c_str(), NULL);
            perror(strcat("couldnt execute shell script for package ", package.name.c_str()));
            exit(-1);
        }
        std::cout << "\33[2K\rDone runinng after install for " << package.name << std::endl;
        std::cout.flush();
    }
    return true;
}

bool InstallEngine::VerifyPossible() {
    if(!dependencyEngine.CheckDependencies(package_list)) { return false; }
    // We now check and make sure that all files the packages want to install dont already exist
    bool passed = true;
    for(PackageFile package : package_list) {
        for(std::string file : package.files) {
            if(fs::exists(fs::path(install_root + file))) {
                std::cout << "Error: file " << install_root + file << " (part of package " << package.name << ") already exists" << std::endl;
                passed = false;
            }
        }
    }
    return passed;
}