#include "SgpParam.h"
#include "SgpApi.h"
#include <iostream>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include <opencv2/opencv.hpp>

// TODO: export LD_LIBRARY_PATH=/home/yuan/thermal_camera/Thermal_IPT430M/lib:$LD_LIBRARY_PATH

/* ---------------------------------- 函式宣告 ---------------------------------- */
static void GetIrRtsp(unsigned char *outdata, int w, int h, void *ptr);
static void GetY16Data(short *y16, int length, void *ptr);
cv::Mat convertY16ToGray(const short *y16Data);
cv::Mat convertRGBToMat(const unsigned char *rgbData);

/* ---------------------------------- 熱像儀輸出結構 ---------------------------------- */
struct ThermalData
{
    // 成員變數
    float *TempMatrix;
    unsigned char *Thermal_RGB_Image;
    short *Thermal_Y16_Image;

    // 構造函式
    ThermalData()
    {
        TempMatrix = nullptr;
        Thermal_RGB_Image = nullptr;
        Thermal_Y16_Image = nullptr;
    }

    // 解構函式
    ~ThermalData()
    {
        if (TempMatrix != nullptr)
        {
            delete[] TempMatrix;
            TempMatrix = nullptr;
        }
        if (Thermal_RGB_Image != nullptr)
        {
            delete[] Thermal_RGB_Image;
            Thermal_RGB_Image = nullptr;
        }
        if (Thermal_Y16_Image != nullptr)
        {
            delete[] Thermal_Y16_Image;
            Thermal_Y16_Image = nullptr;
        }
    }
};

/* ---------------------------------- 設定常量 ---------------------------------- */
const int WIDTH = 512;
const int HEIGHT = 384;
// const int SIZE = WIDTH * HEIGHT; // =196608
const char *server = "192.168.1.168";
const char *username = "admin";
const char *password = "admin123";
const int port = 80;
const int colorbar = 17;

/* ---------------------------------- 全域變數 ---------------------------------- */
ThermalData thermal_data;

int main()
{
    int ret = 0;
    SGP_HANDLE handle = 0;
    handle = SGP_InitDevice();

    SGP_GENERAL_INFO info;
    memset(&info, 0x00, sizeof(info));

    if (!handle)
    {
        std::cerr << "[Error] Init device error" << std::endl;
        SGP_UnInitDevice(handle);
        return handle;
    }
    std::cout << "[Info] Init device success" << std::endl;

    ret = SGP_Login(handle, server, username, password, port);
    if (ret != SGP_OK)
    {
        std::cerr << "[Error] Login device error" << std::endl;
        SGP_UnInitDevice(handle);
        return ret;
    }
    std::cout << "[Info] Login device success" << std::endl;
    ret = SGP_GetGeneralInfo(handle, &info);

    if (SGP_SetColorBarShow(handle, 0))
    {
        std::cerr << "[Error] Set Color Bar Show error" << std::endl;
    }
    if (SGP_SetColorBar(handle, colorbar))
    {
        std::cerr << "[Error] Set Color Bar error" << std::endl;
    }
    if (SGP_SetTempShowMode(handle, 8))
    {
        std::cerr << "[Error] Set Temp Show Mode error" << std::endl;
    }

    /* --------------------------------- 獲取熱像儀資訊 -------------------------------- */
    int ir_model_w = info.ir_model_w;   // 红外模组宽
    int ir_model_h = info.ir_model_h;   // 红外模组高
    int ir_output_w = info.ir_output_w; // 红外通道输出宽
    int ir_output_h = info.ir_output_h; // 红外通道输出高
    int ir_model_size = 0;
    int ir_output_size = 0;
    if (ir_model_w && ir_model_h)
    {
        ir_model_size = ir_model_w * ir_model_h;
    }
    if (ir_output_w && ir_output_h)
    {
        ir_output_size = ir_output_w * ir_output_h;
    }
    std::cout << "[Info] Infra Model Width: " << ir_model_w << std::endl;    // 384
    std::cout << "[Info] Infra Model Height: " << ir_model_h << std::endl;   // 288
    std::cout << "[Info] Infra Model SIZE: " << ir_model_size << std::endl;  //
    std::cout << "[Info] Infra Output Width: " << ir_output_w << std::endl;  // 512
    std::cout << "[Info] Infra Output Height: " << ir_output_h << std::endl; // 384
    std::cout << "[Info] Infra Model SIZE: " << ir_output_size << std::endl; //

    /* ---------------------------------- 定義變數 ---------------------------------- */
    int IRModelHotSpot_x, IRModelHotSpot_y, IRModelColdSpot_x, IRModelColdSpot_y; // 紅外模組座標
    int HotSpot_x, HotSpot_y, ColdSpot_x, ColdSpot_y;                             // 輸出圖像座標
    float HotSpot_temp, ColdSpot_temp;
    int maxIndex = 0;
    float maxTemperature = 0.0;
    float IRModleToRGB_x = static_cast<float>(ir_output_w) / static_cast<float>(ir_model_w);
    float IRModleToRGB_y = static_cast<float>(ir_output_h) / static_cast<float>(ir_model_h);
    // std::cout << IRModleToRGB_x << std::endl;
    // std::cout << IRModleToRGB_y << std::endl;

    /* ---------------------------------- 分配空間 ---------------------------------- */
    if (thermal_data.TempMatrix == nullptr)
    {
        thermal_data.TempMatrix = (float *)calloc(ir_output_size, sizeof(float));
    }

    if (thermal_data.Thermal_RGB_Image == nullptr)
    {
        thermal_data.Thermal_RGB_Image = (unsigned char *)malloc(3 * ir_output_w * ir_output_h);
    }

    if (thermal_data.Thermal_Y16_Image == nullptr)
    {
        thermal_data.Thermal_Y16_Image = (short *)malloc(224256 * sizeof(short));
    }

    ret = SGP_OpenIrVideo(handle, GetIrRtsp, nullptr);
    if (ret != SGP_OK)
    {
        std::cerr << "[Error] OpenIrVideo error" << std::endl;
        SGP_Logout(handle);
        SGP_UnInitDevice(handle);
        return ret;
    }
    std::cout << "[Info] OpenIrVideo susss" << std::endl;

    ret = SGP_GetY16(handle, GetY16Data, nullptr);
    if (ret != SGP_OK)
    {
        std::cerr << "[Error] GetY16 error" << std::endl;
        SGP_Logout(handle);
        SGP_UnInitDevice(handle);
        return ret;
    }
    std::cout << "[Info] GetY16 susss" << std::endl;

    while (true)
    {
        cv::Mat out = convertRGBToMat(thermal_data.Thermal_RGB_Image);
        cv::imshow("Screen Capture", out);

        // cv::Mat Y16_img = convertY16ToGray(thermal_data.Thermal_Y16_Image);
        // cv::imshow("Y16", Y16_img);

        /* -------------------------- 溫度矩陣是紅外模組的大小 384 * 288 ------------------------- */
        ret = SGP_GetTempMatrixEx(handle, thermal_data.TempMatrix, thermal_data.Thermal_Y16_Image, ir_model_w, ir_model_h);

        // std::cout << thermal_data.TempMatrix[ir_model_size - 1] << std::endl;

        maxTemperature = thermal_data.TempMatrix[0];
        for (int i = 0; i < ir_model_size; i++)
        {
            if (thermal_data.TempMatrix[i] > maxTemperature)
            {
                maxTemperature = thermal_data.TempMatrix[i];
                maxIndex = i;
            }
        }
        IRModelHotSpot_x = (maxIndex) % ir_model_w + 1;
        IRModelHotSpot_y = (maxIndex) / ir_model_w + 1;

        HotSpot_x = IRModelHotSpot_x * IRModleToRGB_x;
        HotSpot_y = IRModelHotSpot_y * IRModleToRGB_y;

        /* -------------------------------------------------------------------------- */
        /*                               輸出像素的座標, 不是索引值                       */
        /* -------------------------------------------------------------------------- */
        std::cout << "================================================" << std::endl;
        std::cout << "紅外模組熱點x座標: " << IRModelHotSpot_x << std::endl;
        std::cout << "紅外模組熱點y座標: " << IRModelHotSpot_y << std::endl;
        std::cout << "計算最熱點x座標: " << HotSpot_x << std::endl;
        std::cout << "計算最熱點y座標: " << HotSpot_y << std::endl;
        std::cout << "計算最熱點溫度: " << maxTemperature << std::endl;
        std::cout << "================================================" << std::endl;

        int key = cv::waitKey(1);
        if (key == 27)
        { // ESC 鍵的 ASCII 碼為 27
            break;
        }
        if (key == 102 || key == 70)
        { // F 鍵的 ASCII 碼為 102
            std::cout << "[Info] AUTO FOCUS" << std::endl;
            SGP_SetFocus(handle, SGP_FOCUS_AUTO, 0);
        }
    }
    // 釋放
    SGP_Logout(handle);
    SGP_UnInitDevice(handle);
    return ret;
}

static void GetIrRtsp(unsigned char *outdata, int w, int h, void *ptr)
{
    if (outdata)
    {
        // printf("RGB");
        // std::cout << "複製圖片" << std::endl;
        memcpy(thermal_data.Thermal_RGB_Image, outdata, w * h * 3);
    }
}

static void GetY16Data(short *y16, int length, void *ptr)
{
    if (y16)
    {
        // TODO:搞清楚length是啥？
        // std::cout << length << std::endl;    // 224256
        memcpy(thermal_data.Thermal_Y16_Image, y16, length * sizeof(short));
    }
}

cv::Mat convertRGBToMat(const unsigned char *rgbData)
{
    cv::Mat frameMat(HEIGHT, WIDTH, CV_8UC3);
    cv::Mat rgbMat(HEIGHT, WIDTH, CV_8UC3, const_cast<unsigned char *>(rgbData));
    cv::cvtColor(rgbMat, frameMat, cv::COLOR_RGB2BGR);
    return frameMat.clone();
}

cv::Mat convertY16ToGray(const short *y16Data)
{
    cv::Mat frameMat(HEIGHT, WIDTH, CV_16UC1, const_cast<short *>(y16Data));
    cv::Mat gray8Bit;
    frameMat.convertTo(gray8Bit, CV_8U); // Scale to 0-255
    return gray8Bit;
}