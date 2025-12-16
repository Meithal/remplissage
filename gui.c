#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_BUTTON_TRIGGER_ON_RELEASE
#define NK_IMPLEMENTATION
#include "Nuklear//nuklear.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "gui.h"

static _Bool right_panel_showed = 0;

struct nk_font_atlas atlas;
struct nk_context ctx;

/* ===============================================================
 *
 *                          DEVICE
 *
 * ===============================================================*/

void device_main(const char* font_path, struct device * device)
{

    /* GUI */
    {
        device_init(device);
        {
            const void* image; int w, h;
            device->atlas = &atlas;
            nk_font_atlas_init_default(device->atlas);
            nk_font_atlas_begin(device->atlas);
            if (font_path) {
                device->font = nk_font_atlas_add_from_file(device->atlas, font_path, 13.0f, NULL);
            }
            else {
                device->font = nk_font_atlas_add_default(device->atlas, 13.0f, NULL);
            }
            image = nk_font_atlas_bake(device->atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
            device_upload_atlas(device, image, w, h);
            nk_font_atlas_end(device->atlas, nk_handle_id((int)device->font_tex), &device->tex_null);
        }
        device->ctx = &ctx;
        nk_init_default(device->ctx, &device->font->handle);
    }
}

/** Verifie les inputs utilisateurs et retourne si il faut redessinner ou non le canvas. */
_Bool device_loop(struct nk_context *ctx, GLFWwindow* win, int width, int height, struct remplissage_gui_bridge* gui_bridge)
{
    _Bool should_redraw = 0;

    /* Input */
    {
        double x, y;
        nk_input_begin(ctx);
        glfwPollEvents();
        nk_input_key(ctx, NK_KEY_DEL, glfwGetKey(win, GLFW_KEY_DELETE) == GLFW_PRESS);
        nk_input_key(ctx, NK_KEY_ENTER, glfwGetKey(win, GLFW_KEY_ENTER) == GLFW_PRESS);
        nk_input_key(ctx, NK_KEY_TAB, glfwGetKey(win, GLFW_KEY_TAB) == GLFW_PRESS);
        nk_input_key(ctx, NK_KEY_BACKSPACE, glfwGetKey(win, GLFW_KEY_BACKSPACE) == GLFW_PRESS);
        nk_input_key(ctx, NK_KEY_LEFT, glfwGetKey(win, GLFW_KEY_LEFT) == GLFW_PRESS);
        nk_input_key(ctx, NK_KEY_RIGHT, glfwGetKey(win, GLFW_KEY_RIGHT) == GLFW_PRESS);
        nk_input_key(ctx, NK_KEY_UP, glfwGetKey(win, GLFW_KEY_UP) == GLFW_PRESS);
        nk_input_key(ctx, NK_KEY_DOWN, glfwGetKey(win, GLFW_KEY_DOWN) == GLFW_PRESS);
        if (glfwGetKey(win, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
            glfwGetKey(win, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS) {
            nk_input_key(ctx, NK_KEY_COPY, glfwGetKey(win, GLFW_KEY_C) == GLFW_PRESS);
            nk_input_key(ctx, NK_KEY_PASTE, glfwGetKey(win, GLFW_KEY_P) == GLFW_PRESS);
            nk_input_key(ctx, NK_KEY_CUT, glfwGetKey(win, GLFW_KEY_X) == GLFW_PRESS);
            nk_input_key(ctx, NK_KEY_CUT, glfwGetKey(win, GLFW_KEY_E) == GLFW_PRESS);
            nk_input_key(ctx, NK_KEY_SHIFT, 1);
        }
        else {
            nk_input_key(ctx, NK_KEY_COPY, 0);
            nk_input_key(ctx, NK_KEY_PASTE, 0);
            nk_input_key(ctx, NK_KEY_CUT, 0);
            nk_input_key(ctx, NK_KEY_SHIFT, 0);
        }
        glfwGetCursorPos(win, &x, &y);
        nk_input_motion(ctx, (int)x, (int)y);
        nk_input_button(ctx, NK_BUTTON_LEFT, (int)x, (int)y, glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
        nk_input_button(ctx, NK_BUTTON_MIDDLE, (int)x, (int)y, glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS);
        nk_input_button(ctx, NK_BUTTON_RIGHT, (int)x, (int)y, glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);
        nk_input_end(ctx);
    }

    if (nk_input_is_mouse_released(&ctx->input, NK_BUTTON_RIGHT)) {
        right_panel_showed = !right_panel_showed;
    }

    if (right_panel_showed) {
        right_click_panel(ctx, gui_bridge);
    }

    /* Nos points à nous, càd si on survole pas une frame nuklear */
    if (!nk_window_is_any_hovered(ctx)) {
        
        
        gui_bridge->x_click_canvas = ctx->input.mouse.pos.x;
        gui_bridge->y_click_canvas = ctx->input.mouse.pos.y;
        gui_bridge->canvas_height = height;
        gui_bridge->canvas_width = width;
        
        if(nk_input_is_mouse_pressed(&ctx->input, NK_BUTTON_LEFT)) {
            gui_bridge->is_clicked_canvas = 1;
            should_redraw = 1;
        }

        if (nk_input_is_mouse_down(&ctx->input, NK_BUTTON_LEFT)) {
            gui_bridge->is_dragging_mouse = 1;
            should_redraw = 1;
        }
        
        if (nk_input_is_mouse_released(&ctx->input, NK_BUTTON_RIGHT)) {
            gui_bridge->is_mouse_released = 1;
            should_redraw = 1;
        }
    } else {
        should_redraw = 0;
    }
    
    should_redraw |= color_shower(ctx);

    return should_redraw;
}


static _Bool
color_shower(struct nk_context* ctx)
{
    _Bool refresh = 0;

    static char text[3][64];
    static int text_len[3];
    static const char* items[] = { "Item 0","item 1","item 2" };
    static int selected_item = 0;
    static int check = 1;

    if (nk_begin(ctx, "Selected color", nk_rect(600, 350, 275, 50),
        NK_WINDOW_TITLE | NK_WINDOW_BORDER | NK_WINDOW_MOVABLE |
        NK_WINDOW_NO_SCROLLBAR))
    {
        refresh = 1;

        nk_layout_row_dynamic(ctx, 10, 2);
        nk_label(ctx, "Couleur selectionne:", NK_TEXT_RIGHT);
        struct nk_rect total_space;
        total_space = nk_widget_bounds(ctx);
        total_space.w -= ctx->style.window.padding.x * 2;
        total_space.x += ctx->style.window.padding.x;

        nk_fill_rect(&ctx->current->buffer, total_space, 0, g_current_color);
    }
    nk_end(ctx);

    return refresh;
}

static void
die(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputs("\n", stderr);
    exit(EXIT_FAILURE);
}

static GLuint
image_load(const char* filename)
{
    int x, y, n;
    GLuint tex;
    unsigned char* data = stbi_load(filename, &x, &y, &n, 0);
    if (!data)
        die("failed to load image: %s", filename);

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
device_init(struct device* dev)
{
    GLint status;
    static const GLchar* vertex_shader =
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
    static const GLchar* fragment_shader =
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
device_upload_atlas(struct device* dev, const void* image, int width, int height)
{
    glGenTextures(1, &dev->font_tex);
    glBindTexture(GL_TEXTURE_2D, dev->font_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)width, (GLsizei)height, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, image);
}

void
device_shutdown(struct device* dev)
{
    glDeleteTextures(1, (const GLuint*)&dev->media.skin);
    nk_font_atlas_clear(dev->atlas);
    nk_free(dev->ctx);

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

void
device_draw(struct device* dev, struct nk_context* ctx, int width, int height,
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
        const struct nk_draw_command* cmd;
        void* vertices, * elements;
        const nk_draw_index* offset = NULL;

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
            {
                struct nk_buffer vbuf, ebuf;
                nk_buffer_init_fixed(&vbuf, vertices, MAX_VERTEX_MEMORY);
                nk_buffer_init_fixed(&ebuf, elements, MAX_ELEMENT_MEMORY);
                nk_convert(ctx, &dev->cmds, &vbuf, &ebuf, &config);
            }
        }
        glUnmapBuffer(GL_ARRAY_BUFFER);
        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

        /* iterate over and execute each draw command */
        nk_draw_foreach(cmd, ctx, &dev->cmds)
        {
            if (!cmd->elem_count)
                continue;

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


static void
right_click_panel(struct nk_context* ctx, struct remplissage_gui_bridge * gui_bridge)
{
    /* GUI */
    if (nk_begin(ctx, "Remplissage", nk_rect(ctx->input.mouse.pos.x, ctx->input.mouse.pos.y, 300, 400),
        NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_TITLE))
    {
        int i;
        float id;
        static int slider = 10;
        static int field_len;
        static nk_size prog_value = 60;
        static int current_weapon = 0;
        static char field_buffer[64];
        static float pos;
        static const char* weapons[] = {
                "Couleur",
                "Polygone à découper",
                "Tracé fenêtre",
                "Fenêtrage",
                "Remplissage" };
        const float step = (2 * 3.141592654f) / 32;

        nk_layout_row_static(ctx, 30, 120, 1);

        if (nk_button_label(ctx, "Couleur"))
            fprintf(stdout, "button pressed\n");

        nk_layout_row_dynamic(ctx, 120, 2);
        nk_label(ctx, "Couleur :", NK_TEXT_LEFT);
        struct nk_colorf color;
        nk_bool picked_color = nk_color_pick(ctx, &color, NK_RGB);
        //g_current_color = nk_rgb_cf(nk_color_picker(ctx, nk_color_cf(g_current_color), NK_RGB));
        gui_bridge->is_ask_change_color = picked_color;
        if(picked_color) {
            gui_bridge->remplissage_colors[0] = color.r;
            gui_bridge->remplissage_colors[1] = color.g;
            gui_bridge->remplissage_colors[2] = color.b;
            gui_bridge->remplissage_colors[3] = color.a;
        }
//        g_shapes[g_cur_shape].colors[0] = g_current_color.r;
//        g_shapes[g_cur_shape].colors[1] = g_current_color.g;
//        g_shapes[g_cur_shape].colors[2] = g_current_color.b;
//        g_shapes[g_cur_shape].colors[3] = g_current_color.a;

        nk_layout_row_dynamic(ctx, 25, 1);

        current_weapon = nk_combo(ctx, weapons, LEN(weapons), current_weapon, 25, nk_vec2(nk_widget_width(ctx), 200));

        printf("current selection %d\n", current_weapon);
        if(current_weapon == 2) {
            gui_bridge->is_asking_draw_clip = 1;
            right_panel_showed = 0;
            current_weapon = 0;
        }
    }
    nk_end(ctx);
}


void text_input(GLFWwindow* win, unsigned int codepoint)
{
    nk_input_unicode((struct nk_context*)glfwGetWindowUserPointer(win), codepoint);
}

void scroll_input(GLFWwindow* win, double _, double yoff)
{
    UNUSED(_);
    nk_input_scroll(
        (struct nk_context*)glfwGetWindowUserPointer(win),
        nk_vec2(0, (float)yoff)
    );
}
