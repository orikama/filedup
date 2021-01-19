# Filedup

## Dependencies
- `Boost 1.74`, интересно то, что когда я начинал версия буста в vcpkg была 1.74, сейчас она 1.73, по идее все равно должно работать

## Как я билдил
- vcpkg

  ```
  vcpkg install boost:x64-windows-static
  cmake .. -GNinja  -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE="PATH_TO_VCPKG\\scripts\\buildsystems\\vcpkg.cmake" \
    -DVCPKG_TARGET_TRIPLET="x64-windows-static"
  ninja
  ```
- or conan

  ```
  conan install ..
  cmake .. -GNinja -DCMAKE_BUILD_TYPE=Release
  ninja
  ```

## Usage

```
Usage: filedup_console DIR1 DIR2 [options]

Options:
  --help                display this help message
  --dirs arg            paths to the two directories to compare
  -r [ --recursively ]  search for duplicate files in subfolders recursively
```
