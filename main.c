/* Tiny Farm 2 - a Harvest Moon / Stardew style farming game for the GBA.
 * Freestanding C, no libraries, all graphics generated in code.
 * Features: 3 crops, fishing, mining rocks, village + NPC, day/night, rain,
 * stamina, shop, shipping bin, sound effects, battery save (SRAM). */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

#define REG16(a)      (*(volatile u16*)(a))
#define REG_DISPCNT   REG16(0x04000000)
#define REG_VCOUNT    REG16(0x04000006)
#define REG_BG0CNT    REG16(0x04000008)
#define REG_BG1CNT    REG16(0x0400000A)
#define REG_BG0HOFS   REG16(0x04000010)
#define REG_BG0VOFS   REG16(0x04000012)
#define REG_BLDCNT    REG16(0x04000050)
#define REG_BLDY      REG16(0x04000054)
#define REG_SND1L     REG16(0x04000060)
#define REG_SND1H     REG16(0x04000062)
#define REG_SND1X     REG16(0x04000064)
#define REG_SNDCNTL   REG16(0x04000080)
#define REG_SNDCNTH   REG16(0x04000082)
#define REG_SNDCNTX   REG16(0x04000084)
#define REG_KEYINPUT  REG16(0x04000130)
#define PAL_BG   ((volatile u16*)0x05000000)
#define PAL_OBJ  ((volatile u16*)0x05000200)
#define VRAM_BG  ((volatile u32*)0x06000000)
#define MAP0     ((volatile u16*)0x06004000)   /* world, 64x32 tiles (2 screenblocks) */
#define MAP1     ((volatile u16*)0x06005000)   /* HUD / text, 32x32 tiles */
#define VRAM_OBJ ((volatile u32*)0x06010000)
#define OAM      ((volatile u16*)0x07000000)
#define SRAM     ((volatile u8*)0x0E000000)

#define KEY_A 1
#define KEY_B 2
#define KEY_SELECT 4
#define KEY_START 8
#define KEY_RIGHT 16
#define KEY_LEFT 32
#define KEY_UP 64
#define KEY_DOWN 128
#define KEY_R 256
#define KEY_L 512

#define RGB(r,g,b) ((r)|((g)<<5)|((b)<<10))
#define FONT_BASE 128
#define W 32
#define H 16
#define NPC_X 23
#define NPC_Y 4

/* Tells emulators / flash carts that this ROM uses 32K SRAM saves. */
__attribute__((used)) static const char sram_tag[] = "SRAM_V113";

enum { T_GRASS, T_SOIL, T_SOILW, T_SEED, T_SEEDW, T_SPR, T_SPRW,
       T_R0, T_R0W, T_R1, T_R1W, T_R2, T_R2W,
       T_WATER, T_TREE, T_PATH, T_ROOF, T_WALL, T_DOOR, T_BIN, T_SHOP,
       T_ROCK, T_FLOWER, NTYPES };

static const u16 pal_bg[16] = {
  0, RGB(8,20,6), RGB(6,16,5), RGB(16,10,5), RGB(11,6,3), RGB(8,5,3),
  RGB(6,14,28), RGB(14,22,31), RGB(24,20,12), RGB(14,8,3), RGB(4,14,4),
  RGB(28,5,5), RGB(31,18,3), RGB(31,31,31), RGB(2,2,3), RGB(16,16,17)
};
static const u16 pal_obj[16] = {
  0, RGB(30,22,16), RGB(31,28,6), RGB(6,10,26), RGB(14,8,3), RGB(2,2,3),
  RGB(31,31,31), RGB(28,5,5), RGB(31,16,22), RGB(4,14,4), RGB(16,16,17),
  RGB(16,24,31), RGB(31,18,3), 0, 0, 0
};

/* 3x5 font: digits, A-Z, then ' ' : . ! ? > , - / ' heart */
static const u8 font[47][5] = {
  {7,5,5,5,7},{2,6,2,2,7},{7,1,7,4,7},{7,1,7,1,7},{5,5,7,1,1},
  {7,4,7,1,7},{7,4,7,5,7},{7,1,1,1,1},{7,5,7,5,7},{7,5,7,1,7},
  {2,5,7,5,5},{6,5,6,5,6},{3,4,4,4,3},{6,5,5,5,6},{7,4,6,4,7},{7,4,6,4,4},
  {3,4,5,5,3},{5,5,7,5,5},{7,2,2,2,7},{1,1,1,5,2},{5,5,6,5,5},{4,4,4,4,7},
  {5,7,7,5,5},{7,5,5,5,5},{2,5,5,5,2},{6,5,6,4,4},{2,5,5,7,3},{6,5,6,5,5},
  {3,4,2,1,6},{7,2,2,2,2},{5,5,5,5,7},{5,5,5,5,2},{5,5,7,7,5},{5,5,2,5,5},
  {5,5,2,2,2},{7,1,2,4,7},
  {0,0,0,0,0},{0,2,0,2,0},{0,0,0,0,2},{2,2,2,0,2},{6,1,2,0,2},{4,2,1,2,4},
  {0,0,0,2,4},{0,0,7,0,0},{1,1,2,4,4},{2,2,0,0,0},{5,7,7,2,0}
};

static const int DX[4] = {0, 0, -1, 1};
static const int DY[4] = {1, -1, 0, 0};
static const int need[3]   = {2, 3, 4};      /* waterings until ripe */
static const int cprice[3] = {20, 35, 60};   /* sell price per crop  */
static const int sprice[3] = {5, 10, 20};    /* seed price           */
static const char *const sname[3] = {"TURNIP", "POTATO", "PUMPKIN"};
static const char *const tname[5] = {"HOE", "CAN", "SED", "PIC", "ROD"};
static const char seedl[3] = {'T', 'P', 'U'};
static const int pw[6] = {1, 10, 100, 1000, 10000, 100000};

static const char *const npc_lines[7][2] = {
  {"HELLO! I'M ANN.", "WELCOME TO THE VILLAGE!"},
  {"TURNIPS RIPEN IN 2 WATERINGS.", "POTATOES 3, PUMPKINS 4."},
  {"WATER YOUR CROPS EVERY DAY", "OR THEY WON'T GROW."},
  {"FISH SELL FOR 30G EACH.", "TRY THE POND WITH THE ROD!"},
  {"ROCKS HIDE STONE. SMASH THEM", "WITH THE PICKAXE FOR 10G."},
  {"RAIN WATERS THE FIELD FOR", "YOU. WHAT LUCK!"},
  {"SLEEP BEFORE MIDNIGHT OR", "YOU WILL FAINT."}
};

/* ---------- game state (zero-initialised; set up in init_world) ---------- */
static u8 ter[H][W], ctype[H][W], prog[H][W], wet[H][W];
static int ptx, pty, pdir, pdx, pdy, moving, mprog;
static int tool, sel, day, gold, energy, clock_, tfr, rain, talked, friend, won;
static int seeds[3], crops[3], fish, stone;
static int fcount, msgt;

#ifdef HOST
void host_frame(void);
#endif

static u32 hash(u32 x, u32 y, u32 t) {
  u32 h = x * 73856093u ^ y * 19349663u ^ t * 83492791u;
  h ^= h >> 13; h *= 0x5bd1e995u; h ^= h >> 15;
  return h;
}

static void vsync(void) {
#ifdef HOST
  host_frame();
#else
  while (REG_VCOUNT >= 160) {}
  while (REG_VCOUNT < 160) {}
#endif
}

static int keys(void) { return (int)(~REG_KEYINPUT & 0x3FF); }

static void sfx(int f) {
  REG_SND1L = 0;
  REG_SND1H = (12 << 12) | (3 << 8) | (2 << 6);
  REG_SND1X = f | 0x8000;
}

/* ---------- procedural art ---------- */
static u8 soil_px(int y, int w) {
  if ((y & 3) == 3) return w ? 14 : 4;
  return w ? 5 : 3;
}

static u8 pix(int t, int x, int y) {
  if (t >= T_SEED && t <= T_R2W) {
    int w = !(t & 1);
    u8 c = soil_px(y, w);
    int dx = x - 8;
    if (t <= T_SPRW) {
      if (t <= T_SEEDW) {
        if ((x == 7 || x == 8) && (y == 9 || y == 10)) c = 10;
      } else {
        if ((x == 7 || x == 8) && y >= 6 && y <= 12) c = 10;
        if ((y == 7 || y == 8) && ((x >= 4 && x <= 6) || (x >= 9 && x <= 11))) c = 10;
      }
    } else {
      int ty = (t - T_R0) >> 1, dy;
      if (ty == 0) {                                  /* turnip */
        dy = y - 10;
        if ((x == 7 || x == 8) && y >= 3 && y <= 6) c = 10;
        if ((y == 5 || y == 6) && (x == 6 || x == 9)) c = 10;
        if (dx * dx + dy * dy <= 9) c = (y <= 9) ? 11 : 13;
      } else if (ty == 1) {                           /* potato */
        dy = y - 10;
        if ((x == 7 || x == 8) && y >= 4 && y <= 7) c = 10;
        if ((y == 5 || y == 6) && ((x >= 5 && x <= 6) || (x >= 9 && x <= 10))) c = 10;
        if (dx * dx * 2 + dy * dy * 3 <= 24) c = (((x + y) & 3) == 0) ? 9 : 8;
      } else {                                        /* pumpkin */
        dy = y - 9;
        if (x == 8 && y >= 3 && y <= 5) c = 10;
        if (dx * dx * 2 + dy * dy * 4 <= 40) c = (dx == 0) ? 11 : 12;
      }
    }
    return c;
  }
  switch (t) {
  case T_GRASS: return ((hash(x, y, 1) & 15) == 0) ? 2 : 1;
  case T_SOIL: case T_SOILW: return soil_px(y, t == T_SOILW);
  case T_WATER: return (((x + y * 2) & 7) == 0 && (y & 3) == 1) ? 7 : 6;
  case T_TREE: {
    int dx = x - 8, dy = y - 6;
    if (x >= 6 && x <= 9 && y >= 11) return 9;
    if (dx * dx + dy * dy <= 36 && y <= 12) return ((hash(x, y, 4) & 7) == 0) ? 2 : 10;
    return ((hash(x, y, 1) & 15) == 0) ? 2 : 1;
  }
  case T_PATH: return ((hash(x, y, 3) & 7) == 0) ? 4 : 8;
  case T_ROOF: return ((y & 3) == 3) ? 9 : 11;
  case T_WALL:
    if (x >= 4 && x <= 11 && y >= 3 && y <= 9)
      return (x == 4 || x == 11 || y == 3 || y == 9) ? 9 : 7;
    return (y == 15) ? 9 : 13;
  case T_DOOR:
    if (x >= 4 && x <= 11 && y >= 2) return (x == 10 && y == 9) ? 12 : 9;
    return 13;
  case T_BIN:
    if (x >= 2 && x <= 13 && y >= 5 && y <= 14) {
      if (y <= 7) return 8;
      if (y == 10) return 14;
      return 9;
    }
    return 1;
  case T_SHOP:
    if (y < 6) return ((x >> 2) & 1) ? 11 : 13;
    if (x < 2 || x > 13) return 9;
    if (y >= 9 && y <= 11) return 9;
    if (y == 8 && (x == 7 || x == 8)) return 12;
    return 1;
  case T_ROCK: {
    int dx = x - 8, dy = y - 9;
    if (dx * dx * 16 + dy * dy * 36 <= 576) {
      if (y >= 12) return 4;
      if (y <= 8 && x >= 5 && x <= 7) return 13;
      return 15;
    }
    return ((hash(x, y, 1) & 15) == 0) ? 2 : 1;
  }
  case T_FLOWER:
    if (x == 8 && y >= 8 && y <= 11) return 10;
    if (x == 8 && y == 7) return 12;
    if ((x == 7 && y == 7) || (x == 9 && y == 7) || (x == 8 && (y == 6 || y == 8))) return 11;
    if (x == 4 && y == 12) return 12;
    if ((x == 3 && y == 12) || (x == 5 && y == 12) || (x == 4 && (y == 11 || y == 13))) return 13;
    return ((hash(x, y, 1) & 15) == 0) ? 2 : 1;
  }
  return 0;
}

/* sprite art, OBJ palette */
static u8 ppix(int d, int x, int y) {
  if (y == 0 && x >= 6 && x <= 9) return 2;
  if (y >= 1 && y <= 3 && x >= 3 && x <= 12) return 2;
  if (y >= 4 && y <= 6 && x >= 5 && x <= 10) {
    if (d == 1) return 4;
    if (y == 5) {
      if (d == 0 && (x == 6 || x == 9)) return 5;
      if (d == 2 && x == 6) return 5;
      if (d == 3 && x == 9) return 5;
    }
    return 1;
  }
  if (y >= 7 && y <= 11 && x >= 4 && x <= 11) return 3;
  if (y >= 8 && y <= 10 && (x == 3 || x == 12)) return 1;
  if (y >= 12 && y <= 15 && x >= 5 && x <= 10) return (y >= 14) ? 5 : 4;
  return 0;
}

static u8 npix(int x, int y) {
  if (y == 0 && x >= 5 && x <= 10) return 12;
  if (y >= 1 && y <= 3 && x >= 4 && x <= 11) return ((y == 3) && (x == 4 || x == 11)) ? 8 : 12;
  if (y >= 4 && y <= 6 && x >= 5 && x <= 10) return (y == 5 && (x == 6 || x == 9)) ? 5 : 1;
  if (y >= 7 && y <= 13) {
    int hw = (y >= 10) ? 5 : 3;
    if (x >= 8 - hw - 1 && x <= 7 + hw + 1 && x >= 4 - (y >= 10) && x <= 11 + (y >= 10)) return 7;
  }
  if (y >= 14 && y <= 15 && ((x >= 6 && x <= 7) || (x >= 9 && x <= 10))) return 1;
  return 0;
}

static u8 cpix(int x, int y) {
  if ((x < 3 || x > 12) && (y == 0 || y == 15)) return 6;
  if ((y < 3 || y > 12) && (x == 0 || x == 15)) return 6;
  return 0;
}

static int gl(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'A' && c <= 'Z') return 10 + c - 'A';
  if (c >= 'a' && c <= 'z') return 10 + c - 'a';
  switch (c) {
  case ':': return 37; case '.': return 38; case '!': return 39; case '?': return 40;
  case '>': return 41; case ',': return 42; case '-': return 43; case '/': return 44;
  case '\'': return 45; case '~': return 46;
  }
  return 36;
}

static u8 gpix(int g, int x, int y) {
  if (x >= 1 && x <= 6 && y >= 2 && y <= 6) {
    int gx = (x - 1) >> 1, gy = y - 2;
    if ((font[g][gy] >> (2 - gx)) & 1) return (g == 46) ? 11 : 13;
  }
  return 14;
}

static void tile_obj(int tile, int kind, int d, int sx, int sy) {
  int row, col;
  for (row = 0; row < 8; row++) {
    u32 w = 0;
    for (col = 0; col < 8; col++) {
      int x = sx * 8 + col, y = sy * 8 + row;
      u8 c = 0;
      if (kind == 0) c = ppix(d, x, y);
      else if (kind == 1) c = cpix(x, y);
      else if (kind == 2) c = npix(x, y);
      else if (kind == 3) c = ((x == 4 && (y == 1 || y == 2)) || (x == 3 && (y == 3 || y == 4))) ? 11 : 0;
      w |= (u32)c << (col * 4);
    }
    VRAM_OBJ[tile * 8 + row] = w;
  }
}

static void init_gfx(void) {
  int i, t, k, row, col, g, d;
  for (i = 0; i < 16; i++) { PAL_BG[i] = pal_bg[i]; PAL_OBJ[i] = pal_obj[i]; }
  for (i = 0; i < 8; i++) VRAM_BG[i] = 0;
  for (t = 0; t < NTYPES; t++)
    for (k = 0; k < 4; k++) {
      int tile = 1 + t * 4 + k, sx = k & 1, sy = k >> 1;
      for (row = 0; row < 8; row++) {
        u32 w = 0;
        for (col = 0; col < 8; col++) w |= (u32)pix(t, sx * 8 + col, sy * 8 + row) << (col * 4);
        VRAM_BG[tile * 8 + row] = w;
      }
    }
  for (g = 0; g < 47; g++)
    for (row = 0; row < 8; row++) {
      u32 w = 0;
      for (col = 0; col < 8; col++) w |= (u32)gpix(g, col, row) << (col * 4);
      VRAM_BG[(FONT_BASE + g) * 8 + row] = w;
    }
  for (d = 0; d < 4; d++) for (k = 0; k < 4; k++) tile_obj(d * 4 + k, 0, d, k & 1, k >> 1);
  for (k = 0; k < 4; k++) tile_obj(16 + k, 1, 0, k & 1, k >> 1);   /* cursor */
  for (k = 0; k < 4; k++) tile_obj(20 + k, 2, 0, k & 1, k >> 1);   /* npc    */
  tile_obj(24, 3, 0, 0, 0);                                        /* rain   */
  for (i = 0; i < 2048; i++) MAP0[i] = 0;
  for (i = 0; i < 1024; i++) MAP1[i] = 0;
  for (i = 0; i < 128; i++) { OAM[i * 4] = 0x0200; OAM[i * 4 + 1] = 0; OAM[i * 4 + 2] = 0; }
  REG_SNDCNTX = 0x80; REG_SNDCNTL = 0xFF77; REG_SNDCNTH = 2;
}

/* ---------- text ---------- */
static char mb[48];
static int ml;
static void cat(const char *s) { while (*s && ml < 46) mb[ml++] = *s++; mb[ml] = 0; }
static void catn(int v, int d) {
  int i;
  if (v < 0) v = 0;
  for (i = d - 1; i >= 0; i--) {
    int dg = 0;
    while (v >= pw[i]) { v -= pw[i]; dg++; }
    if (ml < 46) mb[ml++] = (char)('0' + dg);
  }
  mb[ml] = 0;
}
static void catv(int v) { int d = 1; while (d < 5 && v >= pw[d]) d++; catn(v, d); }
static void mkreset(void) { ml = 0; mb[0] = 0; }

static void line(int row, int col, const char *s) {
  while (*s && col < 30) MAP1[row * 32 + col++] = (u16)(FONT_BASE + gl(*s++));
  while (col < 32) MAP1[row * 32 + col++] = 0;
}
static void clear_row(int row) { int i; for (i = 0; i < 32; i++) MAP1[row * 32 + i] = 0; }
static void msg(const char *s) { line(19, 0, s); msgt = 110; }

static void wait_a(void) {
  while (keys() & KEY_A) vsync();
  for (;;) { vsync(); if (keys() & KEY_A) break; }
  while (keys() & KEY_A) vsync();
}
static void dialog(const char *a, const char *b) {
  clear_row(19);
  line(17, 0, a); line(18, 0, b); line(19, 27, "A>");
  wait_a();
  clear_row(17); clear_row(18); clear_row(19);
  msgt = 0;
}

/* ---------- world ---------- */
static void draw_cell(int x, int y) {
  int t = ter[y][x], tx = x * 2, o;
  if (t == T_SOIL) {
    int w = wet[y][x] ? 1 : 0, c = ctype[y][x];
    if (c == 0) t = T_SOIL + w;
    else if (prog[y][x] == 0) t = T_SEED + w;
    else if (prog[y][x] < need[c - 1]) t = T_SPR + w;
    else t = T_R0 + (c - 1) * 2 + w;
  }
  t = 1 + t * 4;
  o = (tx >> 5) * 1024 + (y * 2) * 32 + (tx & 31);
  MAP0[o] = (u16)t; MAP0[o + 1] = (u16)(t + 1);
  MAP0[o + 32] = (u16)(t + 2); MAP0[o + 33] = (u16)(t + 3);
}

static void draw_all(void) {
  int x, y;
  for (y = 0; y < H; y++) for (x = 0; x < W; x++) draw_cell(x, y);
}

static void init_world(void) {
  static const u8 trees[][2] = {{12,5},{5,13},{8,11},{7,5},{18,4},{17,10},{21,11},{30,6},{27,6},{18,13},{20,9},{29,4}};
  int x, y, i;
  for (y = 0; y < H; y++) for (x = 0; x < W; x++) { ter[y][x] = T_GRASS; ctype[y][x] = 0; prog[y][x] = 0; wet[y][x] = 0; }
  for (x = 0; x < W; x++) { ter[0][x] = T_TREE; ter[H - 1][x] = T_TREE; }
  for (y = 0; y < H; y++) { ter[y][0] = T_TREE; ter[y][W - 1] = T_TREE; }
  for (y = 1; y < H - 1; y++) ter[y][15] = T_TREE;
  for (y = 9; y <= 13; y++) for (x = 11; x <= 14; x++) ter[y][x] = T_WATER;
  for (y = 9; y <= 13; y++) for (x = 25; x <= 29; x++) ter[y][x] = T_WATER;
  for (i = 0; i < (int)(sizeof(trees) / 2); i++) ter[trees[i][1]][trees[i][0]] = T_TREE;
  for (x = 2; x <= 4; x++) ter[1][x] = T_ROOF;
  ter[2][2] = T_WALL; ter[2][3] = T_DOOR; ter[2][4] = T_WALL; ter[2][5] = T_BIN; ter[2][9] = T_SHOP;
  for (x = 19; x <= 21; x++) { ter[1][x] = T_ROOF; ter[2][x] = T_WALL; }
  for (x = 26; x <= 28; x++) { ter[1][x] = T_ROOF; ter[2][x] = T_WALL; }
  for (x = 3; x <= 10; x++) ter[3][x] = T_PATH;
  for (y = 4; y <= 7; y++) ter[y][3] = T_PATH;
  for (x = 3; x <= 24; x++) ter[7][x] = T_PATH;
  ter[8][15] = T_PATH; ter[8][14] = T_PATH; ter[8][16] = T_PATH;
  for (y = 3; y <= 7; y++) ter[y][23] = T_PATH;
  for (y = 1; y < H - 1; y++) for (x = 1; x < W - 1; x++) {
    int field = (x >= 5 && x <= 14 && y >= 4 && y <= 8);
    if (ter[y][x] != T_GRASS || field) continue;
    if (hash(x, y, 12) % 20 == 0) ter[y][x] = T_ROCK;
    else if (hash(x, y, 11) % 12 == 0) ter[y][x] = T_FLOWER;
  }
  ptx = 3; pty = 4; pdir = 0; moving = 0; mprog = 0;
  tool = 0; sel = 0; day = 1; gold = 50; energy = 30; clock_ = 360; tfr = 0;
  rain = 0; talked = 0; friend = 0; won = 0;
  seeds[0] = 5; seeds[1] = 0; seeds[2] = 0; crops[0] = crops[1] = crops[2] = 0; fish = 0; stone = 0;
}

static int walkable(int x, int y) {
  int t;
  if (x < 0 || y < 0 || x >= W || y >= H) return 0;
  if (x == NPC_X && y == NPC_Y) return 0;
  t = ter[y][x];
  return t == T_GRASS || t == T_SOIL || t == T_PATH || t == T_FLOWER;
}

/* ---------- save / load (SRAM) ---------- */
#define SAVE_HDR 20
#define SAVE_LEN (SAVE_HDR + 3 * H * W)
static u16 save_sum(void) {
  u32 s = 0; int i;
  for (i = 0; i < SAVE_LEN; i++) if (i < 18 || i >= SAVE_HDR) s += SRAM[i];
  return (u16)s;
}
static void put3(int o, int v) { SRAM[o] = (u8)v; SRAM[o + 1] = (u8)(v >> 8); SRAM[o + 2] = (u8)(v >> 16); }
static int get3(int o) { return SRAM[o] | (SRAM[o + 1] << 8) | (SRAM[o + 2] << 16); }
static void save_game(void) {
  int i, x, y;
  SRAM[0] = 'T'; SRAM[1] = 'F'; SRAM[2] = 'R'; SRAM[3] = '3';
  SRAM[4] = (u8)day; SRAM[5] = (u8)(day >> 8);
  put3(6, gold);
  for (i = 0; i < 3; i++) { SRAM[9 + i] = (u8)seeds[i]; SRAM[12 + i] = (u8)crops[i]; }
  SRAM[15] = (u8)fish; SRAM[16] = (u8)stone; SRAM[17] = (u8)friend;
  i = SAVE_HDR;
  for (y = 0; y < H; y++) for (x = 0; x < W; x++) {
    SRAM[i] = ter[y][x]; SRAM[i + H * W] = ctype[y][x]; SRAM[i + 2 * H * W] = prog[y][x]; i++;
  }
  { u16 s = save_sum(); SRAM[18] = (u8)s; SRAM[19] = (u8)(s >> 8); }
}
static int load_game(void) {
  int i, x, y;
  if (SRAM[0] != 'T' || SRAM[1] != 'F' || SRAM[2] != 'R' || SRAM[3] != '3') return 0;
  if (save_sum() != (u16)(SRAM[18] | (SRAM[19] << 8))) return 0;
  day = SRAM[4] | (SRAM[5] << 8); gold = get3(6);
  for (i = 0; i < 3; i++) { seeds[i] = SRAM[9 + i]; crops[i] = SRAM[12 + i]; }
  fish = SRAM[15]; stone = SRAM[16]; friend = SRAM[17];
  i = SAVE_HDR;
  for (y = 0; y < H; y++) for (x = 0; x < W; x++) {
    u8 t = SRAM[i], c = SRAM[i + H * W], p = SRAM[i + 2 * H * W]; i++;
    if (t >= T_WATER || t == T_GRASS || t == T_SOIL) ter[y][x] = t; /* ignore corrupt ids */
    ctype[y][x] = (c <= 3) ? c : 0; prog[y][x] = p;
  }
  if (day < 1) day = 1;
  return 1;
}

/* ---------- HUD ---------- */
static void hud(void) {
  int h = 0, m = clock_, total = crops[0] + crops[1] + crops[2], i;
  while (m >= 60) { m -= 60; h++; }
  if (h >= 24) h -= 24;
  mkreset(); cat("D"); catn(day, 3); cat(" "); catn(h, 2); cat(":"); catn(m, 2);
  cat(" G"); catn(gold, 5); cat(" E"); catn(energy, 2); cat(" ~"); catn(friend, 2); if (rain) cat(" R");
  line(0, 0, mb);
  mkreset(); cat(">"); cat(tname[tool]); cat(" ");
  mb[ml++] = seedl[sel]; mb[ml] = 0; cat(":"); catn(seeds[sel], 2);
  cat(" C"); catn(total, 2); cat(" F"); catn(fish, 2); cat(" K"); catn(stone, 2);
  line(1, 0, mb);
  (void)i;
}

/* ---------- day cycle ---------- */
static int darkness(void) {
  int d = 0;
  if (clock_ >= 1080) { int t = clock_ - 1080; while (t >= 60) { t -= 60; d++; } d++; }
  return d > 7 ? 7 : d;
}
static void set_tint(void) { REG_BLDCNT = 0xD1; REG_BLDY = (u16)darkness(); }

static void fade(int from, int to) {
  int v = from, step = (to > from) ? 1 : -1;
  REG_BLDCNT = 0xFF;
  for (;;) {
    REG_BLDY = (u16)v;
    vsync(); vsync();
    if (v == to) break;
    v += step;
  }
}

static void new_day(int en) {
  int x, y;
  fade(darkness(), 16);
  for (y = 0; y < H; y++)
    for (x = 0; x < W; x++) {
      if (ter[y][x] == T_SOIL && ctype[y][x] && wet[y][x] && prog[y][x] < need[ctype[y][x] - 1]) prog[y][x]++;
      wet[y][x] = 0;
    }
  if (day < 999) day++;
  clock_ = 360; tfr = 0; energy = en; talked = 0; msgt = 0;
  rain = (day > 2 && (hash(day, 7, 5) & 7) < 2) ? 1 : 0;
  if (rain) for (y = 0; y < H; y++) for (x = 0; x < W; x++) if (ter[y][x] == T_SOIL) wet[y][x] = 1;
  ptx = 3; pty = 3; pdir = 1; moving = 0; mprog = 0;
  clear_row(19);
  draw_all(); hud();
  save_game();
  fade(16, 0);
  set_tint();
  if (rain) msg("IT'S RAINING TODAY!");
}

/* ---------- actions ---------- */
static int spend(int c) {
  if (energy < c) { msg("TOO TIRED! SLEEP AT HOME."); sfx(1500); return 0; }
  energy -= c; hud(); return 1;
}

static void talk(void) {
  int i = (int)((u32)(day * 3 + friend) % 7u);
  dialog(npc_lines[i][0], npc_lines[i][1]);
  if (!talked) {
    talked = 1;
    if (friend < 10) {
      friend++;
      if (friend == 3 || friend == 6 || friend == 9) {
        int s = friend / 3 - 1;
        seeds[s] += 3; if (seeds[s] > 99) seeds[s] = 99;
        mkreset(); cat("ANN GAVE YOU 3 "); cat(sname[s]); cat(" SEEDS!"); msg(mb);
      } else msg("ANN LIKES YOU MORE! ~");
    }
    hud(); sfx(1990);
  }
}

static void act(void) {
  int fx = ptx + DX[pdir], fy = pty + DY[pdir], t;
  if (fx < 0 || fy < 0 || fx >= W || fy >= H) return;
  if (fx == NPC_X && fy == NPC_Y) { talk(); return; }
  t = ter[fy][fx];
  if (t == T_DOOR) {
    if (clock_ < 720) { msg("TOO EARLY TO SLEEP."); return; }
    new_day(30); return;
  }
  if (t == T_BIN) {
    int total = fish * 30 + stone * 10, i;
    for (i = 0; i < 3; i++) { total += crops[i] * cprice[i]; crops[i] = 0; }
    fish = 0; stone = 0;
    if (total == 0) { msg("NOTHING TO SELL."); return; }
    gold += total; if (gold > 99999) gold = 99999;
    mkreset(); cat("SOLD FOR "); catv(total); cat("G!"); msg(mb);
    sfx(1990); hud();
    if (gold >= 10000 && !won) { won = 1; dialog("YOU ARE A FARM LEGEND!", "KEEP PLAYING AS LONG AS YOU LIKE."); }
    return;
  }
  if (t == T_SHOP) {
    if (seeds[sel] >= 99) { msg("YOU HAVE ENOUGH SEEDS."); return; }
    if (gold < sprice[sel]) { msg("NOT ENOUGH GOLD."); sfx(1500); return; }
    gold -= sprice[sel]; seeds[sel]++;
    mkreset(); cat("BOUGHT 1 "); cat(sname[sel]); cat(" SEED"); msg(mb);
    sfx(1950); hud(); return;
  }
  if (t == T_SOIL && ctype[fy][fx] && prog[fy][fx] >= need[ctype[fy][fx] - 1]) {   /* harvest */
    int c = ctype[fy][fx] - 1;
    if (crops[c] < 99) crops[c]++;
    ctype[fy][fx] = 0; prog[fy][fx] = 0;
    draw_cell(fx, fy); hud(); sfx(1950); return;
  }
  switch (tool) {
  case 0:                                                   /* hoe */
    if ((t == T_GRASS || t == T_FLOWER) && spend(2)) {
      ter[fy][fx] = T_SOIL; wet[fy][fx] = rain ? 1 : 0; ctype[fy][fx] = 0; prog[fy][fx] = 0;
      draw_cell(fx, fy); sfx(1750);
    }
    break;
  case 1:                                                   /* watering can */
    if (t == T_SOIL && !wet[fy][fx] && spend(1)) { wet[fy][fx] = 1; draw_cell(fx, fy); sfx(1850); }
    break;
  case 2:                                                   /* seeds */
    if (t == T_SOIL && ctype[fy][fx] == 0) {
      if (seeds[sel] > 0) {
        ctype[fy][fx] = (u8)(sel + 1); prog[fy][fx] = 0; seeds[sel]--;
        draw_cell(fx, fy); hud(); sfx(1800);
      } else msg("NO SEEDS! BUY THEM AT THE SHOP.");
    }
    break;
  case 3:                                                   /* pickaxe */
    if (t == T_ROCK && spend(3)) {
      ter[fy][fx] = T_GRASS; if (stone < 99) stone++;
      draw_cell(fx, fy); hud(); sfx(1300);
    } else if (t == T_SOIL && ctype[fy][fx] == 0 && spend(1)) {
      ter[fy][fx] = T_GRASS; wet[fy][fx] = 0; draw_cell(fx, fy); sfx(1750);
    }
    break;
  case 4:                                                   /* fishing rod */
    if (t == T_WATER && spend(2)) {
      if ((hash(fcount, day, gold) & 7) < 3) { if (fish < 99) fish++; msg("YOU CAUGHT A FISH!"); sfx(1990); }
      else { msg("NO BITE..."); sfx(1600); }
      hud();
    }
    break;
  }
}

/* ---------- rendering ---------- */
static void render(void) {
  int px = ptx * 16 - pdx * mprog, py = pty * 16 - pdy * mprog;
  int cx = px + 8 - 120, cy = py + 8 - 80, i, sx, sy, bob = (moving && (mprog & 4)) ? 1 : 0;
  if (cx < 0) cx = 0;
  if (cx > W * 16 - 240) cx = W * 16 - 240;
  if (cy < 0) cy = 0;
  if (cy > H * 16 - 160) cy = H * 16 - 160;
  REG_BG0HOFS = (u16)cx; REG_BG0VOFS = (u16)cy;
  sx = px - cx; sy = py - cy - bob;
  OAM[0] = (u16)(sy & 0xFF);
  OAM[1] = (u16)((sx & 0x1FF) | (1 << 14));
  OAM[2] = (u16)((pdir * 4) | (1 << 10));
  if (moving) OAM[4] = 0x0200;
  else {
    int fx = ptx + DX[pdir], fy = pty + DY[pdir];
    sx = fx * 16 - cx; sy = fy * 16 - cy;
    OAM[4] = (u16)(sy & 0xFF);
    OAM[5] = (u16)((sx & 0x1FF) | (1 << 14));
    OAM[6] = (u16)(16 | (1 << 10));
  }
  sx = NPC_X * 16 - cx; sy = NPC_Y * 16 - cy;
  if (sx > -16 && sx < 240) {
    OAM[8] = (u16)(sy & 0xFF);
    OAM[9] = (u16)((sx & 0x1FF) | (1 << 14));
    OAM[10] = (u16)(20 | (1 << 10));
  } else OAM[8] = 0x0200;
  for (i = 0; i < 16; i++) {
    int o = (3 + i) * 4;
    if (!rain) { OAM[o] = 0x0200; continue; }
    sx = (int)(((hash(i, 1, 1) & 255) * 240u) >> 8);
    sy = (int)(((hash(i, 2, 2) & 255) * 160u) >> 8);
    sy = (sy + fcount * 4 + (sx >> 3)) % 160;
    sx = (sx - (fcount >> 1)) % 240; if (sx < 0) sx += 240;
    OAM[o] = (u16)(sy & 0xFF);
    OAM[o + 1] = (u16)(sx & 0x1FF);
    OAM[o + 2] = (u16)(24 | (1 << 10));
  }
}

int main(void) {
  int prev = 0, loaded;
  REG_DISPCNT = 0x0080;
  init_gfx();
  init_world();
  loaded = !(keys() & KEY_SELECT) && load_game();
  REG_BG0CNT = 2 | (8 << 8) | (1 << 14);
  REG_BG1CNT = 0 | (10 << 8);
  set_tint();
  draw_all(); hud(); render();
  REG_DISPCNT = 0x0040 | 0x0100 | 0x0200 | 0x1000;
  if (loaded) dialog("WELCOME BACK TO TINY FARM!", "YOUR GAME WAS LOADED.");
  else {
    dialog("TINY FARM", "D-PAD MOVE  A USE  B/L/R TOOL");
    dialog("SELECT CHANGES SEED TYPE.", "SLEEP AT YOUR DOOR AFTER NOON.");
  }
  for (;;) {
    int k, pr;
    vsync();
    fcount++;
    k = keys(); pr = k & ~prev; prev = k;
    if (moving) {
      mprog -= 2;
      if (mprog <= 0) { mprog = 0; moving = 0; }
    }
    if (++tfr >= 300) {
      tfr = 0; clock_ += 10; set_tint(); hud();
      if (clock_ >= 1440) {
        dialog("YOU FAINTED FROM EXHAUSTION.", "YOU WAKE UP AT HOME.");
        new_day(15); prev = keys(); continue;
      }
      if (clock_ == 1080) msg("IT'S GETTING DARK...");
    }
    if (msgt > 0 && --msgt == 0) clear_row(19);
    if (!moving) {
      int d = -1;
      if (k & KEY_UP) d = 1; else if (k & KEY_DOWN) d = 0;
      else if (k & KEY_LEFT) d = 2; else if (k & KEY_RIGHT) d = 3;
      if (d >= 0) {
        int nx = ptx + DX[d], ny = pty + DY[d];
        pdir = d;
        if (walkable(nx, ny)) { ptx = nx; pty = ny; pdx = DX[d]; pdy = DY[d]; moving = 1; mprog = 16; }
      }
      if (pr & (KEY_B | KEY_R)) { tool = (tool + 1) % 5; hud(); }
      if (pr & KEY_L) { tool = (tool + 4) % 5; hud(); }
      if (pr & KEY_SELECT) { sel = (sel + 1) % 3; hud(); }
      if (pr & KEY_A) { act(); prev = keys(); }
    }
    render();
  }
  return 0;
}
