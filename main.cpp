#include <iostream>
#include <flags.h>
#include <InstallEngine.h>
#include <UninstallEngine.h>
#include <DependencyEngine.h>
#include <config.h>
#include <debug.h>

void printHelp(char* argv0) {
    std::cout << "Usage: " << argv0 << " [--action=install, uninstall, query] [package-name-or-file] {--install-root=path-to-root} {--config-file=path-to-config}" << std::endl;
}

int main(int argc, char** argv) {
    const flags::args args(argc, argv);
    const std::string install_root = args.get<std::string>("install-root", "/");
    const std::string action = args.get<std::string>("action", "");
    const auto packages_to_operate = args.positional();

    if(action == "") {
        printHelp(argv[0]);
        exit(0);
    }

    // Attempt to read config file
    // The config file is always read from the install root
    ConfigFile config = Config::readConfigFile(args.get<std::string>("config-file", install_root + "/etc/bvpm/bvpm.cfg"));
    if(config.values["failed"] == "true") {
        std::cout << "failed to read config file " << args.get<std::string>("config-file", install_root + "/etc/bvpm/bvpm.cfg") << std::endl;
        exit(-1);
    }

    // Check arguments
    PRINT_DEBUG("install root: " << install_root << std::endl);
    if(action == "install") {
        InstallEngine installEngine(install_root);
        for(const auto& package : packages_to_operate) {
            if(!installEngine.AddPackage(std::string(package))) {
                exit(-1);
            }
        }
        if(installEngine.empty()) {
            std::cout << "error: no packages selected" << std::endl;
            exit(-1);
        }
        if(!installEngine.VerifyPossible()) {
            std::cout << "Failed to resolve dependencies, bailing!" << std::endl;
            exit(-1);
        }
        if(!args.get<bool>("y", false)) {
            if(!installEngine.GetUserPermission()) {
                std::cout << "Bailing" << std::endl;
                exit(-1);
            }
        }
        installEngine.Execute();
        std::cout << "Operations complete" << std::endl;
    } else if(action == "uninstall") {
        UninstallEngine uninstallEngine(install_root);
        for(const auto& package : packages_to_operate) {
            if(!uninstallEngine.AddToList(std::string(package))) {
                exit(-1);
            }
        }
        if(uninstallEngine.empty()) {
            std::cout << "error: no packages selected" << std::endl;
            exit(-1);
        }
        if(!args.get<bool>("y", false)) {
            if(!uninstallEngine.GetUserPermission()) {
                std::cout << "Bailing" << std::endl;
                exit(-1);
            }
        }
        uninstallEngine.Execute();
        std::cout << "Operations complete" << std::endl;
    } else if(action == "query") {
        InstallEngine installEngine(install_root);
        // Try to find the package
        for(const auto& package : packages_to_operate) {
            if(installEngine.dependencyEngine.installed_packages.find(std::string(package)) != installEngine.dependencyEngine.installed_packages.end()) {
                std::cout << package << ": " << installEngine.dependencyEngine.installed_packages[std::string(package)] << std::endl;
            }
        }
    } else {
        std::cout << "unknown action: " << action << std::endl;
    }
    return 0;
}
