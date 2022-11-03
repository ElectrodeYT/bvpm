# Basic Versioned Package Manager
BVPM is a basic package manager. It was written basically to make development of basic-linux easier.

# File format
The .bvp file format is just a .tar file (no compression, although if the libarchive installion supports compressed TAR files there is no reason we can't either)

The file has two/three extra files in it:

manifest: The package manifest file. Has three entries: name, version and dependencies.

owned-files: The files this package claims. If the package is uninstalled, these will be deleted.

afterinstall.sh: A script thats run at the end of the package installation. Optional.

sums: A list of file hashes. Optional; will give a warning when not present.

A folder called root must be present. The files in there will be copied to the root folder.

# Repository
BVPM currently has basic repository support. It consists of a single folder, with a repo.manifest file in it.
Packages can be added/removed from it with the bvpm-repo utility, which is in the same executable as bvpm, which is simply symlinked.
