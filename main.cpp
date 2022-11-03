#include <iostream>
#include <args.hxx>
#include <InstallEngine.h>
#include <UninstallEngine.h>
#include <DependencyEngine.h>
#include <config.h>
#include <debug.h>
#include "LocalFolderRepository.h"
#include "RepositoryEngine.h"


int main_bvpm_repo(int argc, char** argv) {
    args::ArgumentParser parser("BVPM is a simple package manager. It can install, uninstall, and query the version number of packages.", "This is the repo manager. Use it to add, remove, or update packages to a BVPM repository.");
    args::HelpFlag help(parser, "help", "Display this help menu", { 'h', "help" });
    args::Group flag_group(parser, "You must choose one of these:", args::Group::Validators::Xor);
    args::Flag add(flag_group, "add", "Add package file to repo", {'a', "add"});
    args::Flag remove(flag_group, "remove", "Remove package from repo", {'r', "remove"});
    args::Flag query(flag_group, "query", "Query packages in repo", {'q', "query"});

    args::ValueFlag<std::string> repository_arg(parser, "repository", "Path to repository folder", {'r', "repository"}, args::Options::Required);
    args::PositionalList<std::string> packages(parser, "packages", "Packages/Package files", args::Options::Required);

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
            error = "You must pass -a, -r or -q";
        } else {
            error = e.what();
        }
        std::cerr << "Failed validating arguments: " << error << std::endl;
        std::cout << parser;
        exit(1);
    }

    const std::string& repository = repository_arg.Get();
    PRINT_DEBUG("repository path: " << repository << std::endl);

    LocalFolderRepository repo("repository", repository);

    if(add.Get()) {
        for(const std::string& package : packages) {
            std::cout << "Adding package file " << package << " to repository" << std::endl;
            repo.addPackageFileToRepository(package);
        }
    } else if(remove.Get()) {
        for(const std::string& package : packages) {
            std::cout << "Removing package " << package << " from repository" << std::endl;
            repo.removePackageFromRepository(package);
        }
    } else if(query.Get()) {
        std::cerr << "TODO" << std::endl;
        exit(-1);
    }

    return 0;
}

int main(int argc, char** argv) {
    // If we are running as bvpm-repo, then we call a different main function
    if(argc && std::string(argv[0]) == "bvpm-repo") {
        return main_bvpm_repo(argc, argv);
    }

    args::ArgumentParser parser("BVPM is a simple package manager. It can install, uninstall, and query the version number of packages.", "To install a package, do: bvpm -i PACKAGE_FILE");
    args::HelpFlag help(parser, "help", "Display this help menu", { 'h', "help" });

    args::Group flag_group(parser, "You must choose one of these:", args::Group::Validators::Xor);
    args::Flag install(flag_group, "install", "Install packages", {'i', "install"});
    args::Flag uninstall(flag_group, "uninstall", "Uninstall packages", {'u', "uninstall"});
    args::Flag query(flag_group, "query", "Query package versions", {'q', "query"});

    args::Group only_for_query(parser, "Only for -q:", args::Group::Validators::DontCare);
    args::Flag query_all(only_for_query, "query-all", "List all packages", {"query-all"}, false);

    args::Flag dont_ask_for_permission(parser, "yes", "Skip asking for permission to perform actions", {'y', "yes"});
    args::Flag assume_inputs_are_files(parser, "files", "Assume that packages to install point directly to bvp files", {"files"});
    args::Flag ignore_dependencies(parser, "ignore-dependencies", "Do not account for dependencies", {"ignore-dependencies"});
    args::ValueFlag<std::string> install_root_arg(parser, "install-root", "Root folder to install to", {"install-root"}, "/");
    args::ValueFlag<std::string> config_file_arg(parser, "config-file", "Path to BVPM config file", {"config-file"}, "/etc/bvpm/bvpm.cfg");
    args::PositionalList<std::string> packages(parser, "packages", "Packages to install");

    try {
        parser.ParseCLI(argc, argv);
        if(packages->empty() && !query_all) {
            std::cerr << "Failed parsing arguments: missing packages list!\n";
            std::cout << parser;
            exit(1);
        }
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
        InstallEngine installEngine(install_root, config);
        if(assume_inputs_are_files.Get()) {
            for (const std::string& package: packages) {
                if (!installEngine.AddPackageFile(std::string(package))) {
                    exit(-1);
                }
            }
        } else {
            for (const std::string& package: packages) {
                if (!installEngine.AddPackage(std::string(package))) {
                    exit(-1);
                }
            }
        }
        if(installEngine.empty()) {
            std::cout << "error: no packages selected" << std::endl;
            exit(-1);
        }
        if(!installEngine.VerifyPossible()) {
            std::cout << "Failed to resolve dependencies during install stage; are there problems with the repositories? Bailing!" << std::endl;
            exit(-1);
        }
        if(!dont_ask_for_permission) {
            if(!installEngine.GetUserPermission()) {
                std::cout << "Bailing" << std::endl;
                exit(-1);
            }
        }
        if(installEngine.Execute()) {
            std::cout << "Operations complete" << std::endl;
        }
    } else if(uninstall) {
        UninstallEngine uninstallEngine(install_root);
        for(const auto& package : packages) {
            if(!uninstallEngine.AddToList(std::string(package), ignore_dependencies.Get())) {
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
        InstallEngine installEngine(install_root, config);
        if(query_all) {
            // Go through all packages
            for (const auto& package: installEngine.dependencyEngine.installed_packages) {
                std::cout << package.first << ": " << package.second << std::endl;
            }
        } else {
            // Try to find the package
            int num_notfound = 0;
            for (const auto& package: packages) {
                if (installEngine.dependencyEngine.installed_packages.find(std::string(package)) !=
                    installEngine.dependencyEngine.installed_packages.end()) {
                    std::cout << package << ": "
                              << installEngine.dependencyEngine.installed_packages[std::string(package)] << std::endl;
                } else {
                    std::cout << "package " << package << " not installed" << std::endl;
                    num_notfound++;
                }
                return num_notfound;
            }
        }
    }
    return 0;
}
