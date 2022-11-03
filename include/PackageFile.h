//
// Created by alexander on 11/1/22.
//

#ifndef BVPM_PACKAGEFILE_H
#define BVPM_PACKAGEFILE_H

#include <string>
#include <vector>
#include <map>
#include <config.h>


/// This is a simplified version of PackageFile, without a file actually backing it.
/// All repo types must be able to provide the data here instantly, without any network activity
/// or opening of large files.
struct SimplePackageData {
    std::string name;
    std::string version;
    std::vector<std::string> dependencies;
    size_t total_package_bytes = 0;
    size_t total_package_file_bytes = 0;

    /// file_hash may be "", in which case there is no hash available.
    std::string file_hash;

    bool from_file = false;
};

struct PackageFile {
    bool readFile(std::string file, std::string display_name = "");

    struct archive* a;
    std::vector<std::string> folders;
    std::vector<std::string> files;
    std::string name;
    std::string version = "";
    std::string path;

    size_t total_package_bytes = 0;
    size_t total_package_file_bytes = 0;
    ConfigFile manifest;

    bool has_after_install = false;
    std::vector<std::string> dependencies;
    std::vector<std::string> owned_files;
    std::map<std::string, std::string> file_hashes;

    [[nodiscard]] SimplePackageData toSimplePackageData() const;
};

#endif //BVPM_PACKAGEFILE_H
