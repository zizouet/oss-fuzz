// SPDX-License-Identifier: Apache-2.0
// Fuzz harness for FreeType SVG renderer (with necessary stubs)

extern "C" {
        #include <ft2build.h>
        #include FT_FREETYPE_H        // FT_Init_FreeType, FT_Library, FT_Face
        #include FT_DRIVER_H          // FT_Load_Glyph, FT_GlyphSlot
        #include FT_RENDER_H          // FT_Render_Glyph, FT_RENDER_MODE_NORMAL, FT_Get_Renderer
        
        #include <freetype/internal/ftserv.h>           // FT_DEFINE_SERVICE
        #include <freetype/internal/ftobjs.h>           // FT_RendererRec_
        #include <freetype/internal/services/svprop.h>  // FT_Property_Set/Get
        #include <freetype/internal/svginterface.h>     // SVG_RendererHooks, SVG_RendererRec
        #include <freetype/otsvg.h>                     // public SVG_RendererHooks API
        
        // -- Stub zlib functions used by fttgzip.c
        int z_inflateInit2_(void* strm, int windowBits, const char* version, int stream_size) { 
            return 0;
        }
        int z_inflateEnd(void* strm) { 
            return 0; 
        }
        int z_inflateReset(void* strm) { 
            return 0; 
        }
        int z_inflate(void* strm, int flush) { 
            return 0; 
        }
        
        // -- Stub bzip2 functions used by ftbzip2.c
        int BZ2_bzDecompressInit(void** bzstream, int small, int verbosity) {
            return 0; 
        }
        int BZ2_bzDecompressEnd(void* bzstream) {
            return 0; 
        }
        int BZ2_bzDecompress(void* bzstream, int action, unsigned int* destLen, unsigned int* srcLen) { 
            return 0; 
        }
        
        // -- Stub libpng shim functions (OSS_FUZZ_png_*)
        void* OSS_FUZZ_png_create_read_struct(const char* ver, void* error_ptr, void* error_fn, void* warn_fn) {
            return nullptr; 
        }
        void* OSS_FUZZ_png_create_info_struct(void* png_ptr) { return nullptr; }
        void OSS_FUZZ_png_destroy_read_struct(void** png_ptr_ptr, void** info_ptr_ptr, void** end_info_ptr_ptr) {}
        void OSS_FUZZ_png_set_error_fn(void* png_ptr, void* error_ptr, void* error_fn, void* warn_fn) {}
        void OSS_FUZZ_png_set_longjmp_fn(void* png_ptr, void (*jmp_fn)(void*), void (*jmp_arg)(void*)) {}
        void OSS_FUZZ_png_error(void* png_ptr, const char* msg, ...) {}
        void OSS_FUZZ_png_set_read_fn(void* png_ptr, void* io_ptr, void (*read_data_fn)(void*, void*, unsigned long)) {}
        void OSS_FUZZ_png_read_info(void* png_ptr, void* info_ptr) {}
        void OSS_FUZZ_png_read_update_info(void* png_ptr, void* info_ptr) {}
        void OSS_FUZZ_png_get_IHDR(void* png_ptr, unsigned long* width, unsigned long* height, int* bit_depth, int* color_type, int* interlace_method, int* compression_method) {}
        unsigned int OSS_FUZZ_png_get_valid(void* png_ptr, void* info_ptr, unsigned int flag) {
            return 0;
        }
        void OSS_FUZZ_png_set_palette_to_rgb(void* png_ptr) {}
        void OSS_FUZZ_png_set_expand_gray_1_2_4_to_8(void* png_ptr, int user_bit_depth, int max_bit_depth) {}
        void OSS_FUZZ_png_set_tRNS_to_alpha(void* png_ptr) {}
        void OSS_FUZZ_png_set_strip_16(void* png_ptr) {}
        void OSS_FUZZ_png_set_packing(void* png_ptr) {}
        void OSS_FUZZ_png_set_gray_to_rgb(void* png_ptr) {}
        void OSS_FUZZ_png_set_interlace_handling(void* png_ptr) {}
        void OSS_FUZZ_png_set_filler(void* png_ptr, unsigned int filler, int flags) {}
        void OSS_FUZZ_png_read_image(void* png_ptr, void* image) {}
        void OSS_FUZZ_png_read_end(void* png_ptr, void* info_ptr) {}
        void* OSS_FUZZ_png_get_io_ptr(void* png_ptr) {
            return nullptr; 
        }
        void* OSS_FUZZ_png_get_error_ptr(void* png_ptr) {
            return nullptr; 
        }
        void OSS_FUZZ_png_set_read_user_transform_fn(void* png_ptr, void (*transform_fn)(void*, void*, unsigned int, unsigned int)) {}
        int BrotliDecoderDecompress(size_t encoded_size, const unsigned char* encoded_buffer, size_t* decoded_size, unsigned char* decoded_buffer) { 
            return 1; 
        }
    }
    
    #include <cstdint>
    #include <cstdlib>
    #include <cstring>
    
    extern "C" {
    // Dummy SVG hooks matching expected signatures
    static FT_Error dummy_init_svg(FT_Pointer *state) { return FT_Err_Ok; }
    static void dummy_free_svg(FT_Pointer *state) {}

    static FT_Error dummy_render_svg(FT_GlyphSlot slot, FT_Pointer *state) {
        if (slot->bitmap.buffer) slot->bitmap.buffer[0] ^= 0xFF;
        return FT_Err_Ok;
    }
    static FT_Error dummy_preset_slot(FT_GlyphSlot slot, FT_Bool cache, FT_Pointer *state) {
        slot->bitmap.rows = 1; slot->bitmap.width = 1; slot->bitmap.pitch = 1;
        slot->bitmap.pixel_mode = FT_PIXEL_MODE_GRAY;
        slot->bitmap.buffer = static_cast<unsigned char*>(std::malloc(1));
        if (!slot->bitmap.buffer) return FT_Err_Out_Of_Memory;
        slot->bitmap.buffer[0] = 0xAB;
        return FT_Err_Ok;
    }
    } // extern "C"
    
    extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
        if (size < 4) 
            return 0;
        // Skip BDF font parsing to avoid hangs on ASCII-based BDF files
        if (size >= 9 && memcmp(data, "STARTFONT", 9) == 0)
            return 0;
        FT_Library library;
        if (FT_Init_FreeType(&library)) return 0;
        SVG_RendererHooks hooks = { dummy_init_svg, dummy_free_svg, dummy_render_svg, dummy_preset_slot };
        FT_Property_Set(library, "ot-svg", "svg-hooks", &hooks);
    
        FT_Face face;
        if (FT_New_Memory_Face(library, data, size, 0, &face)) { FT_Done_FreeType(library); return 0; }
        FT_Set_Pixel_Sizes(face, 0, 16);
        FT_Load_Glyph(face, 0, FT_LOAD_DEFAULT);
    
        if (face->glyph && face->glyph->format == FT_GLYPH_FORMAT_SVG) {
            FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
            FT_Renderer renderer = FT_Get_Renderer(library, FT_GLYPH_FORMAT_SVG);
            if (renderer && renderer->clazz && renderer->clazz->transform_glyph) {
                FT_Matrix mtx = {0x10000,0,0,0x10000}; FT_Vector vec = {0,0};
                renderer->clazz->transform_glyph(renderer, face->glyph, &mtx, &vec);
            }
        }
    
        SVG_RendererHooks out_hooks;
        FT_Property_Get(library, "ot-svg", "svg-hooks", &out_hooks);
        FT_Done_Face(face);
        FT_Done_FreeType(library);
        return 0;
    }
