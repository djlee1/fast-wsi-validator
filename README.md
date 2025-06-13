# Fast WSI Validator

A utility for very fast detection of corrupted JPEG tiles within WSI (Whole Slide Image) files in TIFF and SVS formats.

The primary purpose is to detect corrupted tiles that appear as white in `slideio` and similar tools before image loading. Written in C++ (`libtiff`, `libjpeg-turbo`) for maximum performance, it immediately stops inspection upon finding the first error.

## Key Features

-   Support for TIFF and SVS files
-   Maximized speed by checking only Level 0 (highest resolution) JPEG tiles (option to check all levels available)
-   Quick validation of error existence only (`(isValid, errorMessage)`)

## ⚠️ Prerequisites

This package includes C++ extensions, so the following libraries and development header files **must** be installed in the environment:

-   C++ compiler (g++, clang, etc.)
-   `libjpeg-turbo` (development headers)
-   `libtiff` (development headers)

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install build-essential libjpeg-turbo8-dev libtiff5-dev
pip install fast-wsi-validator
```


# Usage
```python
import fast_wsi_validator
# File path to check
file_path = "path/to/your/slide.svs"

# Check quickly with default settings (level_zero_only=True)
is_valid, error_message = fast_wsi_validator.check_file(file_path)

if is_valid:
    print(f"✅ '{file_path}' is valid.")
else:
    print(f"❌ An error was detected in '{file_path}'.")
    print(f"   Error cause: {error_message}")
```
