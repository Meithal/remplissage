/* nuklear - v1.05 - public domain */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#include "Nuklear/nuklear.h"

#include "gui.h"
#include "shapes.h"
#include "decoupage.h"
#include "remplissage.h"

static
_Bool should_generate_texture = 1;

static
_Bool _is_in_clip_mode = 0;

/* Forme Lesly */


const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec2 aPos;\n"
"layout (location = 1) in vec2 aTexCoord;\n"
"out vec2 TexCoord;\n"
"void main()\n"
"{\n"
"  TexCoord = aTexCoord;\n"
"  gl_Position = vec4(aPos, 0.0, 1.0);\n"
"}\0";

/* // damier
const char* fragmentShaderSource = "#version 330 core\n"
"in vec2 TexCoord;\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"  float scale = 10.0;\n"
"  vec2 uv = TexCoord * scale;\n"
"  vec2 check = floor(mod(uv, 2.0));\n"
"  float color = mod(check.x + check.y, 2.0);\n"
"  FragColor = vec4(vec3(color), 1.0);\n"
"}\0";
 */
const char* fragmentShaderSource =
        "#version 330 core\n"
       "in vec2 TexCoord;\n"
       "out vec4 FragColor;\n"
       "uniform sampler2D texture1;\n"
       "void main()\n"
       "{\n"
       "  FragColor = texture(texture1, TexCoord);\n"
       "}\0";


unsigned int VBO, VAO, EBO, shaderProgram;

static float quad_vertices[] = {
        -1.0f, -1.0f,0.0f, 0.0f, // Bas gauche
        1.0f, -1.0f,1.0f, 0.0f, // Bas droit
        -1.0f, 1.0f,0.0f, 1.0f, // Haut gauche
        1.0f, 1.0f, 1.0f, 1.0f // Haut droit
};

unsigned int indices[] = {
        0, 1, 2,
        1, 3, 2
};

static void initPolygon() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    /*
    int n_pts = g_shapes[g_cur_shape].last_point;
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * n_pts * 2, g_shapes[g_cur_shape].points, GL_STATIC_DRAW);
     */
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof indices, indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

GLuint texture;

static void drawPolygon() {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);

    glBindVertexArray(VAO);
    /*int n_pts = g_shapes[g_cur_shape].last_point;
    glDrawArrays(GL_TRIANGLE_STRIP, 0, n_pts);*/
    glDrawElements(GL_TRIANGLES, sizeof indices / sizeof *indices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

#define TEX_WIDTH 512
#define TEX_HEIGHT 512
unsigned char textureData[TEX_WIDTH * TEX_HEIGHT * 3];

float cross(float w1, float h1, float w2, float h2) {
    return w1 * h2 - h1 * w2;
}

float angle(float y1, float x1, float y2, float x2, float pivoty, float pivotx)
{
    // on calcule produit scalaire des deux vecteurs
    // puis on utilise l'inverse du cosinus pourtrouver l'angle
    float dot = (y1 - pivoty) * (y2 - pivoty) + (x1 - pivotx) * (x2 - pivotx);
    float normeA = sqrtf(powf(y1 - pivoty, 2) + powf(x1 - pivotx, 2));
    float normeB = sqrtf(powf(y2 - pivoty, 2) + powf(x2 - pivotx, 2));

    float costheta = dot / (normeA * normeB);
    float rad = acosf(costheta);
    float degrees = rad * 180.0f / (float)M_PI;

    // si produit vectoriel negatif, on prend l'angle opposé
    /*if(cross(x1 - pivotx, y2 - pivoty,  y1 - pivoty, x2 - pivotx) < 0) {
        degrees = 360 - degrees;
    }*/

    return degrees;
}

#define LINE_THICKNESS 0.005f // La largeur des traits de polygone. TODO, rendre ca specifique a chaque polygone

// Distance d'un point (px, py) a un segment (x1, y1) -> (x2, y2)
float point_line_distance(float px, float py, float x1, float y1, float x2, float y2) {
    float A = px - x1;
    float B = py - y1;
    float C = x2 - x1;
    float D = y2 - y1;

    float dot = A * C + B * D;
    float len_sq = C * C + D * D;
    float param = (len_sq != 0) ? (dot / len_sq) : -1;

    float xx, yy;
    if (param < 0) {
        xx = x1;
        yy = y1;
    } else if (param > 1) {
        xx = x2;
        yy = y2;
    } else {
        xx = x1 + param * C;
        yy = y1 + param * D;
    }

    float dx = px - xx;
    float dy = py - yy;
    return sqrtf(dx * dx + dy * dy);
}

// Verifie si un point de notre canvas est proche d'un segment
// retourne l'id du segment qui est proche, -1 sinon
int is_on_line(float y, float x) {
    for (int i_shape_index = 0; i_shape_index < g_last_shape ; i_shape_index++) {
        int last = g_shapes[i_shape_index].last_point;
        for(int i = 0; i < last ; i++) {
            float y1 = g_shapes[i_shape_index].points[i].y;
            float x1 = g_shapes[i_shape_index].points[i].x;
            float y2 = g_shapes[i_shape_index].points[(i + 1) % last].y;
            float x2 = g_shapes[i_shape_index].points[(i + 1) % last].x;
            
            if(g_clips[g_cur_clip].last_point > 0) {
                if(!CohenSutherlandLineClip(
                                            &x1, &y1, &x2, &y2,
                                            g_clips[g_cur_clip].points[2].y, g_clips[g_cur_clip].points[0].y,
                                            g_clips[g_cur_clip].points[2].x, g_clips[g_cur_clip].points[0].x))
                    continue;
            }
            
            if (point_line_distance(x, y, x1, y1, x2, y2) < LINE_THICKNESS)
                return i_shape_index;
        }
    }
    return -1;
}

_Bool is_on_clip_line(float y, float x) {
    int last = g_clips[g_cur_clip].last_point;
    for(int i = 0; i < last ; i++) {
        float y1 = g_clips[g_cur_clip].points[i].y;
        float x1 = g_clips[g_cur_clip].points[i].x;
        float y2 = g_clips[g_cur_clip].points[(i + 1) % last].y;
        float x2 = g_clips[g_cur_clip].points[(i + 1) % last].x;

        if (point_line_distance(x, y, x1, y1, x2, y2) < LINE_THICKNESS)
            return 1;
    }

    return 0;
}

/**
 Redessine notre texture qui correspond au canvas
 */
void generateTexture()
{
    _Bool isInside = 0;
    struct vec2 intersections[RM_MAX_POINTS] = {0};


    for (int y = 0; y < TEX_HEIGHT ; y++) {

        /*
        calculate_intersections(
                (struct vec2){.y = -norm(y, TEX_HEIGHT), .x = norm(0, TEX_WIDTH)},
                (struct vec2){.y = -norm(y, TEX_HEIGHT), .x = norm(TEX_WIDTH, TEX_WIDTH)},
                        intersections);
*/

        for (int x = 0; x < TEX_WIDTH; x++) {
            
            int index = (y * TEX_WIDTH + x) * 3;
            float normX = norm(x, TEX_WIDTH);
            float normY = -norm(y, TEX_HEIGHT);
            
            int shape_id = -1;

            if ((shape_id = is_on_line( normY, normX)) > -1) {
                textureData[index] = g_shapes[shape_id].colors[0];
                textureData[index + 1] = g_shapes[shape_id].colors[1];
                textureData[index + 2] = g_shapes[shape_id].colors[2];
            }
            else if(is_on_clip_line( normY, normX)) {
                textureData[index] = g_clips[g_cur_clip].colors[0];
                textureData[index + 1] = g_clips[g_cur_clip].colors[1];
                textureData[index + 2] = g_clips[g_cur_clip].colors[2];
            }
            else {
                textureData[index] = 255;
                textureData[index + 1] = 255;
                textureData[index + 2] = 255;
            }
        }
    }
}

void loadTexture() {
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEX_WIDTH, TEX_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void treatInput(struct remplissage_gui_bridge * gui_bridge)
{
    /** Logique de dessin du clip */
    if(_is_in_clip_mode) {
        if(gui_bridge->is_clicked_canvas) {
            gui_bridge->is_clicked_canvas = 0;
            g_clips[g_cur_clip].points[0].y = convert_to_ratio(gui_bridge->y_click_canvas, 0, gui_bridge->canvas_height, -1, 1);
            g_clips[g_cur_clip].points[0].x = convert_to_ratio(gui_bridge->x_click_canvas, 0, gui_bridge->canvas_width, -1, 1);
            g_clips[g_cur_clip].last_point = 1;
            
        }
        else if (gui_bridge->is_dragging_mouse) {
            gui_bridge->is_dragging_mouse = 0;
            g_clips[g_cur_clip].last_point = 4;
            g_clips[g_cur_clip].points[2].y = convert_to_ratio(gui_bridge->y_click_canvas, 0, gui_bridge->canvas_height, -1, 1);
            g_clips[g_cur_clip].points[2].x = convert_to_ratio(gui_bridge->x_click_canvas, 0, gui_bridge->canvas_width, -1, 1);
            g_clips[g_cur_clip].points[1].y = g_clips[g_cur_clip].points[0].y;
            g_clips[g_cur_clip].points[1].x = g_clips[g_cur_clip].points[2].x;
            g_clips[g_cur_clip].points[3].y = g_clips[g_cur_clip].points[2].y;
            g_clips[g_cur_clip].points[3].x = g_clips[g_cur_clip].points[0].x;
            
        }
        
        if (gui_bridge->is_mouse_released) {
            gui_bridge->is_mouse_released = 0;
            g_clips[g_cur_clip].last_point = 0;
            

            _is_in_clip_mode = 0;
        }
    }
    /** Logique d'addition de points */
    else {
        if(gui_bridge->is_clicked_canvas) {
            gui_bridge->is_clicked_canvas = 0;
            int idx = g_shapes[g_active_shape].last_point;
            
            g_shapes[g_active_shape].points[idx].y = convert_to_ratio(gui_bridge->y_click_canvas, 0, gui_bridge->canvas_height, -1, 1);
            g_shapes[g_active_shape].points[idx].x = convert_to_ratio(gui_bridge->x_click_canvas, 0, gui_bridge->canvas_width, -1, 1);
            
            g_shapes[g_active_shape].last_point++;
            
            gui_bridge->is_drawing_shape = 1;
        }
    }
    
    if(gui_bridge->is_asking_end_draw_shape) {
        gui_bridge->is_asking_end_draw_shape = 0;
        gui_bridge->is_drawing_shape = 0;
        g_last_shape++;
        g_active_shape = g_last_shape - 1;
    }
    
    if(gui_bridge->is_ask_change_color) {
        gui_bridge->is_ask_change_color = 0;
        g_shapes[g_active_shape].colors[0] = gui_bridge->remplissage_colors[0];
        g_shapes[g_active_shape].colors[1] = gui_bridge->remplissage_colors[1];
        g_shapes[g_active_shape].colors[2] = gui_bridge->remplissage_colors[2];
        g_shapes[g_active_shape].colors[3] = gui_bridge->remplissage_colors[3];
    }
    
    if(gui_bridge->is_asking_draw_clip) {
        gui_bridge->is_asking_draw_clip = 0;
        _is_in_clip_mode = 1;
        g_clips[g_cur_clip].last_point = 0;
    }

}

/* glfw callbacks (I don't know if there is a easier way to access text and scroll )*/
static void error_callback(int e, const char *d){
    printf("Error %d: %s\n", e, d);
}

int main(int argc, char *argv[])
{
    /* Platform */
    static GLFWwindow *win;
    int width = 0, height = 0;
    int display_width=0, display_height=0;

    /* nuklear device */
    struct device device;

    /* bridge between our logic and the GUI */
    struct remplissage_gui_bridge
    gui_bridge;

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
    glClearColor(0.5882f, 0.6666f, 0.6666f, 1.0f);

    /* GUI */
    device_main((argc > 1) ? argv[1] : 0, &device);

    glfwSetWindowUserPointer(win, device.ctx);

    initPolygon();
    generateTexture();
    loadTexture();


    while (!glfwWindowShouldClose(win))
    {
        /* High DPI displays */
        struct nk_vec2 scale;
        glfwGetWindowSize(win, &width, &height);
        glfwGetFramebufferSize(win, &display_width, &display_height);
        scale.x = (float)display_width/(float)width;
        scale.y = (float)display_height/(float)height;

        should_generate_texture = device_loop(device.ctx, win, width, height, &gui_bridge);

        /* Draw */
        glViewport(0, 0, display_width, display_height);
        glClear(GL_COLOR_BUFFER_BIT);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        if(should_generate_texture) {
            treatInput(&gui_bridge);
            generateTexture();
            loadTexture();

            should_generate_texture = 0;
        }

        drawPolygon();

        device_draw(&device, device.ctx, width, height, scale, NK_ANTI_ALIASING_OFF);
        glfwSwapBuffers(win);
    }

    device_shutdown(&device);
    glfwTerminate();
    return 0;
}
