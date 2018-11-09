#pragma once
#include <string>
#include "allheaders.h"
#include "baseapi.h"
#include "basedir.h"
#include "dict.h"
#include "openclwrapper.h"
#include "osdetect.h"
#include "renderer.h"
#include "strngs.h"

class TessOCR
{
public:
    TessOCR();
    ~TessOCR();

    bool Init(const std::wstring& language);
    bool RecognizeImage(const std::wstring& image_file);
    //x,y,w,h
    std::wstring GetRecognizedText(RECT image_area);
private:
    tesseract::TessBaseAPI ocr_base_;
    bool initialized_ = false;
    Pix* pixs_ = nullptr;
    bool image_set_ = false;
};

