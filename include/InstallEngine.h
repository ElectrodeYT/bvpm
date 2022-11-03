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
#include <PackageFile.h>
#include "RepositoryEngine.h"

class InstallEngine {
public:
    InstallEngine(const std::string root, ConfigFile global_config_file) : dependencyEngine(root), repositoryEngine(global_config_file, root),
                                                                           install_root(root) { }

    bool AddPackageFile(std::string package);
    bool AddPackage(const std::string& package_file);
    bool VerifyPossible();
    bool VerifyIntegrity();
    bool Execute();
    bool GetUserPermission();

    bool empty() { return package_list.empty() && packages_by_name_list.empty(); }

    DependencyEngine dependencyEngine;
    RepositoryEngine repositoryEngine;
private:

    const std::string install_root;
    std::vector<PackageFile> package_list;
    std::vector<std::string> packages_by_name_list;
    std::vector<SimplePackageData> all_packages_to_install;
};


#endif //BVPM_INSTALLENGINE_H
