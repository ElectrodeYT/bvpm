#include <iostream>
#include <flags.h>
#include <InstallEngine.h>
#include <DependencyEngine.h>
#include <config.h>
#include <debug.h>

void printHelp(char* argv0) {
    std::cout << "Usage: " << argv0 << " [install, uninstall, query] [package-name-or-file] {--install-root=path-to-root}" << std::endl;
}

int main(int argc, char** argv) {
    const flags::args args(argc, argv);
    const std::string install_root = args.get<std::string>("install-root", "/");
    const std::string action = args.get<std::string>("action", "");
    const auto packages_to_operate = args.positional();

    InstallEngine installEngine(install_root);

    // Attempt to read config file
    // The config file is always read from the install root
    ConfigFile config = Config::readConfigFile(install_root + "/etc/bvpm/bvpm.cfg");
    if(config.values["failed"] == "true") {
        std::cout << "failed to read config file!" << std::endl;
        exit(-1);
    }

    // Check arguments
    PRINT_DEBUG("install root: " << install_root << std::endl);
    if(action == "install") {
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

    } else if(action == "query") {
        // Try to find the package
        for(const auto& package : packages_to_operate) {
            if(installEngine.dependencyEngine.installed_packages.find(std::string(package)) != installEngine.dependencyEngine.installed_packages.end()) {
                std::cout << package << ": " << installEngine.dependencyEngine.installed_packages[std::string(package)] << std::endl;
            }
        }
    } else if(action == "") {
        printHelp(argv[0]);
    } else {
        std::cout << "unknown action: " << action << std::endl;
    }
    return 0;
}
