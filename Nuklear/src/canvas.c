/* nuklear - v1.05 - public domain */
#include "common.h"



nk_bool canvas_begin(struct nk_context* ctx, struct nk_canvas* canvas,const char* name  ,nk_flags flags,
    int x, int y, int width, int height, struct nk_color background_color)
{
    /* save style properties which will be overwritten */
    canvas->panel_padding = ctx->style.window.padding;
    canvas->item_spacing = ctx->style.window.spacing;
    canvas->window_background = ctx->style.window.fixed_background;

    /* use the complete window space and set background */
    ctx->style.window.spacing = nk_vec2(0, 0);
    ctx->style.window.padding = nk_vec2(0, 0);
    ctx->style.window.fixed_background = nk_style_item_color(background_color);

    /* create/update window and set position + size */
    if (!nk_begin(ctx, name, nk_rect(x, y, width, height), NK_WINDOW_NO_SCROLLBAR | flags))
        return nk_false;

    /* allocate the complete window space for drawing */
    {
        struct nk_rect total_space;
        total_space = nk_window_get_content_region(ctx);
        nk_layout_row_dynamic(ctx, total_space.h, 1);
        nk_widget(&total_space, ctx);
        canvas->painter = nk_window_get_canvas(ctx);
    }

    return nk_true;
}

void canvas_end(struct nk_context* ctx, struct nk_canvas* canvas)
{
    nk_end(ctx);
    ctx->style.window.spacing = canvas->panel_padding;
    ctx->style.window.padding = canvas->item_spacing;
    ctx->style.window.fixed_background = canvas->window_background;
}