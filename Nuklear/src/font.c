#include "common.h"


struct nk_user_font* nk_font_load( struct nk_context* ctx,const char* fullname, float pixel_h,float scale ,const nk_rune* range) {
	struct nk_font_atlas* atlas = NULL;
	struct nk_font* font = NULL;
	struct nk_font_config cfg = nk_font_config(pixel_h); 
	if (range) cfg.range = range;

	// cfg.oversample_h = 1.0;
	// cfg.oversample_v = 1.0;

	nk_glfw3_font_stash_begin(&atlas);
	font = nk_font_atlas_add_from_file(atlas, fullname , pixel_h , &cfg);
	nk_glfw3_font_stash_end();

	if (scale > 0) font->handle.height *= scale;

	return &font->handle;
}
