#include "SgpParam.h"
#include "SgpApi.h"
#include <iostream>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include <opencv2/opencv.hpp>

// TODO: export LD_LIBRARY_PATH=/home/yuan/thermal_camera/IPT430M/lib:$LD_LIBRARY_PATH

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
const int SIZE = WIDTH * HEIGHT; // =196608
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
    

    /* ---------------------------------- 分配空間 ---------------------------------- */
    if (thermal_data.TempMatrix == nullptr)
    {
        thermal_data.TempMatrix = (float *)calloc(SIZE, sizeof(float));
    }

    if (thermal_data.Thermal_RGB_Image == nullptr)
    {
        thermal_data.Thermal_RGB_Image = (unsigned char *)malloc(3 * WIDTH * HEIGHT);
    }

    if (thermal_data.Thermal_Y16_Image == nullptr)
    {
        thermal_data.Thermal_Y16_Image = (short *)malloc(WIDTH * HEIGHT * sizeof(short));
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

        ret = SGP_GetTempMatrixEx(handle, thermal_data.TempMatrix, thermal_data.Thermal_Y16_Image, WIDTH, HEIGHT);


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

        std::cout << thermal_data.TempMatrix[SIZE-1] << std::endl;



        // if (thermal_data.TempMatrix != NULL)
        // {
        //     ret = SGP_GetImageTemps(handle, thermal_data.TempMatrix, SIZE * 4, 0);
        //     if (ret == SGP_OK)
        //     {
        //         std::cout << thermal_data.TempMatrix[SIZE - 1] << std::endl;
        //     }
        //     else
        //     {
        //         // 失败，TODO......
        //     }
        // }
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
        // printf("y16");
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