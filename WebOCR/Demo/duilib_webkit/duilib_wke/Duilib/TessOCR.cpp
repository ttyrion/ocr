#include "TessOCR.h"

namespace {

    bool wstring2string(std::string& dest, const std::wstring& src) {
        int length = src.length();

        int count = WideCharToMultiByte(CP_UTF8, 0, src.c_str(), length, (LPSTR)dest.c_str(), 0, 0, false);
        if (count == 0) {
            return false;
        }

        dest.resize(count, 0);
        count = WideCharToMultiByte(CP_UTF8, 0, src.c_str(), length, (LPSTR)dest.c_str(), count, 0, false);
        if (count == 0) {
            return false;
        }

        return true;
    }

    bool string2wstring(std::wstring &dest, const std::string& src) {
        int length = src.length();

        int count = MultiByteToWideChar(CP_UTF8, 0, src.c_str(), length, (LPTSTR)dest.c_str(), 0);
        if (count == 0) {
            return false;
        }

        dest.resize(count, 0);
        count = MultiByteToWideChar(CP_UTF8, 0, src.c_str(), length, (LPTSTR)dest.c_str(), count);
        if (count == 0) {
            return false;
        }

        return true;
    }
}

TessOCR::TessOCR()
{

}


TessOCR::~TessOCR()
{
    if (pixs_) {
        pixDestroy(&pixs_);
        pixs_ = nullptr;

    }

    image_set_ = false;
}

bool TessOCR::Init(const std::wstring& language) {
    PERF_COUNT_START("Tesseract:main");
    tesseract::Dict::GlobalDawgCache();

    WCHAR buffer[MAX_PATH] = { 0 };
    ::GetModuleFileName(NULL, buffer, MAX_PATH);
    std::wstring path(buffer);
    path = path.substr(0, path.find_last_of(L"\\"));
    std::string tessdata_path;
    wstring2string(tessdata_path, path);

    std::string lang;
    wstring2string(lang, language);

    int failed = ocr_base_.Init(tessdata_path.c_str(), lang.c_str(), tesseract::OEM_DEFAULT, NULL, 0, NULL, NULL, false);
    if (failed) {
        ::MessageBox(NULL, L"Init Failed!", L"OCR", MB_OK);
        return false;
    }

    if (ocr_base_.GetPageSegMode() == tesseract::PSM_SINGLE_BLOCK) {
        ocr_base_.SetPageSegMode(tesseract::PSM_AUTO);
    }

    initialized_ = true;
    return true;
}

bool TessOCR::RecognizeImage(const std::wstring& image_file) {
    std::string file;
    wstring2string(file, image_file);

    if (pixs_) {
        pixDestroy(&pixs_);
        pixs_ = nullptr;
    }

    pixs_ = pixRead(file.c_str());
    if (!pixs_) {
        ::MessageBox(NULL, L"pixRead Failed!", L"OCR", MB_OK);
        return false;
    }

    ocr_base_.SetImage(pixs_);
    image_set_ = true;

    return true;
}

std::wstring TessOCR::GetRecognizedText(RECT image_area) {
    if (!initialized_ || !image_set_) {
        return L"";
    }

    if (image_area.top < 0)
    {
        image_area.top = 0;
    }

    if (image_area.left < 0)
    {
        image_area.left = 0;
    }

    if (image_area.bottom <= 0) {
        image_area.bottom = image_area.top;
    }

    if (image_area.right <= 0) {
        image_area.right = image_area.left;
    }


    ocr_base_.SetRectangle(image_area.left, image_area.top, image_area.right - image_area.left, image_area.bottom - image_area.top);
    char* ptext = ocr_base_.GetUTF8Text();
    if (!ptext)
    {
        return L"";
    }

    std::string text(ptext);
    std::wstring wtext;
    string2wstring(wtext, text);
    for (std::wstring::iterator iter = wtext.begin(); iter != wtext.end();)
    {
        if (*iter == L'\n')
        {
            wtext.erase(iter);
        }
        else {
            ++iter;
        }
    }

    return wtext;
}