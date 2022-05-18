//
// Created by alexander on 5/18/22.
//
#include <filesystem>
#include <iostream>
#include <config.h>
#include <DependencyEngine.h>
#include <InstallEngine.h>
#include <debug.h>

namespace fs = std::filesystem;

void DependencyEngine::LoadInstalledPackages() {
    std::cout << "Loading installed package manifests" << std::endl;

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
}

bool DependencyEngine::CheckDependencies(std::vector<PackageFile>& packages) {
    bool passed = true;
    for(PackageFile package : packages) {
        // If the manifest has a dependencies section, then we check if they are already installed
        // If not, then we check if they are in the install list
        // If also not, then we bail
        // TODO: redo this if/when network install is added
        if(package.manifest.values.find("DEPENDENCY") != package.manifest.values.end()) {
            PRINT_DEBUG("checking dependencies for " << package.name << std::endl);
            std::stringstream ss(package.manifest.values["DEPENDENCY"]);
            while(ss.good()) {
                std::string package_dep;
                std::getline(ss, package_dep, ',');

                // We now check if this dependency is installed
                if(installed_packages.find(package_dep) == installed_packages.end()) {
                    std::cout << "Package " << package.name << " is missing dependency " << package_dep << std::endl;
                    passed = false;
                }
            }
        }
    }
    return passed;
}