//
// Created by alexander on 5/20/22.
//

#ifndef BVPM_UNINSTALLENGINE_H
#define BVPM_UNINSTALLENGINE_H
#include <DependencyEngine.h>

class UninstallEngine {
public:
    UninstallEngine(std::string root) : dependencyEngine(root), install_root(root) { }
    bool AddToList(std::string name);
    bool GetUserPermission();
    bool Execute();

    bool empty() { return uninstall_list.empty(); }

    DependencyEngine dependencyEngine;
private:
    std::map<std::string, std::vector<std::string>> uninstall_list;
    std::string install_root;
};


#endif //BVPM_UNINSTALLENGINE_H
