#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/complex.h>

#include <cstdio>
#include <string>
#include <vector>
#include <utility> // pair 사용
#include <csetjmp>

#include <tiffio.h>
#include <jpeglib.h>

namespace py = pybind11;

// --- 이전과 동일한 libjpeg 오류 처리 및 청크 검증 함수 ---
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

// --- 최종 속도에 최적화된 검증 함수 ---
// 반환값: {유효 여부, 첫 오류 메시지}
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
                tmsize_t tile_size = TIFFTileSize(tif);
                if (tile_size > 0) {
                    std::vector<unsigned char> buf(tile_size);
                    uint32_t num_tiles = TIFFNumberOfTiles(tif);
                    for (uint32_t tile_index = 0; tile_index < num_tiles; ++tile_index) {
                        tmsize_t bytes_read = TIFFReadRawTile(tif, tile_index, buf.data(), tile_size);
                        
                        // 타일 읽기 실패 시 즉시 반환
                        if (bytes_read < 0) {
                            TIFFClose(tif);
                            return {false, "Failed to read raw tile data"};
                        }
                        
                        auto result = validate_jpeg_chunk(buf.data(), bytes_read);
                        
                        // ★★★★★ 핵심 최적화: 첫 오류 발견 시 즉시 반환 ★★★★★
                        if (!result.first) {
                            TIFFClose(tif); // 리소스 정리 후 반환
                            return {false, result.second};
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
    // 모든 검사를 통과한 경우
    return {true, ""};
}

// --- Pybind11 모듈 정의 ---
PYBIND11_MODULE(fast_wsi_validator, m) {
    m.doc() = "A highly optimized validator to check for the existence of JPEG errors in TIFF/SVS files.";
    m.def("check_file", &has_jpeg_error,
          "Checks for JPEG errors in a TIFF/SVS file and returns immediately on the first error. "
          "Returns (isValid, errorMessage).",
          py::arg("filename"),
          py::arg("level_zero_only") = true);
}