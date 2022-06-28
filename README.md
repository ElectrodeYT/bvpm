# Basic Versioned Package Manager
BVPM is a basic package manager. It was written basically to make development of basic-linux easier.

BVPM is currently in a super-duper-early stage, and should not be used for anything but the most stupid systems.
It's dependency support is very dumb, and it will probably destroy itself really quickly when faced with a hard situation.

# File format
The .bvp file format is just a .tar file (can be compressed, if the installed libarchive supports it)

The file has two/three extra files in it:

manifest: The package manifest file. Has three entries: name, version and dependencies.

owned-files: The files this package claims. If the package is uninstalled, these will be deleted.

afterinstall.sh: A script thats run at the end of the package installation. Optional.

sums: A list of file hashes. Optional; will give a warning when not present.

A folder called root must be present. The files in there will be copied to the root folder.
