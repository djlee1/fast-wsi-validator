#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <string>
#include <vector>
#include <utility>

#include <tiffio.h>

namespace py = pybind11;

// Final validation function: Uses libtiff's built-in decoding functionality to check tile integrity.
std::pair<bool, std::string> has_jpeg_error(const std::string& filename, bool level_zero_only = true) {
    // Disable libtiff warning and error message output.
    TIFFSetWarningHandler(nullptr);
    TIFFSetErrorHandler(nullptr);

    TIFF* tif = TIFFOpen(filename.c_str(), "r");
    if (!tif) {
        return {false, "Cannot open file"};
    }

    int dir_index = 0;
    do {
        uint16_t compression;
        // Only perform checks if compression method is JPEG or OJPEG.
        if (TIFFGetField(tif, TIFFTAG_COMPRESSION, &compression) && (compression == COMPRESSION_JPEG || compression == COMPRESSION_OJPEG)) {
            if (TIFFIsTiled(tif)) {
                // Get the buffer size required for tile decompression.
                tmsize_t tile_size = TIFFTileSize(tif);
                if (tile_size <= 0) {
                    // Skip invalid tile sizes.
                    continue;
                }

                std::vector<char> buf(tile_size);
                uint32_t num_tiles = TIFFNumberOfTiles(tif);

                for (uint32_t tile_index = 0; tile_index < num_tiles; ++tile_index) {
                    // Core: Read tile using libtiff's own decoding functionality.
                    // This function automatically handles all internal rules including JPEGTABLES.
                    tmsize_t bytes_read = TIFFReadEncodedTile(tif, tile_index, buf.data(), tile_size);

                    // If bytes_read is negative, it means libtiff failed to decode the tile.
                    // This is the most definitive evidence of "tile corruption".
                    if (bytes_read < 0) {
                        TIFFClose(tif);
                        std::string error_msg = "Tile integrity error: Failed to decode tile #" + std::to_string(tile_index) + " in directory " + std::to_string(dir_index);
                        return {false, error_msg};
                    }
                }
            }
        }

        if (level_zero_only) {
            break;
        }
        dir_index++;
    } while (TIFFReadDirectory(tif));

    TIFFClose(tif);
    // If all checks pass, the file is valid.
    return {true, ""};
}


// Pybind11 module definition
PYBIND11_MODULE(fast_wsi_validator, m) {
    m.doc() = "A highly optimized validator to check for the existence of JPEG errors in TIFF/SVS files.";

    m.def("check_file", &has_jpeg_error,
          "Checks for JPEG errors in a TIFF/SVS file by attempting to decode each tile. "
          "Returns (isValid, errorMessage).",
          py::arg("filename"),
          py::arg("level_zero_only") = true);
}