#include "common.h"
#include "nuklear_internal.h"

// 圆8等分
#define  COPIES8   (NK_PI / 4) 
#define CALC_PERCENT(D1,D2)  ((D2)==0? 1:(D1)/(D2 * 1.0f )) 
// 初始化资源
int set_watch_style(frm_context_t * frm) {
    struct nk_context* ctx = frm->ctx;
    struct nk_watch_style* style = &frm->style.watch;
    struct nk_display* displayinfo = &frm->display; 

    // 配色
    style->color.bg_color = nk_rgba(0, 0, 0, 255);
    style->color.front_color = nk_rgba(0x40, 0x40, 0x40, 255);
    style->color.panel_color = nk_rgba(0x1c, 0x1c, 0x1c, 255);
    style->color.font = nk_rgba(0xad, 0xad, 0xad, 255);
    style->color.font_time = nk_white;
    style->color.ring3 = nk_rgba(0x48, 0x48, 0x48, 255);
    style->color.fatburning = nk_rgba(0xF0, 0x98, 0x3B, 255);
    style->color.sitdown = nk_rgba(0x1B,0x70,0xFA, 255);
    style->color.footstep = nk_rgba(0xE5,0x4D,0x38, 255);

    // 字体
    style->font.generic = nk_font_load(ctx, SYS_FONT_MSYH, 19, displayinfo->scale.y, nk_font_chinese_glyph_ranges());
    style->font.larger = nk_font_load(ctx, SYS_FONT_MSYH, 144, displayinfo->scale.y, NULL);

    // 图标
    style->icon.heart = icon_load(IMG_FULL_PATCH("/img/heart.png"));
    style->icon.personheart = icon_load(IMG_FULL_PATCH("/img/personheart.png"));
    style->icon.moon = icon_load(IMG_FULL_PATCH("/img/moon.png"));
    style->icon.cloud = icon_load(IMG_FULL_PATCH("/img/cloud.png"));
    style->icon.celsius = icon_load(IMG_FULL_PATCH("/img/celsius.png")); 
    style->icon.fatburning = icon_load(IMG_FULL_PATCH("/img/fire.png"));
    style->icon.footstep = icon_load(IMG_FULL_PATCH("/img/foot.png"));
    style->icon.sitdown = icon_load(IMG_FULL_PATCH("/img/work.png"));
    style->icon.sun = icon_load(IMG_FULL_PATCH("/img/daytime.png"));
    style->icon.heartbeat = icon_load(IMG_FULL_PATCH("/img/heartbeat.png"));
       
    style->thickless.con_circle = NK_SCALE_FONT(8);
    style->thickless.outer_arc = NK_SCALE_FONT(8);

    style->panel_radius = NK_SCALE_W(28);

    style->padding.ring =  NK_SCALE_FONT(18);
    style->padding.panel = style->panel_radius/3;
    

    // 一定要加
    nk_style_set_font(ctx, style->font.generic);

    return 0;
}

// 将文字居中显示
static int render_text_center(struct nk_command_buffer* p, struct nk_rect rect,  const char* txt, struct nk_user_font* font, struct nk_color bg, struct nk_color fg,int middle) {
    float fw = 0, fh = 0,len=0;
    struct nk_rect rt;
    if (!font || !txt) return -1;
    len = nk_strlen(txt);
    rt.h = fh = font->height;
    // 计算宽度
    rt.w = fw = font->width(font->userdata, fh, txt,  len ); 

    
    // 重算写入区域
    rt.x = 0 == middle? rect.x:(rect.x + (rect.w - fw) / 2); 
    rt.y = rect.y + (rect.h - fh) / 2;

#ifdef  DEBUG_REFERENCE_LINE
    nk_stroke_rect(p, rt, 0, 1, nk_green);
#endif
    // 绘入命令
    nk_draw_text(p, rt , txt, len , font,  bg ,fg); 
    return 0;
}
 
// 根据弧度获取圆上的点坐标
static int get_arc_coord(float cx, float cy, float r, float arc, float* x, float* y) {
    static float rarc = 2 * NK_PI;
    float carc = arc;
    int rstep = 0;
    if (NK_ABS(arc) > rarc) {
        rstep = nk_ifloorf(arc / rarc);
        carc -= rstep * rarc;;
    }

    if (carc < 0) carc += rarc;

    *x = cx + NK_COS(carc) * r;
    *y = cy + NK_SIN(carc) * r;

    return 0;
}

// 基于圆弧的进度条
static float  render_arc_processbar(struct nk_command_buffer* p,
    float r,
    float cx, float cy,
    float barc, float earc,
    float thickless,
    float process,
    struct nk_color bgcolor,
    struct nk_color color) {

    float in_line_r = thickless / 2.0f ; // 线半径
    float in_arc_r = r - in_line_r;     // 线内径
    float c_arc = 0;

    struct nk_rect bp, ep, cp;

    // 注，最大中考虑一个圆
    if (NK_ABS(earc - barc) > 2 * NK_PI) return in_arc_r - in_line_r;

    // 起点
    get_arc_coord(cx, cy, in_arc_r, barc, &bp.x, &bp.y);
    bp.x -= in_line_r ;
    bp.y -= in_line_r ;
    bp.w = bp.h = in_line_r * 2 ;

    // 终点
    get_arc_coord(cx, cy, in_arc_r, earc, &ep.x, &ep.y);
    ep.x -= in_line_r;
    ep.y -= in_line_r;
    ep.w = ep.h = in_line_r * 2;

    nk_fill_arc(p, cx, cy, r, barc, earc, bgcolor);
    nk_fill_circle(p, bp, bgcolor);
    nk_fill_circle(p, ep, bgcolor);

    // 有比例时
    if (process > 0) {
        c_arc = barc + (earc - barc) * process;
        get_arc_coord(cx, cy, in_arc_r, c_arc, &cp.x, &cp.y);
        cp.x -= in_line_r;
        cp.y -= in_line_r;
        cp.w = cp.h = in_line_r * 2;

       nk_fill_arc(p, cx, cy, r, barc, c_arc, color);
       nk_fill_circle(p, bp, color);
       nk_fill_circle(p, cp, color);
    }
    return    in_arc_r - in_line_r;
}

// 背景
static int render_background(frm_context_t* frm, struct nk_command_buffer* p, struct nk_rect rect) {
    struct nk_watch_style* style = &frm->style.watch;
    
#ifdef  DEBUG_REFERENCE_LINE
    nk_stroke_circle(p, rect, 1, nk_red);
#endif

    nk_fill_circle(p, rect, style->color.bg_color);

    return 0;
}

// 外层圆环
static int render_outer_ring(frm_context_t* frm, struct nk_command_buffer* p, struct nk_rect rect) {
    int cols = 3;
    float fsize = 0, cx = 0, cy = 0, radius = 0, wh = 0,persent=0,padding=0;
    struct nk_rect bounds;
    struct nk_watch_style* style = &frm->style.watch;
    struct nk_watch_data* info = &frm->data;
    char str[128] = { 0 };
    size_t s_max = 128 - 1;
 

    fsize = style->font.generic->height *1.5;

    wh = rect.w < rect.h ? rect.w : rect.h;
    radius = wh / 2;
    cx = rect.x + radius;
    cy = rect.y + radius;

#ifdef  DEBUG_REFERENCE_LINE
    nk_stroke_circle(p, rect, 1, nk_red);
#endif

    persent =CALC_PERCENT( info->powerlevel,100.0f );
    render_arc_processbar(p, radius, cx, cy, 5.5 * COPIES8, 6.5 * COPIES8, style->thickless.outer_arc, persent, style->color.front_color, nk_rgba(0x2D, 0xA8, 0x28, 255));

  
    persent = CALC_PERCENT( info->heartrate , info->heartlimit);
    render_arc_processbar(p, radius, cx, cy, 7.5 * COPIES8, 8.5 * COPIES8, style->thickless.outer_arc, persent, style->color.front_color, nk_rgba(0x1A,0x9B,0xAD, 255));
     
    // AM / PM
    if (info->time.hour >= 6 && info->time.hour < 18) {
        persent = CALC_PERCENT( (info->time.hour - 6) *60 + info->time.minute ,12 * 60 );
    }
    else {
        if(info->time.hour <6 )         persent = CALC_PERCENT( 6*60 + info->time.hour *60 +info->time.minute  , 12 * 60);
        else   persent = CALC_PERCENT((info->time.hour - 6) * 60 + info->time.minute, 12 * 60);
    }
    
    render_arc_processbar(p, radius, cx, cy, 1.5 * COPIES8, 2.5 * COPIES8, style->thickless.outer_arc, persent, style->color.front_color, nk_rgba(0x7A, 0x2C, 0xFF, 255));
    
    persent = CALC_PERCENT(info->temperature, info->templimit);
    render_arc_processbar(p, radius, cx, cy, 3.5 * COPIES8, 4.5 * COPIES8, style->thickless.outer_arc, persent, style->color.front_color, nk_rgba(0xAC,0x55,0x18,255));
       

    // 遮线   
    bounds.w = bounds.h = (radius - style->thickless.outer_arc) *2;
    bounds.x = cx - bounds.w/2;
    bounds.y = cy - bounds.h/2;
    nk_fill_circle(p, bounds, style->color.bg_color);
         
    // 标签项

    // 左
    bounds.w = fsize;
    bounds.h = bounds.w * cols;
    bounds.x = rect.x + style->padding.ring;
    bounds.y = rect.y + (rect.h - bounds.h) / 2;
#ifdef  DEBUG_REFERENCE_LINE
    nk_stroke_rect(p, bounds, 0, 1, nk_red);
#endif
    padding = fsize * 0.2;
    
    nk_draw_image(p, nk_rect(bounds.x + padding , bounds.y +padding , fsize - 2*padding , fsize -2*padding ), &style->icon.cloud, nk_rgb_factor(nk_white, 1.0));
    snprintf(str, s_max, "%d", info->temperature);
    render_text_center(p, nk_rect(bounds.x, bounds.y + fsize, fsize, fsize), str, style->font.generic, style->color.bg_color, style->color.font, 1);
    nk_draw_image(p, nk_rect(bounds.x +padding , bounds.y + 2 * fsize + padding, fsize -2*padding, fsize -2*padding ), &style->icon.celsius, nk_rgb_factor(nk_white, 1.0));


    // 右 
    bounds.x = rect.x + rect.w - style->padding.ring - bounds.w;
    bounds.y = rect.y + (rect.h - bounds.h) / 2;
#ifdef  DEBUG_REFERENCE_LINE
    nk_stroke_rect(p, bounds, 0, 1, nk_red);
#endif
    nk_draw_image(p, nk_rect(bounds.x + padding, bounds.y + padding, fsize - 2 * padding, fsize - 2 * padding), &style->icon.personheart, nk_rgb_factor(nk_white, 1.0));
    snprintf(str, s_max, "%d", info->heartrate);
    render_text_center(p, nk_rect(bounds.x, bounds.y + fsize, fsize, fsize), str, style->font.generic, style->color.bg_color, style->color.font, 1);
    nk_draw_image(p, nk_rect(bounds.x + padding , bounds.y + 2 * fsize + padding , fsize -2*padding, fsize -2*padding ), &style->icon.heartbeat, nk_rgb_factor(nk_white, 1.0));


    // 上
    bounds.h = fsize;
    bounds.w = bounds.h * cols;
    bounds.x = rect.x + (rect.w - bounds.w) / 2;
    bounds.y = rect.y + style->padding.ring;
#ifdef  DEBUG_REFERENCE_LINE
    nk_stroke_rect(p, bounds, 0, 1, nk_red);    
#endif
    snprintf(str, s_max, "%d%%", info->powerlevel);
    render_text_center(p, bounds, str, style->font.generic, style->color.bg_color, style->color.font, 1);


    // 下
    bounds.x = rect.x + (rect.w - bounds.w) / 2;
    bounds.y = rect.y + rect.h - style->padding.ring - bounds.h;
#ifdef  DEBUG_REFERENCE_LINE
    nk_stroke_rect(p, bounds, 0, 1, nk_red); 
#endif

    // 白天
    if (info->time.hour >= 6 && info->time.hour < 18) {
        nk_draw_image(p, nk_rect(bounds.x + padding, bounds.y + padding, fsize - 2 * padding, fsize - 2 * padding), &style->icon.sun, nk_rgb_factor(nk_white, 1.0));
        snprintf(str, s_max, "%dh%dm", info->time.hour - 6,info->time.minute );
        render_text_center(p, nk_rect(bounds.x + fsize, bounds.y, 2 * fsize, fsize), str, style->font.generic, style->color.bg_color, style->color.font, 0);
    }
    else {
        nk_draw_image(p, nk_rect(bounds.x + padding, bounds.y + padding, fsize - 2 * padding, fsize - 2 * padding), &style->icon.moon, nk_rgb_factor(nk_white, 1.0));
        snprintf(str, s_max, "%dh%dm", info->time.hour<6? (6+info->time.hour): (info->time.hour - 18),info->time.minute  );
        render_text_center(p, nk_rect(bounds.x + fsize, bounds.y, 2 * fsize, fsize), str, style->font.generic, style->color.bg_color, style->color.font, 0);
    }

    return 0;
}

// 3层圆环进度
static int render_3ring(frm_context_t* frm, struct nk_command_buffer* p, struct nk_rect rect) {

    float cx = 0, cy = 0, r = 0, rw = 0, thickness = 0,
        barc = NK_PI * 3 / 2, earc = barc + 2 * NK_PI - 0.1,
        persent = 0,
        padding = 0;
    struct nk_rect fillrect;

    struct nk_watch_style* style = &frm->style.watch;
    struct nk_watch_data* info = &frm->data;

    // 圆心
    cx = rect.x + rect.w / 2;
    cy = rect.y + rect.h / 2;
    // 最大半径
    r = (rect.w < rect.h ? rect.w : rect.h) / 2 - style->thickless.con_circle;

    // (包含环 + 空白):最大化 x 3
    rw = r / 3.0f;
    thickness = style->thickless.con_circle;
    // 避免挤到一起去
    if (rw - thickness <= 0) {
        padding = 0;
        thickness = rw;// 环间无间距
    }
    else {
        padding = rw - thickness;
        padding = padding > thickness * 0.6 ? thickness * 0.6 : padding;
    }
    // 步数 
    persent = CALC_PERCENT(info->footstep, info->footlimit);
    r = render_arc_processbar(p, r, cx, cy, barc, earc, thickness, persent, style->color.ring3, style->color.footstep);
   
    fillrect.x = cx - r;
    fillrect.y = cy - r;
    fillrect.w = fillrect.h = r * 2;
    nk_fill_circle(p, fillrect, style->color.panel_color);


    // 燃脂
    persent = CALC_PERCENT(info->fatburning, info->fatburninglimit);
    r -= padding;
    r = render_arc_processbar(p, r, cx, cy, barc, earc, thickness, persent, style->color.ring3, style->color.fatburning);
    fillrect.x = cx - r;
    fillrect.y = cy - r;
    fillrect.w = fillrect.h = r * 2;
    nk_fill_circle(p, fillrect, style->color.panel_color);

    // 工作时长
    persent = CALC_PERCENT(info->work, info->worklimit);
    r -= padding;
    r = render_arc_processbar(p, r, cx, cy, barc, earc, thickness, persent, style->color.ring3, style->color.sitdown);
    fillrect.x = cx - r;
    fillrect.y = cy - r;
    fillrect.w = fillrect.h = r * 2;
    nk_fill_circle(p, fillrect, style->color.panel_color);

#ifdef  DEBUG_REFERENCE_LINE
    nk_stroke_rect(p, rect, 1, 1, nk_blue);
    nk_fill_circle(p, nk_rect(cx, cy, 4, 4), nk_red);
#endif 

    return 0;
}

// 面板信息
static int render_panel(frm_context_t* frm, struct nk_command_buffer* p, struct nk_rect rect) {
    int rows = 5, cols = 3;
    float col_width = 0, col_height = 0,h=0,heartpadding=8,padding=0;
    struct nk_rect bounds = rect,rt;
    struct nk_watch_style* style = &frm->style;
    struct nk_watch_data* info = &frm->data;
    char str[128] = {0};
    size_t s_max = 128 - 1;
  
#ifdef  DEBUG_REFERENCE_LINE
    nk_stroke_rect(p, rect, style->panel_radius, 1, nk_red);
#endif

    nk_fill_rect(p,rect, style->panel_radius,style->color.panel_color );
    // 可用区域    
    bounds.x += style->padding.ring;
    bounds.y += style->padding.ring;
    bounds.w -= 2 * style->padding.ring;
    bounds.h -= 2 * style->padding.ring;
#ifdef  DEBUG_REFERENCE_LINE
    nk_stroke_rect(p, bounds, 0, 1, nk_red);
#endif
    // 行列
    col_width = bounds.w / cols;
    col_height = bounds.h / rows;
    
#ifdef  DEBUG_REFERENCE_LINE
    // row
    nk_stroke_line(p, bounds.x, bounds.y + col_height, bounds.x + bounds.w, bounds.y + col_height, 1, nk_red);
    nk_stroke_line(p, bounds.x, bounds.y + 2 * col_height, bounds.x + bounds.w, bounds.y + 2 * col_height, 1, nk_red);
    nk_stroke_line(p, bounds.x, bounds.y + 3 * col_height, bounds.x + bounds.w, bounds.y + 3 * col_height, 1, nk_red);
    nk_stroke_line(p, bounds.x, bounds.y + 4 * col_height, bounds.x + bounds.w, bounds.y + 4 * col_height, 1, nk_red);
    // col
    nk_stroke_line(p, bounds.x + col_width, bounds.y, bounds.x + col_width, bounds.y + bounds.h, 1, nk_red);
    nk_stroke_line(p, bounds.x + 2 * col_width, bounds.y, bounds.x + 2 * col_width, bounds.y + bounds.h, 1, nk_red);
#endif
    // 星期，日期，心率
    {
        render_text_center(p, nk_rect(bounds.x, bounds.y, col_width, col_height), get_week_name( info->time.wday ), style->font.generic, style->color.front_color, style->color.font, 1);
        snprintf(str, s_max, "%d.%d", info->time.month +1, info->time.day);
        render_text_center(p, nk_rect(bounds.x + col_width, bounds.y, col_width, col_height),str, style->font.generic, style->color.front_color, style->color.font, 1);

        rt.w = col_width;
        rt.h = style->font.generic->height * 1.5;
        h = rt.h = rt.h < col_height ? rt.h : col_height;
        rt.x = bounds.x + 2 * col_width;
        rt.y = bounds.y + (col_height - rt.h) / 2;

        nk_fill_rect(p, rt, rt.h / 2, nk_rgb(0x4A, 0x20, 0x25));
        rt.w = rt.h;
        nk_stroke_circle(p, rt, 4, nk_rgb(0xdc, 0x26, 0x26));
        rt.x += heartpadding;
        rt.y += heartpadding;
        rt.w -= heartpadding * 2;
        rt.h -= heartpadding * 2;
        nk_draw_image(p, rt, &style->icon.heart, nk_rgb_factor(nk_white, 1.0));
        h += 8;
        snprintf(str, s_max, "%d", info->heartrate);
        render_text_center(p, nk_rect(bounds.x + 2 * col_width + h, bounds.y, col_width - h, col_height), str, style->font.generic, style->color.front_color, style->color.font, 0);
    }

    // 第二行
    {
        snprintf(str, s_max, "%02d", info->time.hour);
        render_text_center(p, nk_rect(bounds.x  , bounds.y + col_height , 2*col_width, 2*col_height), str, style->font.larger, style->color.front_color, style->color.font_time, 1);
        
        render_3ring(frm, p, nk_rect( bounds.x + 2*col_width, bounds.y + col_height, col_width,2*col_height ));
    }

    {
        snprintf(str, s_max, "%02d", info->time.minute);
        render_text_center(p, nk_rect(bounds.x, bounds.y + 3 * col_height, 2 * col_width, 2 * col_height), str, style->font.larger, style->color.front_color, style->color.font_time, 1);

        // 右边区域再分三行
        rt.x = bounds.x + 2 * col_width;
        rt.y = bounds.y + 3 * col_height;
        rt.w = col_width;
        rt.h = col_height * 2;
      
        col_height = rt.h / 3;
        padding = col_height / 4; // h /2 /2

#ifdef  DEBUG_REFERENCE_LINE
        nk_stroke_rect(p, rt, 0, 1, nk_green);
        nk_stroke_rect(p, nk_rect(rt.x, rt.y, col_height, col_height), 0, 1, nk_green);
        nk_stroke_rect(p, nk_rect(rt.x, rt.y + col_height, col_height, col_height), 0, 1, nk_green);
        nk_stroke_rect(p, nk_rect(rt.x, rt.y + 2 * col_height, col_height, col_height), 0, 1, nk_green);
#endif

        // 第一行
        nk_draw_image(p, nk_rect(rt.x + padding, rt.y + padding, col_height - 2 * padding, col_height - 2 * padding), &style->icon.sitdown, nk_rgb_factor(nk_white, 1.0));
        snprintf(str, s_max, "%u", info->work);
        render_text_center(p, nk_rect(rt.x + col_height, rt.y , rt.w - col_height , col_height), str , style->font.generic, style->color.front_color, style->color.font_time, 0);


        nk_draw_image(p, nk_rect(rt.x  + padding , rt.y + col_height + padding, col_height - 2 * padding, col_height - 2 * padding), &style->icon.fatburning, nk_rgb_factor(nk_white, 1.0));
        snprintf(str, s_max, "%u", info->fatburning);
        render_text_center(p, nk_rect(rt.x + col_height, rt.y + col_height , rt.w - col_height, col_height), str, style->font.generic, style->color.front_color, style->color.font_time, 0);


        nk_draw_image(p, nk_rect(rt.x  + padding, rt.y + 2 * col_height + padding, col_height - 2 * padding, col_height - 2 * padding), &style->icon.footstep, nk_rgb_factor(nk_white, 1.0));
        snprintf(str, s_max, "%u", info->footstep);
        render_text_center(p, nk_rect(rt.x + col_height, rt.y + 2* col_height, rt.w - col_height, col_height), str, style->font.generic, style->color.front_color, style->color.font_time, 0);

    }


    return 0;
}
 

int render_watch(frm_context_t *frm) {

    struct nk_canvas canvas;
    struct nk_rect rect;
    nk_flags flags=0;
    float x = 0, y = 0,cx=0,cy=0, w = 0, h = 0, radius = 0,wh=0;
    int watch_size = 0;


    struct nk_display* displayinfo = &frm->display;
    struct nk_watch_style* style = &frm->style;

    watch_size = displayinfo->win.width < displayinfo->win.height ? displayinfo->win.width : displayinfo->win.height;
    if (watch_size <= 0) return 0;

    if (canvas_begin( frm->ctx, &canvas, "Watch", flags, 0, 0, watch_size, watch_size, nk_rgba(250, 250, 250, 0)))
    {
        x = canvas.painter->clip.x;
        y = canvas.painter->clip.y;
        w = canvas.painter->clip.w;
        h = canvas.painter->clip.h;
        radius = (w < h ? w : h) / 2;
        cx = x + radius;
        cy = y + radius;
        wh = 2 * radius;

        // 表盘背景
       rect = nk_rect(x, y, wh, wh);
       render_background(frm,canvas.painter,rect );

       // 表盘
       rect.x += style->padding.ring;
       rect.y += style->padding.ring;
       rect.w -= 2 * style->padding.ring;
       rect.h -= 2 * style->padding.ring; 
       render_outer_ring(frm, canvas.painter, rect);
      
       // 面板
       wh = (rect.w/2) * NK_INV_SQRT(2.0); // 圆内接正方形边长
       rect.x = cx - wh;
       rect.y = cy - wh;
       rect.w = 2 * wh;
       rect.h = 2 * wh; 
       render_panel(frm, canvas.painter, rect); 
 
#ifdef  DEBUG_REFERENCE_LINE
        // // 中心标记
        nk_fill_circle(canvas.painter, nk_rect(cx - 10, cy - 10, 20, 20), nk_rgb(255, 0, 0));
#endif //  DEBUG_REFERENCE_LINE

      
    }
    canvas_end(frm->ctx, &canvas);
	return 0;
}