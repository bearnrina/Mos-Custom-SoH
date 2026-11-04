# Source snapshot build notes

This export includes current source and customized libultraship/Torch trees, without Git metadata or prebuilt dependencies. It matches the source used for the tested Release executable, but the export has not been built in a clean environment.

Use Visual Studio 2022 C++ v143, a suitable Windows SDK, CMake 3.26 or newer, Python 3, and the dependency tooling described in docs/BUILDING.md. Dependency setup may need network access.

For this Git-free snapshot, add -DSOH_SKIP_GIT_METADATA=ON when configuring. Start with a new build directory. Use -DSOH_REUSE_LOCAL_DEPENDENCIES=OFF for fresh dependency setup; the default local-reuse optimization is intended for the author's installed dependencies.

Build the custom engine archive using the GenerateSohOtr target. Extract runtime game archives separately from your own supported ROM. Keep the included dependency sources rather than checking out upstream replacements.

Engine metadata remains upstream 9.2.3; this custom release is v1.1.