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
cv::Mat convertRGBToMat(const unsigned char *rgbData);
cv::Mat convertY16ToGray(const short *y16Data);



/* ---------------------------------- 設定常量 ---------------------------------- */
const int WIDTH = 512;
const int HEIGHT = 384;
const int SIZE = WIDTH * HEIGHT; // =196608
const char *server = "192.168.1.168";
const char *username = "admin";
const char *password = "admin123";
const int port = 80;



/* ---------------------------------- 全域變數 ---------------------------------- */
float *TempMatrix = nullptr;    
unsigned char *Thermal_RGB_Image = nullptr;
short *Thermal_Y16_Image = nullptr;



int main()
{
    SGP_HANDLE handle = 0;
    handle = SGP_InitDevice();

    SGP_GENERAL_INFO info;
    memset(&info, 0x00, sizeof(info));


    if (!handle)
    {
        std::cerr << "[Error] Init device error" << std::endl;
        return handle;
    }

    std::cout << "[Info] Init device success" << std::endl;


    int ret = SGP_Login(handle, server, username, password, port);
    ret = SGP_GetGeneralInfo(handle, &info);

    if (ret == SGP_OK)
    {
        int heigth = info.ir_model_h;
        int width = info.ir_model_w;

        int length = 512 * 384;
        int type = 0;
        float *output = (float *)calloc(length, sizeof(float));

        std::cout << "[Info] heigth:" << heigth << std::endl;
        std::cout << "[Info] width:" << width << std::endl;





        std::cout << "login device success" << std::endl;

        if (Thermal_RGB_Image == NULL)
        {
            Thermal_RGB_Image = (unsigned char *)malloc(3 * WIDTH * HEIGHT);
        }

        ret = SGP_OpenIrVideo(handle, GetIrRtsp, NULL);

        if (ret == SGP_OK)
        {

            std::cout << "OpenIrVideo susss" << std::endl;

            while (true)
            {
                cv::Mat out = convertRGBToMat(Thermal_RGB_Image);


                cv::imshow("Screen Capture", out);

                int key = cv::waitKey(1);
                if (key == 27)
                { // ESC 鍵的 ASCII 碼為 27
                    break;
                }
                if (key == 102 || key == 70)
                { // F 鍵的 ASCII 碼為 102
                    SGP_SetFocus(handle, SGP_FOCUS_AUTO, 0);
                }

                if (output != NULL)
                {
                    ret = SGP_GetImageTemps(handle, output, length * 4, type);
                    if (ret == SGP_OK)
                    {
                        std::cout << output[length - 1] << std::endl;
                    }
                    else
                    {
                        // 失败，TODO......
                    }
                }


            }
            free(Thermal_RGB_Image);
            Thermal_RGB_Image = NULL;
            free(output);
            output = NULL;
            free(TempMatrix);
            TempMatrix = NULL;
        }        
    }
    else
    {
        SGP_UnInitDevice(handle);
        std::cout << "login device error" << std::endl;
    }
    // 釋放
        SGP_UnInitDevice(handle);
    


    return 0;
}

static void GetIrRtsp(unsigned char *outdata, int w, int h, void *ptr)
{
    if (outdata)
    {
        // std::cout << "複製圖片" << std::endl;
        memcpy(Thermal_RGB_Image, outdata, w * h * 3);
    }
}

cv::Mat convertRGBToMat(const unsigned char *rgbData)
{
    cv::Mat frameMat(HEIGHT, WIDTH, CV_8UC3);

    // 将 RGB 数据转换为 BGR 格式
    cv::Mat rgbMat(HEIGHT, WIDTH, CV_8UC3, const_cast<unsigned char *>(rgbData));
    cv::cvtColor(rgbMat, frameMat, cv::COLOR_RGB2BGR);

    return frameMat.clone(); // 返回复制的图像以避免内存问题
}

cv::Mat convertY16ToGray(const short *y16Data)
{
    cv::Mat frameMat(HEIGHT, WIDTH, CV_16UC1, const_cast<short *>(y16Data));
    // cv::Mat grayImage(HEIGHT, WIDTH, CV_16UC1, (void*)y16Data);
    // Convert to 8-bit grayscale image
    cv::Mat gray8Bit;
    frameMat.convertTo(gray8Bit, CV_8U); // Scale to 0-255
    return gray8Bit;
}