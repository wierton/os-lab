#include "body/game-common.h"

extern SURFACE *background;
extern SURFACE *screen;

struct RES_IDC {
    int ID;
	union {
		uint32_t life, atk;
	};
    RECT rect;
} res_idc[] =
	{
		{ID_PLAYER, {50}, {0, 0, 29, 33}},
		{ID_ENEMY, {100}, {0, 33, 62, 50}},
		{ID_DOT, {6}, {29, 13, 6, 6}},
		{ID_BLOOD, {0}, {0, 0, BLOOD_W, BLOOD_H}},
		{ID_SMALLBOMB_0, {0}, {32 * 0, 83, 32, 32}},
		{ID_SMALLBOMB_1, {0}, {32 * 1, 83, 32, 32}},
		{ID_SMALLBOMB_2, {0}, {32 * 2, 83, 32, 32}},
		{ID_SMALLBOMB_3, {0}, {32 * 3, 83, 32, 32}},
		{ID_SMALLBOMB_4, {0}, {32 * 4, 83, 32, 32}},
		{ID_SMALLBOMB_5, {0}, {32 * 5, 83, 32, 32}},
		{ID_SMALLBOMB_6, {0}, {32 * 6, 83, 32, 32}},
		{ID_SMALLBOMB_7, {0}, {32 * 7, 83, 32, 32}},
		{ID_BIGBOMB_0, {0}, {180 * 0, 115 + 135 * 0, 180, 135}},
		{ID_BIGBOMB_1, {0}, {180 * 1, 115 + 135 * 0, 180, 135}},
		{ID_BIGBOMB_2, {0}, {180 * 2, 115 + 135 * 0, 180, 135}},
		{ID_BIGBOMB_3, {0}, {180 * 3, 115 + 135 * 0, 180, 135}},
		{ID_BIGBOMB_4, {0}, {180 * 0, 115 + 135 * 1, 180, 135}},
		{ID_BIGBOMB_5, {0}, {180 * 1, 115 + 135 * 1, 180, 135}},
		{ID_BIGBOMB_6, {0}, {180 * 2, 115 + 135 * 1, 180, 135}},
		{ID_BIGBOMB_7, {0}, {180 * 3, 115 + 135 * 1, 180, 135}}
	};

#define NR_RES (sizeof(res_idc)/sizeof(res_idc[0]))

#define NR_ITEM NR_RES

static SURFACE *item[NR_ITEM];

void init_res()
{
    uint32_t i;
    SURFACE *pic = NULL;
    pic = load_bmp(IDB_IT);
    Assert(pic != NULL, "pic not found!\n");

	background = load_bmp(IDB_BK);
    Assert(background != NULL, "bk not found!\n");

    RECT srcrect, dstrect;

    for(i = 0; i < NR_RES; i++)
    {
        SURFACE *tmp = create_surface(res_idc[i].rect.w, res_idc[i].rect.h);
        if(tmp == NULL)
            panic("error occured!\n");
        srcrect.x = res_idc[i].rect.x;
        srcrect.y = res_idc[i].rect.y;
        dstrect.x = 0;
        dstrect.y = 0;
        dstrect.w = srcrect.w = res_idc[i].rect.w;
        dstrect.h = srcrect.h = res_idc[i].rect.h;

        blit_surface(pic, &srcrect, tmp, &dstrect);

        set_colorkey(tmp, 0x00ff00ff);

        item[res_idc[i].ID] = tmp;
    }
	flip(screen);
}

void update_screen()
{
	blit_surface(background, NULL, screen, NULL);
}

void show_item(uint32_t itemID, float x, float y)
{
    Assert((itemID >= 0) && (itemID < NR_ITEM), "Invalid itemID!\n");
	RECT srcrect, dstrect;

	srcrect.x = srcrect.y = 0;
	srcrect.w = item[itemID]->w;
	srcrect.h = item[itemID]->h;

	dstrect.x = x - (item[itemID]->w >> 1);
	dstrect.y = y - (item[itemID]->h >> 1);
	dstrect.w = item[itemID]->w;
	dstrect.h = item[itemID]->h;

	blit_surface(item[itemID], &srcrect, screen, &dstrect);
}

POINT query_itemwh(uint32_t itemID)
{
    Assert(itemID < NR_ITEM, "Invalid itemID!\n");
	return make_point(item[itemID]->w, item[itemID]->h);
}

bool is_offscreen(uint32_t itemID, POINT pos)
{
    int dx = abs(pos.x + ((item[itemID]->w - screen->w) >> 1));
    int dy = abs(pos.y + ((item[itemID]->h - screen->h) >> 1));
    int dw = ((item[itemID]->w + screen->w) >> 1);
    int dh = ((item[itemID]->h + screen->h) >> 1);
    return ((dx > dw) || (dy > dh));
}

bool query_item_collision(uint32_t itemID, int i, int j)
{
	assert(i >= 0 && i < item[itemID]->w && j>= 0 && j < item[itemID]->h);
	uint8_t * pixels = (void *)(item[itemID]->pixels);
	uint32_t * pos = (void *)pixels + 3 * i + j * item[itemID]->bpl;
	return ((pos[0] | 0xff000000) != 0xffff00ff);
}

uint32_t query_life_atk(uint32_t itemID)
{
	Assert(itemID < NR_ITEM, "Invalid itemID!\n");
	return res_idc[itemID].life;
}

void set_life_atk(uint32_t itemID, int life_atk)
{
	Assert(itemID < NR_ITEM, "Invalid itemID!\n");
	res_idc[itemID].life = life_atk;
}

void font_outip(int part, uint32_t total, int x, int y)
{
	uint32_t i, j;
	SURFACE *item_surface = item[ID_BLOOD];
	uint32_t *pixels = (uint32_t * )item_surface->pixels;
	if(part < 0)
		part = 0;
	else if((uint32_t)part >= total)
		part = total;

	for(i = 0; i < BLOOD_W; i ++)
	{
		pixels[i + 0 * BLOOD_W] = 0x00ff0000;
		pixels[i + (BLOOD_H - 1) * BLOOD_W] = 0x00ff0000;
	}

	for(j = 0; j < BLOOD_H; j ++)
	{
	 	pixels[0  + j * BLOOD_W] = 0x00ff0000;
		pixels[BLOOD_W - 1 + j * BLOOD_W] = 0x00ff0000;
	}

	for(i = 0; i < BLOOD_W * part / total; i++)
		for(j = 0; j < BLOOD_H; j++)
		{
			pixels[i + j * BLOOD_W] = 0x00ff0000;
		}
	for(i = BLOOD_W * part / total + 1; i < BLOOD_W; i++)
		for(j = 0; j < BLOOD_H; j++)
		{
			pixels[i + j * BLOOD_W] = 0x00ff00ff;
		}
	show_item(ID_BLOOD, x, y);

	font_out(x + BLOOD_W, y, 0x3f, 2, "%d/%d", part, total);
}
