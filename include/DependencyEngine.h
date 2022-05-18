//
// Created by alexander on 5/18/22.
//

#ifndef BVPM_DEPENDENCYENGINE_H
#define BVPM_DEPENDENCYENGINE_H

#include <map>
#include <string>
#include <vector>

class PackageFile;

class DependencyEngine {
public:
    DependencyEngine(std::string root) : install_root(root) { LoadInstalledPackages(); };

    bool CheckDependencies(std::vector<PackageFile>& packages);
    std::map<std::string, std::string> installed_packages;

private:
    void LoadInstalledPackages();
    std::string install_root;
};


#endif //BVPM_DEPENDENCYENGINE_H
