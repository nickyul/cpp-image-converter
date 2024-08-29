#include "bmp_image.h"
#include "pack_defines.h"

#include <array>
#include <fstream>
#include <string_view>

using namespace std;

namespace img_lib {

    PACKED_STRUCT_BEGIN BitmapFileHeader{
        char b_symbol_ = 'B';
        char m_symbol_ = 'M';
        uint32_t header_data_size_ = 0;
        uint32_t reserved_ = 0;
        uint32_t offset_data_ = 14 + 40;
    }
    PACKED_STRUCT_END

    PACKED_STRUCT_BEGIN BitmapInfoHeader{
        uint32_t header_size_ = 40;
        int width_ = 0;
        int height_ = 0;
        uint16_t planes_ = 1;
        uint16_t bits_for_pixel_ = 24;
        uint32_t compression_type_ = 0;
        uint32_t data_size_ = 0;
        uint32_t width_resolution_ = 11811;
        uint32_t height_resolution_ = 11811;
        int used_colors_ = 0;
        int significant_colors_ = 0x1000000;
    }
    PACKED_STRUCT_END

    // функция вычисления отступа по ширине
    static int GetBMPStride(int w) {
        return 4 * ((w * 3 + 3) / 4);
    }

    // напишите эту функцию
    bool SaveBMP(const Path& file, const Image& image) {
        ofstream out(file, ios::binary);

        const int w = image.GetWidth();
        const int h = image.GetHeight();
        const int bmp_stride = GetBMPStride(w);

        BitmapFileHeader file_header;
        file_header.header_data_size_ = file_header.offset_data_ + bmp_stride * h;
        
        BitmapInfoHeader info_header;
        info_header.width_ = w;
        info_header.height_ = h;
        info_header.data_size_ = bmp_stride * h;

        out.write(reinterpret_cast<const char*>(&file_header), 14);
        out.write(reinterpret_cast<const char*>(&info_header), 40);

        std::vector<char> buff(bmp_stride);

        for (int y = h - 1; y >= 0; --y) {
            const Color* line = image.GetLine(y);
            for (int x = 0; x < w; ++x) {
                buff[x * 3 + 0] = static_cast<char>(line[x].b);
                buff[x * 3 + 1] = static_cast<char>(line[x].g);
                buff[x * 3 + 2] = static_cast<char>(line[x].r);
            }
            out.write(buff.data(), bmp_stride);
        }
        
        return true;
    }

    // напишите эту функцию
    Image LoadBMP(const Path& file) {
        ifstream ifs(file, ios::binary);

        BitmapFileHeader file_header;
        BitmapInfoHeader info_header;

        ifs.read(reinterpret_cast<char*>(&file_header), 14);
        ifs.read(reinterpret_cast<char*>(&info_header), 40);

        // мы поддерживаем изображения только формата P6
        // с максимальным значением цвета 255
        if (file_header.b_symbol_ != 'B' || file_header.m_symbol_ != 'M' || file_header.offset_data_ != 54 || info_header.header_size_ != 40
            || info_header.planes_ != 1 || info_header.bits_for_pixel_ != 24 || info_header.compression_type_ != 0 || info_header.width_resolution_ != 11811
            || info_header.height_resolution_ != 11811 || info_header.used_colors_ != 0 || info_header.significant_colors_ != 0x1000000) {
            return {};
        }

        Image result(info_header.width_, info_header.height_, Color::Black());
        std::vector<char> buff(info_header.data_size_ / info_header.height_);

        for (int y = info_header.height_ - 1; y >= 0; --y) {
            Color* line = result.GetLine(y);
            ifs.read(buff.data(), info_header.data_size_ / info_header.height_);

            for (int x = 0; x < info_header.width_; ++x) {
                line[x].b = static_cast<byte>(buff[x * 3 + 0]);
                line[x].g = static_cast<byte>(buff[x * 3 + 1]);
                line[x].r = static_cast<byte>(buff[x * 3 + 2]);
            }
        }
        return result;
    }

}  // namespace img_lib
