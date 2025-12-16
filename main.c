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


_Bool should_generate_texture = 1;

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

#define texWidth 512
#define texHeight 512
unsigned char textureData[texWidth * texHeight * 3];

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

#define LINE_THICKNESS 0.005f // Adjust this value for thicker/thinner edges

// Function to calculate the distance from a point (px, py) to a line segment (x1, y1) -> (x2, y2)
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

// Check if a point is close to any of the polygon's edges
_Bool is_on_line(float y, float x) {
    int last = g_shapes[g_cur_shape].last_point;
    for(int i = 0; i < last ; i++) {
        float y1 = g_shapes[g_cur_shape].points[i].y;
        float x1 = g_shapes[g_cur_shape].points[i].x;
        float y2 = g_shapes[g_cur_shape].points[(i + 1) % last].y;
        float x2 = g_shapes[g_cur_shape].points[(i + 1) % last].x;

        if(g_clips[g_cur_clip].last_point > 0) {
            if(!CohenSutherlandLineClip(
                    &x1, &y1, &x2, &y2,
                    g_clips[g_cur_clip].points[2].y, g_clips[g_cur_clip].points[0].y,
                    g_clips[g_cur_clip].points[2].x, g_clips[g_cur_clip].points[0].x))
                continue;
        }

        if (point_line_distance(x, y, x1, y1, x2, y2) < LINE_THICKNESS)
            return 1;
    }
    return 0;
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

void generateTexture()
{
    _Bool isInside = 0;
    struct vec2 intersections[RM_MAX_POINTS] = {0};


    for (int y = 0; y < texHeight ; y++) {

        /*
        calculate_intersections(
                (struct vec2){.y = -norm(y, texHeight), .x = norm(0, texWidth)},
                (struct vec2){.y = -norm(y, texHeight), .x = norm(texWidth, texWidth)},
                        intersections);
*/

        for (int x = 0; x < texWidth; x++) {
            int index = (y * texWidth + x) * 3;

            float normX = norm(x, texWidth);
            float normY = -norm(y, texHeight);

            if (is_on_line( normY, normX)) {
                textureData[index] = g_shapes[g_cur_shape].colors[0];
                textureData[index + 1] = g_shapes[g_cur_shape].colors[1];
                textureData[index + 2] = g_shapes[g_cur_shape].colors[2];
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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
    glfwSetWindowUserPointer(win, &device.ctx);
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

        should_generate_texture = device_loop(&device.ctx, win, width, height);

        /* Draw */
        glViewport(0, 0, display_width, display_height);
        glClear(GL_COLOR_BUFFER_BIT);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        if(should_generate_texture) {
            generateTexture();
            loadTexture();

            should_generate_texture = 0;
        }

        drawPolygon();

        device_draw(&device, &device.ctx, width, height, scale, NK_ANTI_ALIASING_OFF);
        glfwSwapBuffers(win);
    }

    device_shutdown(&device);
    glfwTerminate();
    return 0;
}
