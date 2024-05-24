
#include <stdio.h>
#include <time.h>
#include "common.h"


// ms
time64_t time64_now(void)
{
    time64_t v;

#if defined(_WIN32)
    FILETIME ft;
    GetSystemTimeAsFileTime((FILETIME*)&ft);
    v = (((__int64)ft.dwHighDateTime << 32) | (__int64)ft.dwLowDateTime) / 10000; // to ms
    v -= 0xA9730B66800; /* 11644473600000LL */ // January 1, 1601 (UTC) -> January 1, 1970 (UTC).
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    v = tv.tv_sec;
    v *= 1000;
    v += tv.tv_usec / 1000;
#endif
    return v;
}
int time64_local(time64_t time, struct tm64* tm64)
{
    struct tm t;
    time_t seconds;

    seconds = (time_t)(time / 1000);

#if defined(_WIN32)
    localtime_s(&t, &seconds);
#else
    localtime_r(&seconds, &t);
#endif

    tm64->year = t.tm_year;
    tm64->month = t.tm_mon;
    tm64->day = t.tm_mday;
    tm64->wday = t.tm_wday;
    tm64->hour = t.tm_hour;
    tm64->minute = t.tm_min;
    tm64->second = t.tm_sec;
    tm64->millisecond = (int)(time % 1000);
    return 0;
}

// 数据采样频率
#define SAMPLE_RATE  (10)
// 间隔
#define SAMPLE_DURATION(R) (1000/((R)<1?1:(R) ) ) 

static int sensor_sampling(struct nk_watch_data* info) {
     
    static int steps[] = { 1,80,100,1,1,1 };
    time64_t  timestamp = time64_now();

    if (timestamp - info->timestamp < SAMPLE_DURATION(SAMPLE_RATE)) return 0;
    info->timestamp = timestamp; 
    time64_local(timestamp, &info->time);

    // 数据上限
    info->footlimit = 15000;
    info->fatburninglimit = 9999;
    info->heartlimit = 220;
    info->worklimit = 8;
    info->templimit = 45; 
      
    if (info->heartrate   + steps[0]  <= 0 || info->heartrate     + steps[0] > info->heartlimit) steps[0] *=-1;
    if (info->fatburning  + steps[1]  <= 0 || info->fatburning    + steps[1] > info->fatburninglimit) steps[1] *=-1;
    if (info->footstep    + steps[2]  <= 0 || info->footstep      + steps[2] > info->footlimit) steps[2] *=-1;
    if (info->temperature + steps[3]  <= 0 || info->temperature   + steps[3] > info->templimit) steps[3] *=-1;
    if (info->powerlevel  + steps[4]  <= 0 || info->powerlevel    + steps[4] > 100 ) steps[4] *=-1;
    if (info->work        + steps[5]  <= 0 || info->work          + steps[5] > info->worklimit) steps[5] *=-1;
     

    info->heartrate     += steps[0];
    info->fatburning    += steps[1];
    info->footstep      += steps[2];
    info->temperature   += steps[3];
    info->powerlevel    += steps[4];
    info->work          += steps[5];

    return 0;
}


static void error_callback(int e, const char* d) {
    printf("Error %d: %s\n", e, d);
}
static int glfw_get_display(float* w,float *h,float* xscale,float *yscale  ) {
	GLFWmonitor* primary = NULL;
    GLFWvidmode* mode = NULL;
    char* errmsg = NULL;

    primary = glfwGetPrimaryMonitor();
    if (!primary) {
        printf("monitor failed! [%d]:%s \n", glfwGetError(&errmsg), errmsg);
        return -1;
    }

    mode = glfwGetVideoMode(primary);
    glfwGetMonitorContentScale(primary, xscale, yscale);

    *w = mode->width;
    *h = mode->height;

	return 0;
} 

int main(int argc, char** argv) {
  
    GLFWwindow* win = NULL;   
    frm_context_t ctx; 
    struct nk_colorf bg = { .r = 0.10f, .g = 0.18f, .b = 0.24f, .a = 1.0f };
   
    memset( &ctx,0,sizeof(ctx) );

    // 初始化glfw
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        printf("[GFLW] failed to init!\n");
        return -1;
    }
    // 窗口适配监视器缩放
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
    // 窗口边框取消
    glfwWindowHint(GLFW_DECORATED, GL_FALSE);
    // 窗口背景透明
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

    // 获取屏幕尺寸与缩放
    if (0 != glfw_get_display(&ctx.display.screen.width, &ctx.display.screen.height, &ctx.display.scale.x, &ctx.display.scale.y)) {
        glfwTerminate();
        return -1;
    }
    printf(" Display:{%.2f,%.2f},Scale:{%.2f,%.2f} \n", ctx.display.screen.width, ctx.display.screen.height, ctx.display.scale.x, ctx.display.scale.y);

    // 创建默认窗口大小
    // ctx.display.win.width = ctx.display.screen.width * 2 / 3;
    // ctx.display.win.height = ctx.display.screen.height * 2 / 3; 

    ctx.display.win.width = 500  ;
    ctx.display.win.height = 500  ;

    win = glfwCreateWindow(ctx.display.win.width, ctx.display.win.height, "Demo", NULL, NULL);
    if (!win) {
        printf("failed to create window!\n");
        glfwTerminate();
        return -1;
    }

    printf("Window Form Size:{%d , %d} \n", ctx.display.win.width, ctx.display.win.height);
    glfwMakeContextCurrent(win); 
    // 整体窗口透明度
    // glfwSetWindowOpacity(win, 0.5f); 
    
    // 重新确认窗口区域大小 
    glfwGetWindowSize(win, &ctx.display.win.width, &ctx.display.win.height);
    printf("Window Form Size:{%d , %d} \n", ctx.display.win.width, ctx.display.win.height);

    // 初始化 glfw
    ctx.ctx = nk_glfw3_init(win, NK_GLFW3_INSTALL_CALLBACKS);
    if (!ctx.ctx) {
        printf("failed to nuklear context!\n");
        glfwTerminate();
        return -1;
    } 

    // 初始化glew 需要用它加载图片资源
    if (GLEW_OK != glewInit()) {
        glfwTerminate();
        return -1;
    }

    // 初始化样式
    set_watch_style(&ctx);

    while (!glfwWindowShouldClose(win)) {

        /* Input */
        glfwPollEvents();
        nk_glfw3_new_frame();

        /* Data */
        sensor_sampling(&ctx.data);

        /* GUI */
        render_watch(&ctx);
              

        /* Draw */
        glfwGetWindowSize(win, &ctx.display.win.width, & ctx.display.win.height);       // 窗口大小
        glViewport(0, 0, ctx.display.win.width, ctx.display.win.height);               // 设置更新区域
        glClear(GL_COLOR_BUFFER_BIT);
       // glClearColor(bg.r, bg.g, bg.b, bg.a);
        /* IMPORTANT: `nk_glfw_render` modifies some global OpenGL state
         * with blending, scissor, face culling and depth test and defaults everything
         * back into a default state. Make sure to either save and restore or
         * reset your own state after drawing rendering the UI. */
        nk_glfw3_render(NK_ANTI_ALIASING_OFF);
        glfwSwapBuffers(win);
    }

    nk_glfw3_shutdown();
    glfwTerminate();

    return 0;
}