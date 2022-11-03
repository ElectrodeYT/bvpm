//
// Created by alexander on 11/1/22.
//

#ifndef BVPM_LOCALFOLDERREPOSITORY_H
#define BVPM_LOCALFOLDERREPOSITORY_H

#include <Repository.h>

#include <utility>
#include "config.h"

class LocalFolderRepository : public Repository {
public:
    LocalFolderRepository(std::string name, std::string _path);

    bool good() override;

    bool checkIfPackageIsAvailable(const std::string& package_name) override;
    bool preparePackage(const std::string& package) override { return true; }
    std::string getPackageBVPFilePath(const std::string& package_name) override;
    std::string getPackageVersion(const std::string& package_name) override;
    size_t getPackageFileSize(const std::string& package_name) override;
    size_t getPackageTotalSize(const std::string& package_name) override;
    std::vector<std::string> getPackageDependencies(const std::string& package_name) override;

    bool addPackageFileToRepository(const std::string& package_file) override;
    bool removePackageFromRepository(const std::string& package_name) override;
    ConfigFile getManifestFile(const std::string& package_name);
private:
    bool _good = true; // By default, we consider the repo to be good, and set it to false in case of an error

    std::string path_str;
};


#endif //BVPM_LOCALFOLDERREPOSITORY_H
