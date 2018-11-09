#include <stdafx.h>
#include "GDIUtil.h"
#include <Gdiplus.h>
#include<math.h>

using namespace Gdiplus;

GDIUtil::GDIUtil()
{
}


GDIUtil::~GDIUtil()
{
}

int GDIUtil::GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
    UINT  num = 0;            // number of image encoders
    UINT  size = 0;           // size of the image encoder array in bytes

    ImageCodecInfo* pImageCodecInfo = NULL;

    GetImageEncodersSize(&num, &size);
    if (size == 0)
        return -1;  // Failure

    pImageCodecInfo = (ImageCodecInfo*)malloc(size);
    if (pImageCodecInfo == NULL)
        return -1;  // Failure

    GetImageEncoders(num, size, pImageCodecInfo);

    for (UINT j = 0; j < num; ++j)
    {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
        {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;  // Success
        }
    }
    free(pImageCodecInfo);
    return -1;
}

bool GDIUtil::ClearRGB(const std::wstring& file) {
    CLSID clsid;
    int ret = -1;
    if (-1 != GetEncoderClsid(L"image/jpeg", &clsid))
    {
        GdiplusStartupInput startupInput;
        ULONG_PTR token;
        GdiplusStartup(&token, &startupInput, NULL);
        {
            Gdiplus::Bitmap gdi_bitmap(file.c_str(), NULL);
            UINT width = gdi_bitmap.GetWidth();
            UINT height = gdi_bitmap.GetHeight();

            const int block_column = 11;
            const int block_row = 6;
            const BYTE line_separeator_pixels = 4;

            const int block_width = (width - block_column * line_separeator_pixels) / block_column;
            const int block_height = (height - (block_row - 1) * line_separeator_pixels) / block_row;

            POINT center[block_row][block_column] = { 0 };
            BYTE line_deviation = 0;
            for (int i = 0; i < block_row; ++i) {
                line_deviation = i;
                for (int j = 0; j < block_column; ++j) {
                    center[i][j].x = (block_width + line_separeator_pixels) * j + block_width / 2;
                    center[i][j].y = (block_height + line_separeator_pixels) * i + block_height / 2 + line_deviation;
                }
            }

            const int half_character_width = 38 / 2;
            const int half_character_height = 44 / 2;
            const BYTE major_channel_deviation = 20;
            const BYTE major_green_channel_deviation = 80;
            const BYTE minor_channel_deviation = 110;
            Gdiplus::Color black = Gdiplus::Color::Black;
            Gdiplus::Color color;

            for (int i = 0; i < block_row; ++i) {
                for (int j = 0; j < block_column; ++j) {
                    int xcenter = center[i][j].x;
                    int ycenter = center[i][j].y;

                    int min_x = j * (block_width + line_separeator_pixels);
                    int max_x = min_x + (block_width + line_separeator_pixels);
                    int min_y = i * (block_height + line_separeator_pixels);
                    int max_y = min_y + (block_height + line_separeator_pixels);

                    for (int n = min_y; n <= max_y; ++n)
                    {
                        for (int m = min_x; m <= max_x; ++m)
                        {
                            if (m >= width || n >= height)
                            {
                                continue;
                            }

                            if ((m - xcenter < (-half_character_width) || m - xcenter > half_character_width)
                                || (n - ycenter < (-half_character_height) || n - ycenter > half_character_height)
                                )
                            {
                                gdi_bitmap.SetPixel(m, n, black);
                            }
                            else {
                                gdi_bitmap.GetPixel(m, n, &color);
                                BYTE r = color.GetR();
                                BYTE g = color.GetG();
                                BYTE b = color.GetB();
                                
                                // Red
                                if (0xFF - r <= major_channel_deviation
                                    && g - 0x00 <= minor_channel_deviation && b - 0x00 <= minor_channel_deviation)
                                {
                                    gdi_bitmap.SetPixel(m, n, black);
                                }
                                // Green
                                else if (r - 0x00 <= minor_channel_deviation
                                    && 0xFF - g <= major_green_channel_deviation && b - 0x00 <= minor_channel_deviation)
                                {
                                    gdi_bitmap.SetPixel(m, n, black);
                                }
                                // Blue
                                else if (r - 0x00 <= minor_channel_deviation
                                    && g - 0x00 <= minor_channel_deviation && 0xFF - b <= major_channel_deviation)
                                {
                                    gdi_bitmap.SetPixel(m, n, black);
                                }
                            }
                        }
                    }
                }
            }

            Status status = gdi_bitmap.Save(/*file.c_str()*/L"test.jpg", &clsid, NULL);
            if (!status) {
                return true;
            }
        }


    }

    return false;
}


bool GDIUtil::ConvertBMP2JPG(const std::wstring& dest_jpg_file, const std::wstring& src_bmp_file) {
    GdiplusStartupInput startupInput;
    ULONG_PTR token;
    GdiplusStartup(&token, &startupInput, NULL);
    {
        Image image(src_bmp_file.c_str(), false);
        CLSID clsid;
        int ret = -1;
        if (-1 != GetEncoderClsid(L"image/jpeg", &clsid))
        {
            Status status = image.Save(dest_jpg_file.c_str(), &clsid, NULL);
            if (!status) {
                return true;
            }
        }
    }

    return false;
}