// aocr.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <atlstr.h>
#include <string>
#include <iostream>
#include "allheaders.h"
#include "baseapi.h"
#include "basedir.h"
#include "dict.h"
#include "openclwrapper.h"
#include "osdetect.h"
#include "renderer.h"
#include "strngs.h"
#include "TessOCR.h"


int main()
{
    TessOCR ocr;
    ocr.Init(L"eng");
    ocr.RecognizeImage(L"image2.png");
    RECT area = {1271, 152, 1505, 194};
    std::wstring text = ocr.GetRecognizedText(area);
    ::MessageBox(NULL, text.c_str(), L"ocr", MB_OK);

    return 0;
}

