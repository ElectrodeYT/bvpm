//
// Created by alexander on 11/1/22.
//

#include <iostream>
#include <filesystem>
#include <utility>
#include <RepositoryEngine.h>
#include <LocalFolderRepository.h>
#include "human-readable.h"

namespace fs = std::filesystem;

RepositoryEngine::RepositoryEngine(const ConfigFile& globalConfigFile, std::string _install_root) : install_root(std::move(_install_root)) {
    // We look through the config file to find repository references
    // The name after the _ is only for humans, we ignore it and use the name referenced in
    // the manifest file
    for(const std::pair<std::string, std::string> config : globalConfigFile.values) {
        if(config.first.rfind("REPOSITORY_", 0) == 0) {
            Repository* repo = new LocalFolderRepository("local-folder-repo", config.second);
            if(!repo->good()) {
                delete repo; continue;
            }
            repositories.push_back(repo);
        }
    }
}

bool RepositoryEngine::isPackageInRepos(const std::string& package_name) {
    for(Repository* repo : repositories) {
        if(repo->checkIfPackageIsAvailable(package_name)) { return true; }
    }
    return false;
}

Repository* RepositoryEngine::findBestRepoForPackage(const std::string& package_name) {
    for(Repository* repo : repositories) {
        if(repo->checkIfPackageIsAvailable(package_name)) { return repo; }
    }
    return nullptr;
}

bool RepositoryEngine::preparePackage(const std::string& package_name) {
    Repository* repo = findBestRepoForPackage(package_name);
    if(!repo) { return false; }
    return repo->preparePackage(package_name);
}

std::string RepositoryEngine::getBVPFileForPackage(const std::string& package_name) {
    Repository* repo = findBestRepoForPackage(package_name);
    if(!repo) { return ""; }
    return repo->getPackageBVPFilePath(package_name);
}

bool RepositoryEngine::GetUserPermission(const std::vector<std::string>& packages) {
    std::cout << "The following packages will be fetched and installed: " << std::endl;
    size_t total_size = 0;
    size_t total_file_size = 0;
    for(const std::string& package : packages) {
        std::cout << "\t" << package << " (size: " << humanSize(getPackageTotalSize(package))
        << ", file size: " << humanSize(getPackageFileSize(package)) << ")" << std::endl;
        total_size += getPackageTotalSize(package);
        total_file_size += getPackageFileSize(package);
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

std::string RepositoryEngine::getPackageVersion(const std::string& package_name) {
    Repository* repo = findBestRepoForPackage(package_name);
    if(!repo) { return ""; }
    return repo->getPackageVersion(package_name);
}

size_t RepositoryEngine::getPackageFileSize(const std::string& package_name) {
    Repository* repo = findBestRepoForPackage(package_name);
    if(!repo) { return 0; }
    return repo->getPackageFileSize(package_name);
}

size_t RepositoryEngine::getPackageTotalSize(const std::string& package_name) {
    Repository* repo = findBestRepoForPackage(package_name);
    if(!repo) { return 0; }
    return repo->getPackageTotalSize(package_name);
}

SimplePackageData RepositoryEngine::getSimplePackageData(const std::string& package_name) {
    SimplePackageData ret;
    ret.name = package_name;
    ret.version = getPackageVersion(package_name);
    ret.dependencies = getPackageDependencies(package_name);
    ret.total_package_bytes = getPackageTotalSize(package_name);
    ret.total_package_file_bytes = getPackageFileSize(package_name);
    return ret;
}

std::vector<std::string> RepositoryEngine::getPackageDependencies(const std::string& package_name) {
    Repository* repo = findBestRepoForPackage(package_name);
    if(!repo) { return {}; }
    return repo->getPackageDependencies(package_name);
}
