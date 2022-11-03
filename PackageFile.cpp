#include <iostream>
#include <PackageFile.h>
#include <cstring>
#include <archive.h>
#include <archive_entry.h>
#include <human-readable.h>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

bool PackageFile::readFile(std::string file, std::string display_name) {
    name = "";
    path = file;
    a = archive_read_new();
    archive_read_support_filter_all(a);
    archive_read_support_format_all(a);
    if(archive_read_open_filename(a, path.c_str(), 1024) != ARCHIVE_OK) {
        std::cout << "error reading package " << display_name << ": archive not ok" << std::endl;
        return false;
    }
    struct archive_entry* file_entry;
    bool has_manifest = false;
    bool has_owned_files = false;
    bool has_hashes = false;
    size_t file_size = fs::file_size(file);
    total_package_file_bytes = file_size;
    while(archive_read_next_header(a, &file_entry) == ARCHIVE_OK) {
        if(display_name != "") {
            std::cout << "\33[2K\rReading package " << display_name << ": " << humanSize(archive_filter_bytes(a, -1))
                      << "/" << humanSize(file_size);
        } else {
            std::cout << "\33[2K\rReading package file: " << humanSize(archive_filter_bytes(a, -1)) << "/" << humanSize(file_size);
        }
        std::cout.flush();
        std::string file_name = archive_entry_pathname(file_entry);
        total_package_bytes += archive_entry_size(file_entry);
        if(file_name == "manifest") {
            has_manifest = true;
            // We also try to read the manifest now
            size_t manifest_size = archive_entry_size(file_entry);
            char* data = (char*)malloc(manifest_size + 2);
            memset(data, 0, manifest_size + 1);
            archive_read_data(a, data, manifest_size);
            manifest = Config::readFromData(data);
        }
        if(file_name == "owned-files") {
            has_owned_files = true;
            // We also try to read the manifest now
            size_t owned_files_size = archive_entry_size(file_entry);
            char* data = (char*)malloc(owned_files_size + 2);
            memset(data, 0, owned_files_size + 1);
            archive_read_data(a, data, owned_files_size);

            // Split the owned-files by newline
            std::istringstream ss(data);
            std::string line;
            while(std::getline(ss, line, '\n')) {
                owned_files.push_back(line);
            }
        }
        if(file_name == "sums") {
            has_hashes = true;
            // We also try to read the manifest now
            size_t sums_size = archive_entry_size(file_entry);
            char* data = (char*)malloc(sums_size + 2);
            memset(data, 0, sums_size + 1);
            archive_read_data(a, data, sums_size);

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

                file_hashes[file_str] = hash;
            }
        }
        if(file_name == "afterinstall.sh") { has_after_install = true; }
        // We also make a record of all files and folders in root/
        if(file_name.rfind("root/", 0) == 0) {
            // Create a std::string and chop the root/ off
            std::string name_str(file_name.erase(0, strlen("root/")));
            if(name_str.empty()) { continue; }
            if(*(file_name.end()--) == '/') {
                //std::cout << "folder in root: " << name_str << std::endl;
                folders.push_back(name_str);
            } else {
                files.push_back(name_str);
            }
        }
        archive_read_data_skip(a);
    }

    if(has_manifest) {
        // We now parse the manifest (mostly to find the package name)
        if(manifest.values.find("PACKAGE") == manifest.values.end()) {
            std::cout << std::endl <<  "error adding package " << display_name << " to install list: manifest is missing package name" << std::endl;
            return false;
        } else {
            name = manifest.values["PACKAGE"];
            display_name = name;
        }
        if(manifest.values.find("VERSION") != manifest.values.end()) {
            version = manifest.values["VERSION"];
        }
        if(manifest.values.find("DEPENDENCY") != manifest.values.end() && !manifest.values["DEPENDENCY"].empty()) {
            std::stringstream ss(manifest.values["DEPENDENCY"]);
            while (ss.good()) {
                std::string package_dep;
                std::getline(ss, package_dep, ',');

                dependencies.push_back(package_dep);
            }
        }
    }
    if(!has_manifest || !has_owned_files) {
        std::cout << std::endl << "error reading package " << display_name << ": archive is missing required files" << std::endl;
        return false;
    }
    if(!has_hashes) {
        std::cout << std::endl << "warning reading package " << display_name << ": archive is missing hashes" << std::endl;
    }

    return true;
}

SimplePackageData PackageFile::toSimplePackageData() const {
    SimplePackageData ret;
    ret.name = name;
    ret.version = version;
    ret.dependencies = dependencies;
    ret.from_file = true;
    return ret;
}
