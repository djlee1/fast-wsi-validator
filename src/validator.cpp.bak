#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/complex.h>

#include <cstdio>
#include <string>
#include <vector>
#include <utility> 
#include <csetjmp>

#include <tiffio.h>
#include <jpeglib.h>

namespace py = pybind11;


struct my_error_mgr { struct jpeg_error_mgr pub; jmp_buf setjmp_buffer; char message[JMSG_LENGTH_MAX]; };
void my_error_exit(j_common_ptr cinfo) { my_error_mgr* myerr = (my_error_mgr*) cinfo->err; (*cinfo->err->format_message)(cinfo, myerr->message); longjmp(myerr->setjmp_buffer, 1); }
std::pair<bool, std::string> validate_jpeg_chunk(const unsigned char* data, unsigned long size) {
    if (size == 0) return {false, "Empty tile data"};
    struct jpeg_decompress_struct cinfo;
    struct my_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;
    if (setjmp(jerr.setjmp_buffer)) { jpeg_destroy_decompress(&cinfo); return {false, std::string(jerr.message)}; }
    jpeg_create_decompress(&cinfo);
    jpeg_mem_src(&cinfo, data, size);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);
    while (cinfo.output_scanline < cinfo.output_height) { jpeg_skip_scanlines(&cinfo, 1); }
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    return {true, ""};
}



std::pair<bool, std::string> has_jpeg_error(const std::string& filename, bool level_zero_only = true) {
    TIFFSetWarningHandler(nullptr);
    TIFFSetErrorHandler(nullptr);

    TIFF* tif = TIFFOpen(filename.c_str(), "r");
    if (!tif) {
        return {false, "Cannot open file"};
    }

    do {
        uint16_t compression;
        if (TIFFGetField(tif, TIFFTAG_COMPRESSION, &compression) && (compression == COMPRESSION_JPEG || compression == COMPRESSION_OJPEG)) {
            if (TIFFIsTiled(tif)) {

                // 1. JPEGTABLES
                uint16_t jpeg_tables_count = 0;
                void* jpeg_tables_data = nullptr;
                bool has_jpeg_tables = TIFFGetField(tif, TIFFTAG_JPEGTABLES, &jpeg_tables_count, &jpeg_tables_data);

                tmsize_t tile_size = TIFFTileSize(tif);
                if (tile_size > 0) {
                    std::vector<unsigned char> buf(tile_size);
                    uint32_t num_tiles = TIFFNumberOfTiles(tif);

                    for (uint32_t tile_index = 0; tile_index < num_tiles; ++tile_index) {
                        tmsize_t bytes_read = TIFFReadRawTile(tif, tile_index, buf.data(), tile_size);
                        
                        if (bytes_read < 0) {
                            TIFFClose(tif);
                            return {false, "Failed to read raw tile data"};
                        }


                        if (has_jpeg_tables && jpeg_tables_count > 0) {
                            std::vector<unsigned char> combined_data;
                            
                            // SOI (Start of Image) marker
                            combined_data.push_back(0xFF);
                            combined_data.push_back(0xD8);
                            
                            // 전역 테이블 데이터 추가 (DQT, DHT 등)
                            combined_data.insert(combined_data.end(), (unsigned char*)jpeg_tables_data, ((unsigned char*)jpeg_tables_data) + jpeg_tables_count);
                            
                            // 타일 데이터에서 SOI 마커(0xFFD8)가 있다면 건너뛰고 추가
                            size_t tile_data_offset = 0;
                            if (bytes_read >= 2 && buf[0] == 0xFF && buf[1] == 0xD8) {
                                tile_data_offset = 2;
                            }
                            combined_data.insert(combined_data.end(), buf.begin() + tile_data_offset, buf.begin() + bytes_read);

                            // 합쳐진 데이터로 유효성 검사
                            auto result = validate_jpeg_chunk(combined_data.data(), combined_data.size());
                            if (!result.first) {
                                TIFFClose(tif);
                                return {false, result.second};
                            }
                        } else {
                            // 전역 테이블이 없는 경우, 기존 방식대로 검사
                            auto result = validate_jpeg_chunk(buf.data(), bytes_read);
                            if (!result.first) {
                                TIFFClose(tif);
                                return {false, result.second};
                            }
                        }
                    }
                }
            }
        }
        if (level_zero_only) {
            break;
        }
    } while (TIFFReadDirectory(tif));

    TIFFClose(tif);
    return {true, ""};
}


PYBIND11_MODULE(fast_wsi_validator, m) {
    m.doc() = "A highly optimized validator to check for the existence of JPEG errors in TIFF/SVS files.";
    m.def("check_file", &has_jpeg_error,
          "Checks for JPEG errors in a TIFF/SVS file and returns immediately on the first error. "
          "Returns (isValid, errorMessage).",
          py::arg("filename"),
          py::arg("level_zero_only") = true);
}