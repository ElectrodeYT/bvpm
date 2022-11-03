//
// Created by alexander on 11/1/22.
//

#include <iostream>
#include <filesystem>
#include <utility>
#include <LocalFolderRepository.h>
#include <debug.h>
#include <PackageFile.h>

namespace fs = std::filesystem;

bool LocalFolderRepository::checkIfPackageIsAvailable(const std::string& package_name) {
    if(!good()) { return false; }

    // If manifests/package_name exists, then the package is in the repo
    auto path = fs::path(path_str) / "manifests" / package_name / "manifest";
    return fs::exists(path);
}

bool LocalFolderRepository::addPackageFileToRepository(const std::string& package_file) {
    if(!good()) { return false; }

    PackageFile file;
    if(!file.readFile(package_file)) { return false; }

    if(checkIfPackageIsAvailable(file.name)) {
        // We have to remove the current one first
        // TODO: we should be able to have logic to update the manifest and maintain old versions
        PRINT_DEBUG("Removing old package files first" << std::endl);
        removePackageFromRepository(file.name);
    }

    // We are now going to create the manifest file
    // We don't need the owned-files, or the sums file
    auto manifest_package_folder_path = fs::path(path_str) / "manifests" / file.name;
    auto bvp_files_package_folder_path = fs::path(path_str) / "packages" / file.name;

    // We now ensure that these folders exist
    fs::create_directories(manifest_package_folder_path);
    fs::create_directories(bvp_files_package_folder_path);

    auto manifest_file_path = manifest_package_folder_path / "manifest";

    auto bvp_file_path = bvp_files_package_folder_path / fs::path(package_file).filename();

    // This string has the contents of the new manifest file in it
    std::string package_repo_manifest = "NAME=" + file.name + "\n";
    package_repo_manifest += "NEWEST_VERSION=" + file.version + "\n";
    package_repo_manifest += "INSTALLED_SIZE=" + std::to_string(file.total_package_bytes) + "\n";
    package_repo_manifest += "FILE_SIZE=" + std::to_string(file.total_package_file_bytes) + "\n";
    package_repo_manifest += "ARCH=todo\n";

    package_repo_manifest += "DEPENDENCIES=";
    std::cout << "amount dependencies: " << file.dependencies.size() << std::endl;
    for(size_t i = 0; i < file.dependencies.size(); i++) {
        package_repo_manifest += file.dependencies[i];
        if(i < (file.dependencies.size() - 1)) { package_repo_manifest += ","; }
    }
    package_repo_manifest += "\n";

    // We now add the package file names for each version
    // For now, we only have the one version
    package_repo_manifest += "FILENAME_" + file.version + "=" + bvp_file_path.filename().generic_string() + "\n";

    {
        std::ofstream package_repo_manifest_stream(manifest_file_path);
        package_repo_manifest_stream << package_repo_manifest;
        package_repo_manifest_stream.flush();
        package_repo_manifest_stream.close();
    }

    // Now we copy the bvp file to the correct location
    std::cout << "Copying BVP file to " << bvp_file_path << std::endl;
    if(fs::exists(bvp_file_path)) { fs::remove(bvp_file_path); }
    fs::copy(fs::path(package_file), bvp_file_path);

    return true;
}

bool LocalFolderRepository::removePackageFromRepository(const std::string& package_name) {
    if(!good() || !checkIfPackageIsAvailable(package_name)) { return false; }

    // We simply delete the two folders containing the package files and the manigests
    auto manifest_package_folder_path = fs::path(path_str) / "manifests" / package_name;
    auto bvp_files_package_folder_path = fs::path(path_str) / "packages" / package_name;

    if(fs::exists(manifest_package_folder_path)) {
        if(fs::is_directory(manifest_package_folder_path)) {
            fs::remove_all(manifest_package_folder_path);
        } else {
            fs::remove(manifest_package_folder_path);
        }
    }

    if(fs::exists(bvp_files_package_folder_path)) {
        if(fs::is_directory(bvp_files_package_folder_path)) {
            fs::remove_all(bvp_files_package_folder_path);
        } else {
            fs::remove(bvp_files_package_folder_path);
        }
    }

    return true;
}

std::string LocalFolderRepository::getPackageBVPFilePath(const std::string& package_name) {
    if(!good() || !checkIfPackageIsAvailable(package_name)) { return ""; }

    ConfigFile manifest = getManifestFile(package_name);

    if(manifest.values.find("NEWEST_VERSION") == manifest.values.end()) { return ""; }
    std::string filename_manifest_entry = "FILENAME_" + manifest.values["NEWEST_VERSION"];
    if(manifest.values.find(filename_manifest_entry) == manifest.values.end()) { return ""; }

    auto bvp_files_package_folder_path = fs::path(path_str) / "packages" / package_name;
    auto bvp_file_path = bvp_files_package_folder_path / manifest.values[filename_manifest_entry];
    return bvp_file_path;
}

std::string LocalFolderRepository::getPackageVersion(const std::string& package_name) {
    if(!good() || !checkIfPackageIsAvailable(package_name)) { return ""; }

    ConfigFile manifest = getManifestFile(package_name);

    if(manifest.values.find("NEWEST_VERSION") == manifest.values.end()) { return ""; }
    return manifest.values["NEWEST_VERSION"];
}

size_t LocalFolderRepository::getPackageFileSize(const std::string& package_name) {
    if(!good() || !checkIfPackageIsAvailable(package_name)) { return 0; }

    ConfigFile manifest = getManifestFile(package_name);

    if(manifest.values.find("FILE_SIZE") == manifest.values.end()) { return 0; }
    return std::atoll(manifest.values["FILE_SIZE"].c_str());
}

size_t LocalFolderRepository::getPackageTotalSize(const std::string& package_name) {
    if(!good() || !checkIfPackageIsAvailable(package_name)) { return 0; }

    ConfigFile manifest = getManifestFile(package_name);

    if(manifest.values.find("INSTALLED_SIZE") == manifest.values.end()) { return 0; }
    return std::atoll(manifest.values["INSTALLED_SIZE"].c_str());
}

std::vector<std::string> LocalFolderRepository::getPackageDependencies(const std::string& package_name) {
    if(!good() || !checkIfPackageIsAvailable(package_name)) { return {}; }

    ConfigFile manifest = getManifestFile(package_name);

    std::vector<std::string> ret;
    if(manifest.values.find("DEPENDENCIES") != manifest.values.end() && !manifest.values["DEPENDENCIES"].empty()) {
        std::stringstream ss(manifest.values["DEPENDENCIES"]);
        while (ss.good()) {
            std::string package_dep;
            std::getline(ss, package_dep, ',');

            ret.push_back(package_dep);
        }
    }
    return ret;
}

bool LocalFolderRepository::good() {
    return _good;
}

LocalFolderRepository::LocalFolderRepository(std::string _name, std::string _path) : Repository(std::move(_name)), path_str(std::move(_path)) {
    auto path = fs::path(path_str);
    path_str = fs::absolute(path).generic_string();
    PRINT_DEBUG("repo path: " + path_str << std::endl);

    auto repo_manifest_path = path / "repo.manifest";
    if(!fs::exists(repo_manifest_path)) {
        std::cerr << "error reading local folder repository: missing manifest file" << std::endl;
        _good = false;
        return;
    }

    ConfigFile repo_manifest = Config::readConfigFile(repo_manifest_path.generic_string());
    if(repo_manifest.values.find("NAME") == repo_manifest.values.end()) {
           name = repo_manifest.values["NAME"];
    }
}

ConfigFile LocalFolderRepository::getManifestFile(const std::string& package_name) {
    // We now try to open the manifest for this package
    auto manifest_package_folder_path = fs::path(path_str) / "manifests" / package_name;
    auto bvp_files_package_folder_path = fs::path(path_str) / "packages" / package_name;

    // We now ensure that these folders exist
    fs::create_directories(manifest_package_folder_path);
    fs::create_directories(bvp_files_package_folder_path);

    auto manifest_file_path = manifest_package_folder_path / "manifest";

    return Config::readConfigFile(manifest_file_path);
}
