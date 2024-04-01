#include "SgpParam.h"
#include "SgpApi.h"
#include <iostream>
#include <time.h>
#include <unistd.h>
#include <string.h>


#include <opencv2/opencv.hpp>




// TODO: export LD_LIBRARY_PATH=/home/yuan/thermal_camera/IPT430M/lib:$LD_LIBRARY_PATH




/* ---------------------------------- 設定常量 ---------------------------------- */
const int WIDTH = 512;
const int HEIGHT = 384;
const int SIZE = WIDTH * HEIGHT; // =196608
const char *server = "192.168.1.168";
const char *username = "admin";
const char *password = "admin123";
const int port = 80;




float * TempMatrix = NULL;
unsigned char *img = NULL;



static void GetIrRtsp(unsigned char *outdata, int w, int h, void *ptr);
cv::Mat convertRGBToMat(const unsigned char *rgbData);



int main() {
    const char path[] = "../result/screenpic.jpg";
    SGP_HANDLE handle = 0;
    handle = SGP_InitDevice();


    if (handle) {
        std::cout << "init device success" << std::endl;
        
        int ret = SGP_Login(handle, server, username, password, port);

        SGP_GENERAL_INFO info;
        memset(&info, 0x00, sizeof(info));
        ret = SGP_GetGeneralInfo(handle,&info);

        if (ret == SGP_OK) {
            std::cout << "login device success" << std::endl;


            if (img == NULL) {
                img = (unsigned char *)malloc(3 * WIDTH * HEIGHT);
            }



            ret = SGP_OpenIrVideo(handle, GetIrRtsp, NULL);

            if (ret == SGP_OK) {
                
                std::cout << "OpenIrVideo susss" << std::endl;


                while (true) {
                    cv::Mat out = convertRGBToMat(img);

                    
                    cv::imshow("Screen Capture", out);

                    int key = cv::waitKey(1);
                    if (key == 27) { // ESC 鍵的 ASCII 碼為 27
                        break;
                    }
                }
                free(img);
                img = NULL;
            }




            // int height = info.ir_output_h;
            // int width = info.ir_output_w;
            // int length = height*width;
            // int type = 0;
            // float *output = (float *)calloc(length, sizeof(float));


            // // std::cout << "height: " << height << std::endl;
            // // std::cout << "width: " << width << std::endl;
            // // std::cout << "length: " << length << std::endl;

            // int input_length = 1024*1024*10;
            // int output_length = 0;
            // SGP_IMAGE_TYPE type_1= SGP_IR_IMAGE;
            // char *input= (char *)calloc(input_length, sizeof(char));

            // if(input!=NULL) {
            //     ret = SGP_GetScreenCaptureCache(handle, type_1, input, input_length, &output_length);
    
            //     // 定义变量用于存储图像二进制数据和大小
            //     char *image_data; // 图像二进制数据指针
            //     int image_size = output_length;   // 图像二进制数据大小

            //     // 假设已经调用了 SGP_GetScreenCaptureCache 函数获取图像数据并存储在 image_data 中

            //     // 将图像数据转换为 OpenCV Mat 格式
            //     // cv::Mat image(cv::Size(width, height), CV_8UC3, image_data);

            //     // // 在 OpenCV 中显示图像
            //     // cv::imshow("Screen Capture", image);
            //     // cv::waitKey(0);

            // }


            // while(true) {

            //     if(output!=NULL) {
            //         ret = SGP_GetImageTemps(handle, output, length*4, type);

                
            //         if (ret == SGP_OK ) {



            //             std::cout << output[SIZE-1] << std::endl;
            //         }else {

            //             std::cout << "GetImageTemps error" << std::endl;
            //         }
            //     }
                
            // }
            // free(output);

            
            // 分配空間
            // if (TempMatrix == NULL) {
            //     TempMatrix = (float *)malloc(4 * WIDTH * HEIGHT);
            // }

            // TODO: 測量溫度
            // while (true) {
            //     // std::cout << "in loop" << std::endl;
            //     SGP_GetImageTemps(handle, output, length*4, 1);
            //     usleep(1000);

            //     std::cout << output[0] << std::endl;


            // }

            

        }
        else {
            SGP_UnInitDevice(handle);
            std::cout << "login device error" << std::endl;
        }
        // 釋放
        SGP_UnInitDevice(handle);
    }
    else {
        std::cout << "init device error" << std::endl;
    }

    if (TempMatrix != NULL)
    {
        free(TempMatrix);
        TempMatrix = NULL;
    }

    return 0;
}




static void GetIrRtsp(unsigned char *outdata, int w, int h, void *ptr)
{
    if(outdata)
    {
        // std::cout << "複製圖片" << std::endl;
        memcpy(img, outdata, w * h * 3);
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
