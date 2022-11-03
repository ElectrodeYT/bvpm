//
// Created by alexander on 11/1/22.
//

#ifndef BVPM_REPOSITORYENGINE_H
#define BVPM_REPOSITORYENGINE_H

#include <config.h>
#include <Repository.h>
#include <PackageFile.h>

class RepositoryEngine {
public:
    explicit RepositoryEngine(const ConfigFile& globalConfigFile, std::string _install_root);

    bool isPackageInRepos(const std::string& package_name);
    std::string getPackageVersion(const std::string& package_name);

    bool preparePackage(const std::string& package_name);
    std::string getBVPFileForPackage(const std::string& package_name);
    size_t getPackageFileSize(const std::string& package_name);
    size_t getPackageTotalSize(const std::string& package_name);
    std::vector<std::string> getPackageDependencies(const std::string& package_name);
    SimplePackageData getSimplePackageData(const std::string& package_name);

    bool GetUserPermission(const std::vector<std::string>& packages);
private:
    Repository* findBestRepoForPackage(const std::string& package_name);

    std::vector<Repository*> repositories;

    std::string install_root;
};


#endif //BVPM_REPOSITORYENGINE_H
