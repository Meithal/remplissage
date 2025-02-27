#pragma once


#ifdef __APPLE__
#define NK_SHADER_VERSION "#version 150\n"
#else
#define NK_SHADER_VERSION "#version 300 es\n"
#endif

/* macros */
#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 800

#define MAX_VERTEX_MEMORY 512 * 1024
#define MAX_ELEMENT_MEMORY 128 * 1024

#define UNUSED(a) (void)a
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) < (b) ? (b) : (a))
#define LEN(a) (sizeof(a)/sizeof(a)[0])

struct media {
    GLint skin;
    struct nk_image menu;
    struct nk_image check;
    struct nk_image check_cursor;
    struct nk_image option;
    struct nk_image option_cursor;
    struct nk_image header;
    struct nk_image window;
    struct nk_image scrollbar_inc_button;
    struct nk_image scrollbar_inc_button_hover;
    struct nk_image scrollbar_dec_button;
    struct nk_image scrollbar_dec_button_hover;
    struct nk_image button;
    struct nk_image button_hover;
    struct nk_image button_active;
    struct nk_image tab_minimize;
    struct nk_image tab_maximize;
    struct nk_image slider;
    struct nk_image slider_hover;
    struct nk_image slider_active;
};

struct nk_glfw_vertex {
    float position[2];
    float uv[2];
    nk_byte col[4];
};

struct device {
    struct nk_buffer cmds;
    struct nk_draw_null_texture tex_null;
    GLuint vbo, vao, ebo;
    GLuint prog;
    GLuint vert_shdr;
    GLuint frag_shdr;
    GLint attrib_pos;
    GLint attrib_uv;
    GLint attrib_col;
    GLint uniform_tex;
    GLint uniform_proj;
    GLuint font_tex;
    struct media media;
    struct nk_font_atlas atlas;
    struct nk_context ctx;
    struct nk_font* font;
};

static struct nk_color g_current_color;


static void
color_shower(struct nk_context* ctx);
static void
right_click_panel(struct nk_context* ctx);
static void
device_init(struct device* dev);
static void
device_upload_atlas(struct device* dev, const void* image, int width, int height);

void device_draw(struct device* dev, struct nk_context* ctx, int width, int height,
    struct nk_vec2 scale, enum nk_anti_aliasing AA);
void device_shutdown(struct device* dev);
void text_input(GLFWwindow* win, unsigned int codepoint);
void scroll_input(GLFWwindow* win, double _, double yoff);
void device_main(const char* font_path, struct device* device);
void device_loop(struct nk_context* ctx, GLFWwindow* win, int width, int height,
int display_width, int display_height);
