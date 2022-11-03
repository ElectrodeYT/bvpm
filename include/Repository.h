//
// Created by alexander on 11/1/22.
//

#ifndef BVPM_REPOSITORY_H
#define BVPM_REPOSITORY_H

#include <string>
#include <vector>
#include <utility>

class Repository {
public:
    explicit Repository(std::string _name) : name(std::move(_name)) { }
    virtual ~Repository() = default;

    /// Check if this repository is capable of providing a specified package.
    /// \param package_name The package name.
    /// \return If true, this package can be queried or installed from this source. Call preparePackages() before doing anything other than querying more metadata.
    virtual bool checkIfPackageIsAvailable(const std::string& package_name) { return false; }

    /// Get the version of a package in this repository.
    /// This function can be called before preparePackages()
    /// \param package_name The package name.
    /// \return The version as a string, or "0.0.0" if unavailable.
    virtual std::string getPackageVersion(const std::string& package_name) { return "0.0.0"; }

    virtual size_t getPackageTotalSize(const std::string& package_name) { return 0; }

    virtual size_t getPackageFileSize(const std::string& package_name) { return 0; }

    virtual std::vector<std::string> getPackageDependencies(const std::string& package_name) { return {}; }

    /// Get the path to a bvp file for a specific package. Call preparePackages() before using this function.
    /// \param package_name The package name.
    /// \return Absolute path to the package, not relative to any repository, or install root.
    virtual std::string getPackageBVPFilePath(const std::string& package_name) { return ""; }

    /// Call this function before calling any other function not relating to querying package metadata
    /// \param packages List of packages to e.g. download
    /// \return If true, the package got prepared successfully. If false, then there was a critical error.
    virtual bool preparePackage(const std::string& package) { return false; }

    /// Check if the repository is not e.g. corrupted, unreachable, or unusable
    /// \return If true, this repository is not bad. This does not mean that packages themselves are not corrupt
    virtual bool good() { return false; }

    /// Check if a package in a repository is not corrupt. Call preparePackages() first.
    /// \param package_name The package name.
    /// \return If true, the package and the repository are both not bad.
    virtual bool packageGood(const std::string& package_name) { return false; }

    /// Add a package BVP file to the repository.
    /// \param package_file Path to the absolute package file.
    /// \return If true, then the package can be queried as normal; preparePackages() still must be called. Network repositories must always return false.
    virtual bool addPackageFileToRepository(const std::string& package_file) { return false; }

    /// Remove a package from the repository.
    /// \param package_name The package name.
    /// \return If true, the package has been removed from the repository. Network-backed repositories must always return false.
    virtual bool removePackageFromRepository(const std::string& package_name) { return false; }

    std::string getName() { return name; }

protected:
    std::string name;
};


#endif //BVPM_REPOSITORY_H
