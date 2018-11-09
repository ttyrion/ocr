#pragma once

class GDIUtil
{
public:
    GDIUtil();
    ~GDIUtil();

    static int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
    static bool GDIUtil::ClearRGB(const std::wstring& file);
    static bool ConvertBMP2JPG(const std::wstring& dest_jpg_file, const std::wstring& src_bmp_file);
};

