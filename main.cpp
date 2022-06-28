#include <iostream>
#include <args.hxx>
#include <InstallEngine.h>
#include <UninstallEngine.h>
#include <DependencyEngine.h>
#include <config.h>
#include <debug.h>


int main(int argc, char** argv) {
    args::ArgumentParser parser("BVPM is a simple package manager. It can install, uninstall, and query the version number of packages.", "To install a package, do: bvpm -i PACKAGE_FILE");
    args::HelpFlag help(parser, "help", "Display this help menu", { 'h', "help" });
    args::Group flag_group(parser, "You must choose one of these:", args::Group::Validators::Xor);
    args::Flag install(flag_group, "install", "Install packages", {'i', "install"});
    args::Flag uninstall(flag_group, "uninstall", "Uninstall packages", {'u', "uninstall"});
    args::Flag query(flag_group, "query", "Query package versions", {'q', "query"});
    args::Flag dont_ask_for_permission(parser, "yes", "Skip asking for permission to perform actions", {'y', "yes"});
    args::ValueFlag<std::string> install_root_arg(parser, "install-root", "Root folder to install to", {"install-root"}, "/");
    args::ValueFlag<std::string> config_file_arg(parser, "config-file", "Path to BVPM config file", {"config-file"}, "/etc/bvpm/bvpm.cfg");
    args::PositionalList<std::string> packages(parser, "packages", "Packages to install", args::Options::Required);

    try {
        parser.ParseCLI(argc, argv);
    } catch(args::Help&) {
        std::cout << parser;
        exit(0);
    } catch(args::ParseError& e) {
        std::cerr << "Failed parsing arguments: " << e.what() << std::endl;
        std::cout << parser;
        exit(1);
    } catch(args::ValidationError& e) {
        std::string error;
        if(std::string(e.what()) == "Group validation failed somewhere!") {
            // Hacky workaround to give a decent error message
            error = "You must pass -i, -u or -q";
        } else {
            error = e.what();
        }
        std::cerr << "Failed validating arguments: " << error << std::endl;
        std::cout << parser;
        exit(1);
    }

    const std::string& install_root = install_root_arg.Get();
    const std::string& config_file = config_file_arg.Get();
    // Attempt to read config file
    // The config file is always read from the install root
    ConfigFile config = Config::readConfigFile(config_file);
    if(config.values["failed"] == "true") {
        std::cout << "failed to read config file " << config_file << std::endl;
        exit(-1);
    }

    // Check arguments
    PRINT_DEBUG("install root: " << install_root << std::endl);
    if(install) {
        InstallEngine installEngine(install_root);
        std::cout << "Starting install engine";
        for(const std::string& package : packages) {
            if(!installEngine.AddPackage(std::string(package))) {
                exit(-1);
            }
        }
        if(!packages->empty()) { std::cout << "\33[2K\rDone reading packages" << std::endl; }
        if(installEngine.empty()) {
            std::cout << "error: no packages selected" << std::endl;
            exit(-1);
        }
        if(!installEngine.VerifyIntegrity()) {
            std::cout << "Archive corrupted, bailing!" << std::endl;
        }
        if(!installEngine.VerifyPossible()) {
            std::cout << "Failed to resolve dependencies, bailing!" << std::endl;
            exit(-1);
        }
        if(!dont_ask_for_permission) {
            if(!installEngine.GetUserPermission()) {
                std::cout << "Bailing" << std::endl;
                exit(-1);
            }
        }
        installEngine.Execute();
        std::cout << "Operations complete" << std::endl;
    } else if(uninstall) {
        UninstallEngine uninstallEngine(install_root);
        for(const auto& package : packages) {
            if(!uninstallEngine.AddToList(std::string(package))) {
                exit(-1);
            }
        }
        if(uninstallEngine.empty()) {
            std::cout << "error: no packages selected" << std::endl;
            exit(-1);
        }
        if(!dont_ask_for_permission) {
            if(!uninstallEngine.GetUserPermission()) {
                std::cout << "Bailing" << std::endl;
                exit(-1);
            }
        }
        uninstallEngine.Execute();
        std::cout << "Operations complete" << std::endl;
    } else if(query) {
        InstallEngine installEngine(install_root);
        // Try to find the package
        int num_notfound = 0;
        for(const auto& package : packages) {
            if(installEngine.dependencyEngine.installed_packages.find(std::string(package)) != installEngine.dependencyEngine.installed_packages.end()) {
                std::cout << package << ": " << installEngine.dependencyEngine.installed_packages[std::string(package)] << std::endl;
            } else {
                std::cout << "package " << package << " not installed" << std::endl;
                num_notfound++;
            }
            return num_notfound;
        }
    }
    return 0;
}
