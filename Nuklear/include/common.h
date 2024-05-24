#ifndef __NK_COMMON_H__
#define __NK_COMMON_H__
#include <stdarg.h>

 

#include <string.h>

#include "nuklear.h"
#include "nuklear_glfw_gl2.h"

#if defined(_WIN32)
#include <Windows.h>

// 字体
#define SYS_FONT_PATH(FT) "C:\\Windows\\Fonts\\"##FT
// 图片资源
#define IMG_FULL_PATCH(P) "D:\\project\\13.nuklear\\DrawUI\\DrawUI\\build\\x64\\Debug" ##P

typedef unsigned __int64 time64_t;
#else

#define SYS_FONT_PATH(FT) "/usr/share/fonts/"##FT
typedef unsigned long long time64_t; 
#endif
// 字库文件
#define SYS_FONT_MSYH  SYS_FONT_PATH("msyh.ttc")
// 绘制参考线
//#define DEBUG_REFERENCE_LINE

enum theme { THEME_BLACK, THEME_WHITE, THEME_RED, THEME_BLUE, THEME_DARK, THEME_DRACULA };


static inline const char* get_week_name(int wday ) {
   static char* wdayname[] = { "周日","周一" ,"周二" ,"周三" ,"周四" ,"周五" ,"周六" };
    if (wday < 0 || wday >6) return ""; 
    return wdayname[wday];
}

struct tm64
{
    int year;			/* years since 1900 */
    int month;			/* months since January 1 - [0,11] */
    int wday;			/* days since Sunday - [0,6] */
    int day;			/* day of the month - [1,31] */
    int hour;			/* hours since midnight - [0,23] */
    int minute;			/* minutes after the hour - [0,59] */
    int second;			/* seconds after the minute - [0,59] */
    int millisecond;	/* milliseconds after the minute - [0,999] */
};

struct nk_watch_data {
  
    time64_t timestamp;
    struct tm64 time;
    
    // 电量、温度、心率
    uint8_t powerlevel,temperature,heartrate;
    // 极值 温度、心率 
    uint8_t templimit, heartlimit;
    // 步数、步数目标、燃脂、
    uint32_t work, footstep ,fatburning;
    // 目标
    uint32_t worklimit, footlimit, fatburninglimit;

};

struct nk_watch_style {
    // 颜色定义
    struct
    {
        // 背景
        struct nk_color bg_color;
        // 前景面板
        struct nk_color front_color;
        struct nk_color panel_color;

        // 通用文字颜色
        struct nk_color font;
        // 时间文字颜色
        struct nk_color font_time;
         
        // 3个环底色,步数,燃烧
        struct nk_color ring3, footstep,fatburning,sitdown;
        
    }color;

    // 图标定义
    struct
    {
        // 心形图标
        struct nk_image heart,heartbeat;
        // 云图标
        struct nk_image cloud;
        // 月亮型图标
        struct nk_image moon,sun;
        // 人心
        struct nk_image personheart;
        // 摄氏度
        struct nk_image celsius;
        struct nk_image fatburning,sitdown, footstep;

    }icon;

    // 线条粗细定义
    struct {
        // 同心圆
        float con_circle;
        // 外层的弧
        float outer_arc;
    }thickless;

    // 字体定义
    struct {
        // 默认字号
        struct nk_user_font* generic;
        // 超大字号
        struct nk_user_font* larger;
    }font;


    struct {
        float ring;
        float panel;
    } padding;

    // 面板圆角
    float panel_radius;
};

struct nk_display {
    struct {
        float width, height;
    }screen;
    struct {
        float x, y;
    }scale;
    struct {
        int width, height;
    }win;
};

typedef struct frm_context_t {
    // nuklear context
    struct nk_context* ctx;
    // screen and winform size
    struct nk_display display;
    // style
    union
    {
        struct nk_watch_style watch;
    }style;
    // winform data
    struct nk_watch_data data; 
}frm_context_t;
 
#define NK_SCALE_W(W) (int)(displayinfo->scale.x>0? displayinfo->scale.x * (W):(W))
#define NK_SCALE_H(H) (int)(displayinfo->scale.y>0? displayinfo->scale.y * (H):(H))
#define NK_SCALE_FONT(H) NK_SCALE_H(H)

struct nk_canvas {
    struct nk_command_buffer* painter;
    struct nk_vec2 item_spacing;
    struct nk_vec2 panel_padding;
    struct nk_style_item window_background;
};

struct nk_user_font* nk_font_load(struct nk_context* ctx, const char* fullname, float pixel_h, float scale, const nk_rune* range);


nk_bool canvas_begin(struct nk_context* ctx, struct nk_canvas* canvas, const char* name , nk_flags flags,
    int x, int y, int width, int height, struct nk_color background_color);
void canvas_end(struct nk_context* ctx, struct nk_canvas* canvas);


int set_watch_style(frm_context_t *ctx);
int render_watch(frm_context_t * ctx);


struct nk_image icon_load(const char* filename);

#endif // !__NK_COMMON_H__