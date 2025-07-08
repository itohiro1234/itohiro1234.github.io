#define _USE_MATH_DEFINES
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include "led-matrix-c.h"
#include "mnb_bmp.h"

int main(int argc, char const **argv)
{
    struct RGBLedMatrixOptions options;
    struct RGBLedMatrix *matrix;
    struct LedCanvas *offscreen_canvas;
    char rgb_sequence[] = "GBR";

    memset(&options, 0, sizeof(options));
    options.rows = options.cols = 64;
    options.led_rgb_sequence = rgb_sequence;
    matrix = led_matrix_create_from_options(&options, &argc, (char ***)&argv);
    if (matrix == NULL)
        return 1;
    offscreen_canvas = led_matrix_create_offscreen_canvas(matrix);

    pthread_t recthread;
    if (pthread_create(&recthread, NULL, (void *)takeTimelapse, NULL))
    {
        return -1;
    }

    int width, height;
    led_canvas_get_size(offscreen_canvas, &width, &height);

    // みなさんが書き換えるのはこれ以降の部分
    // あらかじめ宣言されいて使用可能な変数は，以下の4つ
    //   int width, heigh; //width=64, height=64です
    //   struct RGBLedMatrix *matrix; //LEDディスプレイパネルを表す変数
    //   struct LedCanvas *offscreen_canvas; //バッファ
    // LEDディスプレイパネルを光らせるのに使用する特別な関数は以下の4つのみ
    //   void led_canvas_clear(struct LedCanvas *canvas); //LEDディスプレイパネル前面を黒にする
    //   void led_canvas_set_pixel(struct LedCanvas *canvas, int x, int y,
    //                             uint8_t r, uint8_t g, uint8_t b); //canvasのx,yにr,g,bの色を出す
    //   struct LedCanvas *led_matrix_swap_on_vsync(struct RGBLedMatrix *matrix,
    //                                              struct LedCanvas *canvas); //canvasの内容をLEDパネルに転送
    //   int usleep(useconds_t usec); //usecマイクロ秒待機する．usecは1000000（1秒）まで

    int frame;
    int cx, cy, r;
    int x, y;

    for (frame = 0; frame < 32; ++frame)
    {
        led_canvas_clear(offscreen_canvas);

        // パックマンの中心座標（横に移動）
        cx = frame * 2;
        cy = height / 2;
        r = 10;

        // 口の開き具合（角度：0～360°）
        float mouth_angle = (frame % 2 == 0) ? 30.0 : 10.0; // 偶数フレームで大きく開く

        // パックマンを描画（黄色の円＋口カット）
        for (y = -r; y <= r; y++)
        {
            for (x = -r; x <= r; x++)
            {
                float dist = sqrt(x * x + y * y);
                if (dist > r)
                    continue;

                float angle = atan2((float)y, (float)x) * 180.0 / M_PI;
                if (angle < 0)
                    angle += 360;

                // 口の部分を描かない（右向き）
                if (angle < mouth_angle || angle > (360.0 - mouth_angle))
                    continue;

                int px = cx + x;
                int py = cy + y;

                if (px >= 0 && px < width && py >= 0 && py < height)
                {
                    led_canvas_set_pixel(offscreen_canvas, px, py, 255, 255, 0); // 黄色
                }
            }
        }

        led_matrix_swap_on_vsync(matrix, offscreen_canvas);
        usleep(100000);
    }

    return 0;
}
