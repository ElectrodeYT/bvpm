//
// Created by alexander on 5/18/22.
//

#ifndef BVPM_DEPENDENCYENGINE_H
#define BVPM_DEPENDENCYENGINE_H

#include <map>
#include <string>
#include <vector>
#include <config.h>
#include <RepositoryEngine.h>
#include <PackageFile.h>

class PackageFile;

class DependencyEngine {
public:
    DependencyEngine(std::string root) : install_root(root) { LoadInstalledPackages(); };

    bool CheckDependencies(std::vector<SimplePackageData>& packages, RepositoryEngine& repositoryEngine);
    std::map<std::string, std::string> installed_packages;
    ConfigFile LoadPackageManifest(std::string name);
    std::vector<std::string> GetPackageOwnedFiles(std::string name);
    std::vector<std::string> GetDependedPackages(std::string name_to_compare);
    size_t GetPackageSize(std::string name);
private:
    void InsertPackageIntoListSorted(const std::string& name, std::vector<SimplePackageData>& all_packages, std::vector<SimplePackageData>& sorted_packages);
    void LoadInstalledPackages();
    std::string install_root;

    std::map<std::string, ConfigFile> packageManifests;
    std::map<std::string, std::vector<std::string>> packageOwnedFiles;
    std::map<std::string, size_t> packageSize;
};


#endif //BVPM_DEPENDENCYENGINE_H
