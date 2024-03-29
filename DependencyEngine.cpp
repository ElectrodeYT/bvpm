//
// Created by alexander on 5/18/22.
//
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <config.h>
#include <DependencyEngine.h>
#include <InstallEngine.h>
#include <debug.h>

namespace fs = std::filesystem;

void DependencyEngine::LoadInstalledPackages() {
    std::cout << "Loading installed package manifests... ";

    // First, we generate a list of package folders in /etc/bvpm/packages
    // While these should be the same as the package name, as we need to read the package manifest anyway to get the
    // version, we just store these temporarily
    std::vector<std::string> folders;
    for(auto& p : fs::recursive_directory_iterator(install_root + "/etc/bvpm/packages")) {
        if(p.is_directory()) { folders.push_back(p.path().string()); }
    }

    // We can now iterate through each folder, open its manifest, and try to read the installed package name and ver
    for(std::string package : folders) {
        std::string manifest_file_name = package + "/manifest";
        ConfigFile manifest = Config::readConfigFile(manifest_file_name);
        std::string name;
        std::string version = "";
        if(manifest.values.find("failed") != manifest.values.end()) {
            std::cout << "couldnt open manifest file " << manifest_file_name << std::endl;
            continue;
        }
        if(manifest.values.find("PACKAGE") == manifest.values.end()) {
            std::cout << "package folder " << manifest_file_name << " has corrupted manifest: no package name" << std::endl;
            continue;
        } else {
            name = manifest.values["PACKAGE"];
        }
        if(manifest.values.find("VERSION") != manifest.values.end()) {
            version = manifest.values["VERSION"];
        }
        installed_packages[name] = version;
        PRINT_DEBUG("installed package: " << name << ", version: " << version << std::endl);
    }
    std::cout << installed_packages.size() << " installed packages." << std::endl;
}

bool DependencyEngine::CheckDependencies(std::vector<SimplePackageData>& packages, RepositoryEngine& repositoryEngine) {
    bool passed = true;
    bool sort_req = false;
    for(SimplePackageData& package : packages) {
        // If the manifest has a dependencies section, then we check if they are already installed
        // If not, then we check if they are in the install list
        // If also not, then we check if we can add it to the package list
        // If even then not, we bail
        PRINT_DEBUG(package.name << std::endl);
        for(const std::string& package_dep : package.dependencies) {
            PRINT_DEBUG("\t" << package_dep << std::endl);
            // We now check if this dependency is installed
            if(installed_packages.find(package_dep) == installed_packages.end()) {
                // If we dont have the dependency, then we can go and check if we are also about to install it:
                // TODO: when network works, add some shit for this
                bool found = false;
                for(SimplePackageData depsearch : packages) {
                    if(depsearch.name == package_dep) {
                        // If they have us in their dep list, than we report this and cut the dependency between them and us
                        if(std::find(depsearch.dependencies.begin(), depsearch.dependencies.end(), package.name) != depsearch.dependencies.end()) {
                            std::cout << "Package " << package_dep << " has a circular dependency with us, breaking." << std::endl;
                            // TODO
                            found = true;
                            break;
                        }
                        found = true;
                        sort_req = true;
                        // PRINT_DEBUG("added dep " << depsearch.name << " to package " << package.name << ", dep count now " << package.dependencies.size() << std::endl);
                        break;
                    }
                }
                if(!found) {
                    // We now ask the repository manager if this package is available
                    if(!repositoryEngine.isPackageInRepos(package_dep)) {
                        std::cout << "Package " << package.name << " is missing dependency " << package_dep << std::endl;
                        passed = false;
                    } else {
                        // We can add it to the package dep list
                        PRINT_DEBUG("\t\tadded" << std::endl);
                        packages.push_back(repositoryEngine.getSimplePackageData(package_dep));
                        sort_req = true;
                    }
                }
            }
        }
    }
    if(!passed) { return false; }
    // We can now perform a toplogical sort.
    if(!sort_req) { return passed; }
//    for(PackageFile& package : packages) {
//        std::cout << package.name << std::endl;
//        for(std::string dep : package.dependencies) {
//            std::cout << "-- " << dep << std::endl;
//        }
//    }
    std::cout << "Sorting dependencies..." << std::endl;
    std::vector<SimplePackageData> sorted_packages;
    while(!packages.empty()) {
        InsertPackageIntoListSorted((*packages.begin()).name, packages, sorted_packages);
    }
//    for(PackageFile& package : sorted_packages) {
//        std::cout << package.name << std::endl;
//        for(std::string dep : package.dependencies) {
//            std::cout << "-- " << dep << std::endl;
//        }
//    }
    packages = sorted_packages;
    return passed;
}

void DependencyEngine::InsertPackageIntoListSorted(const std::string& name, std::vector<SimplePackageData>& all_packages, std::vector<SimplePackageData>& sorted_packages) {
    auto package = std::find_if(all_packages.begin(), all_packages.end(), [name](SimplePackageData pack) {
        return name == pack.name;
    });
    if(package != all_packages.end()) {
        SimplePackageData obj = *package;
        all_packages.erase(package);
        // PRINT_DEBUG("inserting for pack " << obj.name << std::endl);
        // PRINT_DEBUG(" dep count: " << obj.dependencies.size() << std::endl);
        for(const std::string& dep : obj.dependencies) {
            // PRINT_DEBUG("inserting dep " << dep << " for " << obj.name << std::endl);
            InsertPackageIntoListSorted(dep, all_packages, sorted_packages);
        }
        sorted_packages.push_back(obj);
    }
}

ConfigFile DependencyEngine::LoadPackageManifest(std::string name) {
    // First we check if we have this cached
    if(packageManifests.find(name) != packageManifests.end()) {
        return packageManifests[name];
    }
    // If we dont, we can just return the correct call to readConfigFile
    ConfigFile config = Config::readConfigFile(install_root + "/etc/bvpm/packages/" + name + "/manifest");
    packageManifests[name] = config;
    return config;
}

std::vector<std::string> DependencyEngine::GetPackageOwnedFiles(std::string name) {
    // First we check if we have this cached
    if(packageOwnedFiles.find(name) != packageOwnedFiles.end()) {
        return packageOwnedFiles[name];
    }
    // We have to now getline the file a lot, to read everyline into a string that we save
    std::ifstream file(install_root + "/etc/bvpm/packages/" + name + "/owned-files");
    std::vector<std::string> lines;
    // If we opened succesfully, than we read the file
    // otherwise we just return a nothing
    if(file.is_open()) {
        std::string line;
        while(std::getline(file, line, '\n')) {
            lines.push_back(line);
        }
    }
    packageOwnedFiles[name] = lines;
    return lines;
}

std::vector<std::string> DependencyEngine::GetDependedPackages(std::string name_to_compare) {
    std::vector<std::string> ret;
    for(std::pair<std::string, std::string> package : installed_packages) {
        const std::string& name = package.first;
        ConfigFile manifest = LoadPackageManifest(name);
        if(manifest.values.find("DEPENDENCY") != manifest.values.end()) {
            std::stringstream ss(manifest.values["DEPENDENCY"]);
            while (ss.good()) {
                std::string package_dep;
                std::getline(ss, package_dep, ',');
                // We now check if this dependency is installed
                if (package_dep == name_to_compare) {
                    ret.push_back(name);
                }
            }
        }
    }
    return ret;
}

size_t DependencyEngine::GetPackageSize(std::string name) {
    std::vector<std::string> files = GetPackageOwnedFiles(name);
    size_t size = 0;
    for(std::string file : files) {
        try {
            if(fs::is_symlink(fs::path(install_root) += file)) { continue; }
            size += fs::file_size(fs::path(install_root) += file);
        } catch(fs::filesystem_error& e) {
            std::cout << "Error getting size of file " << (fs::path(install_root) += file) << ": " << e.what() << std::endl;
        }
    }
    return size;
}