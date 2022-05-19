//
// Created by alexander on 5/18/22.
//

#ifndef BVPM_INSTALLENGINE_H
#define BVPM_INSTALLENGINE_H

#include <string>
#include <vector>
#include <archive.h>
#include <archive_entry.h>
#include <config.h>
#include <DependencyEngine.h>

struct PackageFile {
    struct archive* a;
    std::vector<std::string> folders;
    std::vector<std::string> files;
    std::string name;
    std::string version = "";
    std::string path;

    size_t total_package_bytes = 0;
    ConfigFile manifest;

    bool has_after_install = false;
    std::vector<std::string> dependencies;
    std::vector<std::string> owned_files;
};

class InstallEngine {
public:
    InstallEngine(const std::string root) : install_root(root), dependencyEngine(root) { }

    bool AddPackage(std::string package);
    bool VerifyPossible();
    bool Execute();
    bool GetUserPermission();

    bool empty() { return package_list.empty(); }

    DependencyEngine dependencyEngine;
private:

    const std::string install_root;
    std::vector<PackageFile> package_list;
};


#endif //BVPM_INSTALLENGINE_H
