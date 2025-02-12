/* nuklear - v1.05 - public domain */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <time.h>
#include <limits.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#include "Nuklear//nuklear.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

/* macros */
#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 800

#define MAX_VERTEX_MEMORY 512 * 1024
#define MAX_ELEMENT_MEMORY 128 * 1024

#define UNUSED(a) (void)a
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) < (b) ? (b) : (a))
#define LEN(a) (sizeof(a)/sizeof(a)[0])

#ifdef __APPLE__
#define NK_SHADER_VERSION "#version 150\n"
#else
#define NK_SHADER_VERSION "#version 300 es\n"
#endif

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


/* ===============================================================
 *
 *                          DEVICE
 *
 * ===============================================================*/
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
};

static void
die(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputs("\n", stderr);
    exit(EXIT_FAILURE);
}

static GLuint
image_load(const char *filename)
{
    int x,y,n;
    GLuint tex;
    unsigned char *data = stbi_load(filename, &x, &y, &n, 0);
    if (!data) die("failed to load image: %s", filename);

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    return tex;
}

static void
device_init(struct device *dev)
{
    GLint status;
    static const GLchar *vertex_shader =
            NK_SHADER_VERSION
            "uniform mat4 ProjMtx;\n"
            "in vec2 Position;\n"
            "in vec2 TexCoord;\n"
            "in vec4 Color;\n"
            "out vec2 Frag_UV;\n"
            "out vec4 Frag_Color;\n"
            "void main() {\n"
            "   Frag_UV = TexCoord;\n"
            "   Frag_Color = Color;\n"
            "   gl_Position = ProjMtx * vec4(Position.xy, 0, 1);\n"
            "}\n";
    static const GLchar *fragment_shader =
            NK_SHADER_VERSION
            "precision mediump float;\n"
            "uniform sampler2D Texture;\n"
            "in vec2 Frag_UV;\n"
            "in vec4 Frag_Color;\n"
            "out vec4 Out_Color;\n"
            "void main(){\n"
            "   Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
            "}\n";

    nk_buffer_init_default(&dev->cmds);
    dev->prog = glCreateProgram();
    dev->vert_shdr = glCreateShader(GL_VERTEX_SHADER);
    dev->frag_shdr = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(dev->vert_shdr, 1, &vertex_shader, 0);
    glShaderSource(dev->frag_shdr, 1, &fragment_shader, 0);
    glCompileShader(dev->vert_shdr);
    glCompileShader(dev->frag_shdr);
    glGetShaderiv(dev->vert_shdr, GL_COMPILE_STATUS, &status);
    assert(status == GL_TRUE);
    glGetShaderiv(dev->frag_shdr, GL_COMPILE_STATUS, &status);
    assert(status == GL_TRUE);
    glAttachShader(dev->prog, dev->vert_shdr);
    glAttachShader(dev->prog, dev->frag_shdr);
    glLinkProgram(dev->prog);
    glGetProgramiv(dev->prog, GL_LINK_STATUS, &status);
    assert(status == GL_TRUE);

    dev->uniform_tex = glGetUniformLocation(dev->prog, "Texture");
    dev->uniform_proj = glGetUniformLocation(dev->prog, "ProjMtx");
    dev->attrib_pos = glGetAttribLocation(dev->prog, "Position");
    dev->attrib_uv = glGetAttribLocation(dev->prog, "TexCoord");
    dev->attrib_col = glGetAttribLocation(dev->prog, "Color");

    {
        /* buffer setup */
        GLsizei vs = sizeof(struct nk_glfw_vertex);
        size_t vp = offsetof(struct nk_glfw_vertex, position);
        size_t vt = offsetof(struct nk_glfw_vertex, uv);
        size_t vc = offsetof(struct nk_glfw_vertex, col);

        glGenBuffers(1, &dev->vbo);
        glGenBuffers(1, &dev->ebo);
        glGenVertexArrays(1, &dev->vao);

        glBindVertexArray(dev->vao);
        glBindBuffer(GL_ARRAY_BUFFER, dev->vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dev->ebo);

        glEnableVertexAttribArray((GLuint)dev->attrib_pos);
        glEnableVertexAttribArray((GLuint)dev->attrib_uv);
        glEnableVertexAttribArray((GLuint)dev->attrib_col);

        glVertexAttribPointer((GLuint)dev->attrib_pos, 2, GL_FLOAT, GL_FALSE, vs, (void*)vp);
        glVertexAttribPointer((GLuint)dev->attrib_uv, 2, GL_FLOAT, GL_FALSE, vs, (void*)vt);
        glVertexAttribPointer((GLuint)dev->attrib_col, 4, GL_UNSIGNED_BYTE, GL_TRUE, vs, (void*)vc);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

static void
device_upload_atlas(struct device *dev, const void *image, int width, int height)
{
    glGenTextures(1, &dev->font_tex);
    glBindTexture(GL_TEXTURE_2D, dev->font_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)width, (GLsizei)height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, image);
}

static void
device_shutdown(struct device *dev)
{
    glDetachShader(dev->prog, dev->vert_shdr);
    glDetachShader(dev->prog, dev->frag_shdr);
    glDeleteShader(dev->vert_shdr);
    glDeleteShader(dev->frag_shdr);
    glDeleteProgram(dev->prog);
    glDeleteTextures(1, &dev->font_tex);
    glDeleteBuffers(1, &dev->vbo);
    glDeleteBuffers(1, &dev->ebo);
    nk_buffer_free(&dev->cmds);
}

static void
device_draw(struct device *dev, struct nk_context *ctx, int width, int height,
            struct nk_vec2 scale, enum nk_anti_aliasing AA)
{
    GLfloat ortho[4][4] = {
            {2.0f, 0.0f, 0.0f, 0.0f},
            {0.0f,-2.0f, 0.0f, 0.0f},
            {0.0f, 0.0f,-1.0f, 0.0f},
            {-1.0f,1.0f, 0.0f, 1.0f},
    };
    ortho[0][0] /= (GLfloat)width;
    ortho[1][1] /= (GLfloat)height;

    /* setup global state */
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glActiveTexture(GL_TEXTURE0);

    /* setup program */
    glUseProgram(dev->prog);
    glUniform1i(dev->uniform_tex, 0);
    glUniformMatrix4fv(dev->uniform_proj, 1, GL_FALSE, &ortho[0][0]);
    {
        /* convert from command queue into draw list and draw to screen */
        const struct nk_draw_command *cmd;
        void *vertices, *elements;
        const nk_draw_index *offset = NULL;

        /* allocate vertex and element buffer */
        glBindVertexArray(dev->vao);
        glBindBuffer(GL_ARRAY_BUFFER, dev->vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dev->ebo);

        glBufferData(GL_ARRAY_BUFFER, MAX_VERTEX_MEMORY, NULL, GL_STREAM_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_ELEMENT_MEMORY, NULL, GL_STREAM_DRAW);

        /* load draw vertices & elements directly into vertex + element buffer */
        vertices = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        elements = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
        {
            /* fill convert configuration */
            struct nk_convert_config config;
            static const struct nk_draw_vertex_layout_element vertex_layout[] = {
                    {NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(struct nk_glfw_vertex, position)},
                    {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(struct nk_glfw_vertex, uv)},
                    {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, NK_OFFSETOF(struct nk_glfw_vertex, col)},
                    {NK_VERTEX_LAYOUT_END}
            };
            NK_MEMSET(&config, 0, sizeof(config));
            config.vertex_layout = vertex_layout;
            config.vertex_size = sizeof(struct nk_glfw_vertex);
            config.vertex_alignment = NK_ALIGNOF(struct nk_glfw_vertex);
            config.tex_null = dev->tex_null;
            config.circle_segment_count = 22;
            config.curve_segment_count = 22;
            config.arc_segment_count = 22;
            config.global_alpha = 1.0f;
            config.shape_AA = AA;
            config.line_AA = AA;

            /* setup buffers to load vertices and elements */
            {struct nk_buffer vbuf, ebuf;
                nk_buffer_init_fixed(&vbuf, vertices, MAX_VERTEX_MEMORY);
                nk_buffer_init_fixed(&ebuf, elements, MAX_ELEMENT_MEMORY);
                nk_convert(ctx, &dev->cmds, &vbuf, &ebuf, &config);}
        }
        glUnmapBuffer(GL_ARRAY_BUFFER);
        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

        /* iterate over and execute each draw command */
        nk_draw_foreach(cmd, ctx, &dev->cmds)
        {
            if (!cmd->elem_count) continue;
            glBindTexture(GL_TEXTURE_2D, (GLuint)cmd->texture.id);
            glScissor(
                    (GLint)(cmd->clip_rect.x * scale.x),
                    (GLint)((height - (GLint)(cmd->clip_rect.y + cmd->clip_rect.h)) * scale.y),
                    (GLint)(cmd->clip_rect.w * scale.x),
                    (GLint)(cmd->clip_rect.h * scale.y));
            glDrawElements(GL_TRIANGLES, (GLsizei)cmd->elem_count, GL_UNSIGNED_SHORT, offset);
            offset += cmd->elem_count;
        }
        nk_clear(ctx);
        nk_buffer_clear(&dev->cmds);
    }

    /* default OpenGL state */
    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glDisable(GL_BLEND);
    glDisable(GL_SCISSOR_TEST);
}

/* glfw callbacks (I don't know if there is a easier way to access text and scroll )*/
static void error_callback(int e, const char *d){printf("Error %d: %s\n", e, d);}
static void text_input(GLFWwindow *win, unsigned int codepoint)
{nk_input_unicode((struct nk_context*)glfwGetWindowUserPointer(win), codepoint);}
static void scroll_input(GLFWwindow *win, double _, double yoff)
{UNUSED(_);nk_input_scroll((struct nk_context*)glfwGetWindowUserPointer(win), nk_vec2(0, (float)yoff));}

int main(int argc, char *argv[])
{
    /* Platform */
    static GLFWwindow *win;
    int width = 0, height = 0;
    int display_width=0, display_height=0;

    /* GUI */
    struct device device;
    struct nk_font_atlas atlas;
    struct media media;
    struct nk_context ctx;
    struct nk_font *font;

    /* GLFW */
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        fprintf(stdout, "[GFLW] failed to init!\n");
        exit(1);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    win = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Demo", NULL, NULL);
    glfwMakeContextCurrent(win);
    glfwSetWindowUserPointer(win, &ctx);
    glfwSetCharCallback(win, text_input);
    glfwSetScrollCallback(win, scroll_input);
    glfwGetWindowSize(win, &width, &height);
    glfwGetFramebufferSize(win, &display_width, &display_height);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        fprintf(stderr, "Failed to initialize GLAD\n");
        exit(1);
    }

    /* OpenGL */
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    //glewExperimental = 1;
    /*if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to setup GLEW\n");
        exit(1);
    }*/


    /* GUI */
    {device_init(&device);
        {const void *image; int w, h;
            const char *font_path = (argc > 1) ? argv[1]: 0;
            nk_font_atlas_init_default(&atlas);
            nk_font_atlas_begin(&atlas);
            if (font_path) font = nk_font_atlas_add_from_file(&atlas, font_path, 13.0f, NULL);
            else font = nk_font_atlas_add_default(&atlas, 13.0f, NULL);
            image = nk_font_atlas_bake(&atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
            device_upload_atlas(&device, image, w, h);
            nk_font_atlas_end(&atlas, nk_handle_id((int)device.font_tex), &device.tex_null);}
        nk_init_default(&ctx, &font->handle);}

    while (!glfwWindowShouldClose(win))
    {
        /* High DPI displays */
        struct nk_vec2 scale;
        glfwGetWindowSize(win, &width, &height);
        glfwGetFramebufferSize(win, &display_width, &display_height);
        scale.x = (float)display_width/(float)width;
        scale.y = (float)display_height/(float)height;

        /* Input */
        {
            double x, y;
            nk_input_begin(&ctx);
            glfwPollEvents();
            nk_input_key(&ctx, NK_KEY_DEL, glfwGetKey(win, GLFW_KEY_DELETE) == GLFW_PRESS);
            nk_input_key(&ctx, NK_KEY_ENTER, glfwGetKey(win, GLFW_KEY_ENTER) == GLFW_PRESS);
            nk_input_key(&ctx, NK_KEY_TAB, glfwGetKey(win, GLFW_KEY_TAB) == GLFW_PRESS);
            nk_input_key(&ctx, NK_KEY_BACKSPACE, glfwGetKey(win, GLFW_KEY_BACKSPACE) == GLFW_PRESS);
            nk_input_key(&ctx, NK_KEY_LEFT, glfwGetKey(win, GLFW_KEY_LEFT) == GLFW_PRESS);
            nk_input_key(&ctx, NK_KEY_RIGHT, glfwGetKey(win, GLFW_KEY_RIGHT) == GLFW_PRESS);
            nk_input_key(&ctx, NK_KEY_UP, glfwGetKey(win, GLFW_KEY_UP) == GLFW_PRESS);
            nk_input_key(&ctx, NK_KEY_DOWN, glfwGetKey(win, GLFW_KEY_DOWN) == GLFW_PRESS);
            if (glfwGetKey(win, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
                glfwGetKey(win, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS) {
                nk_input_key(&ctx, NK_KEY_COPY, glfwGetKey(win, GLFW_KEY_C) == GLFW_PRESS);
                nk_input_key(&ctx, NK_KEY_PASTE, glfwGetKey(win, GLFW_KEY_P) == GLFW_PRESS);
                nk_input_key(&ctx, NK_KEY_CUT, glfwGetKey(win, GLFW_KEY_X) == GLFW_PRESS);
                nk_input_key(&ctx, NK_KEY_CUT, glfwGetKey(win, GLFW_KEY_E) == GLFW_PRESS);
                nk_input_key(&ctx, NK_KEY_SHIFT, 1);
            } else {
                nk_input_key(&ctx, NK_KEY_COPY, 0);
                nk_input_key(&ctx, NK_KEY_PASTE, 0);
                nk_input_key(&ctx, NK_KEY_CUT, 0);
                nk_input_key(&ctx, NK_KEY_SHIFT, 0);
            }
            glfwGetCursorPos(win, &x, &y);
            nk_input_motion(&ctx, (int)x, (int)y);
            nk_input_button(&ctx, NK_BUTTON_LEFT, (int)x, (int)y, glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
            nk_input_button(&ctx, NK_BUTTON_MIDDLE, (int)x, (int)y, glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS);
            nk_input_button(&ctx, NK_BUTTON_RIGHT, (int)x, (int)y, glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);
            nk_input_end(&ctx);}

        /* GUI */
        if (nk_begin(&ctx, "Demo", nk_rect(50, 50, 300, 400),
                     NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_TITLE))
        {
            int i;
            float id;
            static int slider = 10;
            static int field_len;
            static nk_size prog_value = 60;
            static int current_weapon = 0;
            static char field_buffer[64];
            static float pos;
            static const char *weapons[] = {"Couleur","Pistol","Shotgun","Plasma","BFG"};
            const float step = (2*3.141592654f) / 32;

            nk_layout_row_static(&ctx, 30, 120, 1);
            if (nk_button_label(&ctx, "button"))
                fprintf(stdout, "button pressed\n");

            nk_layout_row_dynamic(&ctx, 20, 1);
            nk_label(&ctx, "Label", NK_TEXT_LEFT);
            nk_layout_row_dynamic(&ctx, 30, 2);
            nk_check_label(&ctx, "inactive", 0);
            nk_check_label(&ctx, "active", 1);
            nk_option_label(&ctx, "active", 1);
            nk_option_label(&ctx, "inactive", 0);

            nk_layout_row_dynamic(&ctx, 30, 1);
            nk_slider_int(&ctx, 0, &slider, 16, 1);
            nk_layout_row_dynamic(&ctx, 20, 1);
            nk_progress(&ctx, &prog_value, 100, NK_MODIFIABLE);

            nk_layout_row_dynamic(&ctx, 25, 1);
            nk_edit_string(&ctx, NK_EDIT_FIELD, field_buffer, &field_len, 64, nk_filter_default);
            nk_property_float(&ctx, "#X:", -1024.0f, &pos, 1024.0f, 1, 1);
            current_weapon = nk_combo(&ctx, weapons, LEN(weapons), current_weapon, 25, nk_vec2(nk_widget_width(&ctx),200));

            nk_layout_row_dynamic(&ctx, 100, 1);
            if (nk_chart_begin_colored(&ctx, NK_CHART_LINES, nk_rgb(255,0,0), nk_rgb(150,0,0), 32, 0.0f, 1.0f)) {
                nk_chart_add_slot_colored(&ctx, NK_CHART_LINES, nk_rgb(0,0,255), nk_rgb(0,0,150),32, -1.0f, 1.0f);
                nk_chart_add_slot_colored(&ctx, NK_CHART_LINES, nk_rgb(0,255,0), nk_rgb(0,150,0), 32, -1.0f, 1.0f);
                for (id = 0, i = 0; i < 32; ++i) {
                    nk_chart_push_slot(&ctx, (float)fabs(sin(id)), 0);
                    nk_chart_push_slot(&ctx, (float)cos(id), 1);
                    nk_chart_push_slot(&ctx, (float)sin(id), 2);
                    id += step;
                }
            }
            nk_chart_end(&ctx);

            nk_layout_row_dynamic(&ctx, 250, 1);
            if (nk_group_begin(&ctx, "Standard", NK_WINDOW_BORDER|NK_WINDOW_BORDER))
            {
                if (nk_tree_push(&ctx, NK_TREE_NODE, "Window", NK_MAXIMIZED)) {
                    static int selected[8];
                    if (nk_tree_push(&ctx, NK_TREE_NODE, "Next", NK_MAXIMIZED)) {
                        nk_layout_row_dynamic(&ctx, 20, 1);
                        for (i = 0; i < 4; ++i)
                            nk_selectable_label(&ctx, (selected[i]) ? "Selected": "Unselected", NK_TEXT_LEFT, &selected[i]);
                        nk_tree_pop(&ctx);
                    }
                    if (nk_tree_push(&ctx, NK_TREE_NODE, "Previous", NK_MAXIMIZED)) {
                        nk_layout_row_dynamic(&ctx, 20, 1);
                        for (i = 4; i < 8; ++i)
                            nk_selectable_label(&ctx, (selected[i]) ? "Selected": "Unselected", NK_TEXT_LEFT, &selected[i]);
                        nk_tree_pop(&ctx);
                    }
                    nk_tree_pop(&ctx);
                }
                nk_group_end(&ctx);
            }
        }
        nk_end(&ctx);

        /* Draw */
        glViewport(0, 0, display_width, display_height);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.5882, 0.6666, 0.6666, 1.0f);
        device_draw(&device, &ctx, width, height, scale, NK_ANTI_ALIASING_ON);
        glfwSwapBuffers(win);
    }
    glDeleteTextures(1,(const GLuint*)&media.skin);
    nk_font_atlas_clear(&atlas);
    nk_free(&ctx);
    device_shutdown(&device);
    glfwTerminate();
    return 0;
}

