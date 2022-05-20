//
// Created by alexander on 5/20/22.
//

#include <iostream>
#include <UninstallEngine.h>
#include <human-readable.h>
#include <debug.h>
#include <filesystem>

namespace fs = std::filesystem;

bool UninstallEngine::AddToList(std::string name) {
    // Check if this package is already in our list
    // If it is, then we don't need to readd it
    if(uninstall_list.find(name) != uninstall_list.end()) { return true; }
    // Try to read the manifest
    // If this fails than we can assume that this failed
    ConfigFile manifest = dependencyEngine.LoadPackageManifest(name);
    if(manifest.values.find("failed") != manifest.values.end()) {
        std::cout << "No package called " << name << " found" << std::endl;
        return false;
    }
    std::vector<std::string> owned_files = dependencyEngine.GetPackageOwnedFiles(name);
    if(owned_files.empty()) {
        std::cout << "Package " << name << " has no files" << std::endl;
        return false;
    }
    // Try to find all packages that depend on this package
    // We need to delete them all
    for(std::string depended : dependencyEngine.GetDependedPackages(name)) {
        PRINT_DEBUG("trying to add " << depended << " to list" << std::endl);
        if(!AddToList(depended)) {
            std::cout << "Failed to add depended package " << depended << " (package " << name << ") to uninstall list" << std::endl;
            return false;
        }
    }
    // If we are here, we can add this package to the uninstall list
    uninstall_list[name] = owned_files;
    return true;
}

bool UninstallEngine::GetUserPermission() {
    if(empty()) { return false; }
    std::cout << "The following packages will be uninstalled: " << std::endl;
    size_t total_size = 0;
    for(std::pair<std::string, std::vector<std::string>> package : uninstall_list) {
        size_t size = dependencyEngine.GetPackageSize(package.first);
        std::cout << "\t" << package.first << " (size: " << size << ")" << std::endl;
        total_size += size;
    }
    std::cout << "Total size of packages: " << humanSize(total_size) << "\n";
    std::cout << "Are you sure? [y/N] ";
    fflush(stderr);
    fflush(stdout);
    char response = getchar();
    if(response == 'y' || response == 'Y') { return true; }
    return false;
}

bool UninstallEngine::Execute() {
    for(std::pair<std::string, std::vector<std::string>> package : uninstall_list) {
        const std::string& name = package.first;
        std::cout << "Operating on " << name;
        size_t count = 0;
        const size_t& file_count = package.second.size();
        for(std::string file : package.second) {
            std::cout << "\33[2K\rOperating on " << name << ": " << ++count << "/" << file_count << '\r';
            try {
                fs::remove(install_root + file);
            } catch(fs::filesystem_error& e) {
                std::cout << "Failed to remove file " << install_root + file << ": " << e.what() << std::endl;
            }
        }
        // Remove the package folder
        try {
            fs::remove_all(fs::path(install_root + "/etc/bvpm/packages/" + name));
        } catch(fs::filesystem_error& e) {
            std::cout << "Failed to remove package folder " << install_root + "/etc/bvpm/packages/" + name << ": " << e.what() << std::endl;
        }
        std::cout << "\33[2K\rDone operating on " << name << std::endl;
    }
    return true;
}