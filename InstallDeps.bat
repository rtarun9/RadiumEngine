# Default version is set to 64bit by setting environment variable 
# VCPKG_DEFAULT_TRIPLET = x64-windows
vcpkg.exe install stb

vcpkg.exe install imgui[dx11-binding]
vcpkg.exe install imgui[win32-binding]
vcpkg.exe install imgui[docking-experimental]

vcpkg.exe install assimp