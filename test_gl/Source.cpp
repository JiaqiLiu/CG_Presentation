/******************************************************************************\
| Learn and modified from "Anton's OpenGL 4 Tutorials"                         |
|                                                                              | 
| Jiaqi Liu                                                                    |
| 2015-06                                                                     |
\******************************************************************************/
#include "gl_utils.h" // utility functions discussed in earlier tutorials
#include <GL/glew.h> // include GLEW and new version of GL on Windows
#include <GLFW/glfw3.h> // GLFW helper library
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GL_LOG_FILE "gl.log"

using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;
using glm::cross;
using glm::normalize;
using glm::dot;

#pragma region data
vec4 g_camera_pos = vec4(0, 0, 5, 1);

static const vec4 g_light_pos = vec4(g_camera_pos);

mat4 g_model_mat = mat4(1.0f);
mat4 g_view_mat = glm::lookAt(
  vec3(g_camera_pos.x, g_camera_pos.y, g_camera_pos.z),
  vec3(0.0, 0.0, 0.0),
  vec3(0.0, 1.0, 0.0));
mat4 g_projection_mat = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);

GLfloat g_speed = 35.0f;
GLfloat g_delta_1 = abs(g_speed / 15.0f);
GLfloat g_delta_2 = abs(g_speed / 5.0f);

static const int g_triangle_num = 8;
int g_num_draw = 7;
GLfloat g_vertices[] = {
  0.0f,               -0.8f,  1.0f,
  sqrt(3.0f) / 2.0f,  -0.8f,  -0.5f,
  -sqrt(3.0f) / 2.0f, -0.8f,  -0.5f,
  0.0f,               0.8f,   1.0f,
  sqrt(3.0f) / 2.0f,  0.8f,   -0.5f,
  -sqrt(3.0f) / 2.0f, 0.8f,   -0.5f,
};
GLfloat g_triangles[g_triangle_num * 3 * 3];
GLfloat g_normal_data[g_triangle_num * 3 * 3];
#pragma endregion

// keep track of window size for things like the view port and the mouse cursor
int g_gl_width = 640;
int g_gl_height = 480;
GLFWwindow* g_window = NULL;

GLuint vertex_buffer;
GLuint normal_buffer;

// TODO: Please smooth the animation. Anti-aliasing. 

vec3 MyGetVertex(GLfloat array[], GLint i) {
  return vec3(array[i * 3 + 0], array[i * 3 + 1], array[i * 3 + 2]);
}

void MySetVertex(GLfloat array[], GLint i, vec3 v) {
  array[i * 3 + 0] = v.x;
  array[i * 3 + 1] = v.y;
  array[i * 3 + 2] = v.z;
}

void SetTriangle(GLint i, vec3 v1, vec3 v2, vec3 v3) {
  MySetVertex(g_triangles, i * 3 + 0, v1);
  MySetVertex(g_triangles, i * 3 + 1, v2);
  MySetVertex(g_triangles, i * 3 + 2, v3);
}

void CalculateNormals() {
  int vertex_num = g_triangle_num * 3;
  for (int i = 0; i < vertex_num; i = i + 3) {
    vec3 v1 = MyGetVertex(g_triangles, i);
    vec3 v2 = MyGetVertex(g_triangles, i + 1);
    vec3 v3 = MyGetVertex(g_triangles, i + 2);
    vec3 n = normalize(cross(v2 - v1, v3 - v1));
    MySetVertex(g_normal_data, i, n);
    MySetVertex(g_normal_data, i + 1, n);
    MySetVertex(g_normal_data, i + 2, n);
  }
}

// TODO Maybe it's faster if I change to pointer, instead of coping Vec3.
void UpdateTriangles() {
  SetTriangle(0, MyGetVertex(g_vertices, 0), MyGetVertex(g_vertices, 2), MyGetVertex(g_vertices, 1));
  SetTriangle(1, MyGetVertex(g_vertices, 0), MyGetVertex(g_vertices, 1), MyGetVertex(g_vertices, 3));
  SetTriangle(2, MyGetVertex(g_vertices, 1), MyGetVertex(g_vertices, 4), MyGetVertex(g_vertices, 3));
  SetTriangle(3, MyGetVertex(g_vertices, 1), MyGetVertex(g_vertices, 2), MyGetVertex(g_vertices, 4));
  SetTriangle(4, MyGetVertex(g_vertices, 2), MyGetVertex(g_vertices, 5), MyGetVertex(g_vertices, 4));
  SetTriangle(5, MyGetVertex(g_vertices, 2), MyGetVertex(g_vertices, 0), MyGetVertex(g_vertices, 5));
  SetTriangle(6, MyGetVertex(g_vertices, 3), MyGetVertex(g_vertices, 5), MyGetVertex(g_vertices, 0));
  // top
  SetTriangle(7, MyGetVertex(g_vertices, 3), MyGetVertex(g_vertices, 4), MyGetVertex(g_vertices, 5));

  CalculateNormals();
}

// Rotate some vertex. 
void RotateTriangles(GLfloat elapsed_time) {
  bool is_moved = false;
  mat4 R;
  if (glfwGetKey(g_window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
    R = glm::rotate(
      mat4(1.0f),
      g_speed * elapsed_time,
      vec3(0, 1, 0)); 
    is_moved = true;
  }
  if (glfwGetKey(g_window, GLFW_KEY_LEFT) == GLFW_PRESS) {
    R = glm::rotate(
      mat4(1.0f),
      -g_speed * elapsed_time,
      vec3(0, 1, 0)); 
    is_moved = true;
  }    
  if (is_moved) {
    for (int i = 3; i < 6; ++i) {
      vec4 v = R * vec4(MyGetVertex(g_vertices, i), 1);
      MySetVertex(g_vertices, i, vec3(v));
    }
    UpdateTriangles();
    // TODO: Here is not good. You don't have to update all the data every time. Please find a better solution. 
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_triangles), g_triangles, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_normal_data), g_normal_data, GL_STATIC_DRAW);
  }
}

void KeyboardCallback(GLfloat elapsed_time) {
  if (glfwGetKey(g_window, GLFW_KEY_A) == GLFW_PRESS) {
    mat4 R = glm::rotate(
      mat4(1.0f),
      -g_speed * elapsed_time,
      vec3(0, 1, 0)); 
    g_model_mat = R * g_model_mat;
  }
  if (glfwGetKey(g_window, GLFW_KEY_D) == GLFW_PRESS) {
    mat4 R = glm::rotate(
      mat4(1.0f),
      g_speed * elapsed_time,
      vec3(0, 1, 0));
    g_model_mat = R * g_model_mat;
  }
  if (glfwGetKey(g_window, GLFW_KEY_W) == GLFW_PRESS) {
    mat4 R = glm::rotate(
      mat4(1.0f),
      -g_speed * elapsed_time,
      vec3(1, 0, 0));
    g_model_mat = R * g_model_mat;
  }
  if (glfwGetKey(g_window, GLFW_KEY_S) == GLFW_PRESS) {
    mat4 R = glm::rotate(
      mat4(1.0f),
      g_speed * elapsed_time,
      vec3(1, 0, 0)); 
    g_model_mat = R * g_model_mat;
  }
  if (glfwGetKey(g_window, GLFW_KEY_E) == GLFW_PRESS) {
    mat4 R = glm::rotate(
      mat4(1.0f),
      -g_speed * elapsed_time,
      vec3(0, 0, 1));
    g_model_mat = R * g_model_mat;
  }
  if (glfwGetKey(g_window, GLFW_KEY_Q) == GLFW_PRESS) {
    mat4 R = glm::rotate(
      mat4(1.0f),
      g_speed * elapsed_time,
      vec3(0, 0, 1));
    g_model_mat = R * g_model_mat;
  }

  // Camera controller
  // Here I don't want to move the model. If I move it, then it will be near with the light, and the light will be too strong. 
  if (glfwGetKey(g_window, GLFW_KEY_UP) == GLFW_PRESS) {
    g_camera_pos = g_camera_pos - normalize(g_camera_pos) * g_speed * elapsed_time;
    g_view_mat = glm::lookAt(
      vec3(g_camera_pos.x, g_camera_pos.y, g_camera_pos.z),
      vec3(0.0, 0.0, 0.0),
      vec3(0.0, 1.0, 0.0));
  }
  if (glfwGetKey(g_window, GLFW_KEY_DOWN) == GLFW_PRESS) {
    g_camera_pos = g_camera_pos + normalize(g_camera_pos) * g_speed * elapsed_time;
    g_view_mat = glm::lookAt(
      vec3(g_camera_pos.x, g_camera_pos.y, g_camera_pos.z),
      vec3(0.0, 0.0, 0.0),
      vec3(0.0, 1.0, 0.0));
  }

  // change view point
  if (glfwGetKey(g_window, GLFW_KEY_Z) == GLFW_PRESS) {
    g_model_mat = mat4(1.0f);
  }
  if (glfwGetKey(g_window, GLFW_KEY_Y) == GLFW_PRESS) {
    g_model_mat = glm::rotate(
      mat4(1.0f),
      90.0f,
      vec3(1, 0, 0)); 
  } 

  // TODO: Writing like this will change lots of times by pressing only one time. I don't know how to make it better. 
  if (glfwGetKey(g_window, GLFW_KEY_T) == GLFW_PRESS) {
    if (g_num_draw != g_triangle_num) 
      g_num_draw = g_triangle_num;
    else
      g_num_draw = g_triangle_num - 1;
  }

  // change speed
  if (glfwGetKey(g_window, GLFW_KEY_0) == GLFW_PRESS) {
    g_speed += g_delta_1;
    std::cout << "speed: " << g_speed << std::endl;
  }
  if (glfwGetKey(g_window, GLFW_KEY_9) == GLFW_PRESS) {
    g_speed -= g_delta_1;
    if (g_speed < 0) 
      g_speed = 0;
    std::cout << "speed: " << g_speed << std::endl;
  }
  // change speed
  if (glfwGetKey(g_window, GLFW_KEY_8) == GLFW_PRESS) {
    g_speed += g_delta_2;
    std::cout << "speed: " << g_speed << std::endl;
  }
  if (glfwGetKey(g_window, GLFW_KEY_7) == GLFW_PRESS) {
    g_speed -= g_delta_2;
    if (g_speed < 0) 
      g_speed = 0;
    std::cout << "speed: " << g_speed << std::endl;
  }
}

int main() {
  assert(restart_gl_log());
  // all the GLFW and GLEW start-up code is moved to here in gl_utils.cpp
  assert(start_gl());
  glEnable(GL_DEPTH_TEST); // enable depth-testing
  glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"

  // We want to see inside, so we disable glCullFace;
  //glEnable(GL_CULL_FACE); // cull face
  //glCullFace(GL_BACK); // cull back face
  //glFrontFace(GL_CW); // GL_CCW for counter clock-wise

#pragma region shader
  // Shader
  char vertex_shader[1024 * 256];
  char fragment_shader[1024 * 256];
  assert(parse_file_into_str("test_vs.glsl", vertex_shader, 1024 * 256));
  assert(parse_file_into_str("test_fs.glsl", fragment_shader, 1024 * 256));

  GLuint vs = glCreateShader(GL_VERTEX_SHADER);
  const GLchar* p =(const GLchar*)vertex_shader;
  glShaderSource(vs, 1, &p, NULL);
  glCompileShader(vs);

  // check for compile errors
  int params = -1;
  glGetShaderiv(vs, GL_COMPILE_STATUS, &params);
  if(GL_TRUE != params) {
    fprintf(stderr, "ERROR: GL shader index %i did not compile\n", vs);
    print_shader_info_log(vs);
    return 1; // or exit or something
  }

  GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
  p = (const GLchar*)fragment_shader;
  glShaderSource(fs, 1, &p, NULL);
  glCompileShader(fs);

  // check for compile errors
  glGetShaderiv(fs, GL_COMPILE_STATUS, &params);
  if (GL_TRUE != params) {
    fprintf(stderr, "ERROR: GL shader index %i did not compile\n", fs);
    print_shader_info_log(fs);
    return 1; // or exit or something
  }

  GLuint shader_programme = glCreateProgram();
  glAttachShader(shader_programme, fs);
  glAttachShader(shader_programme, vs);
  glLinkProgram(shader_programme);

  glGetProgramiv(shader_programme, GL_LINK_STATUS, &params);
  if (GL_TRUE != params) {
    fprintf(
      stderr,
      "ERROR: could not link shader programme GL index %i\n",
      shader_programme
     );
    print_programme_info_log(shader_programme);
    return false;
  }
  glUseProgram(shader_programme);
#pragma endregion

  UpdateTriangles();
  GLuint projection_mat_id = glGetUniformLocation(shader_programme, "projection_mat");
  GLuint view_mat_id = glGetUniformLocation(shader_programme, "view_mat");
  GLuint model_mat_id = glGetUniformLocation(shader_programme, "model_mat");

  glGenBuffers(1, &vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_triangles), g_triangles, GL_STATIC_DRAW);
  glGenBuffers(1, &normal_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_normal_data), g_normal_data, GL_STATIC_DRAW);

  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  GLuint light_pos_id = glGetUniformLocation(shader_programme, "light_position_world");
  glUniform3f(light_pos_id, g_light_pos.x, g_light_pos.y + 1, g_light_pos.z);

  while (!glfwWindowShouldClose(g_window)) {
    // add a timer for doing animation
    static double previous_seconds = glfwGetTime();
    double current_seconds = glfwGetTime();
    double elapsed_seconds = current_seconds - previous_seconds;
    previous_seconds = current_seconds;

    _update_fps_counter(g_window);
    // wipe the drawing surface clear
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, g_gl_width, g_gl_height);
    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.3f, 0.0f);

    // Note: this call is not necessary, but I like to do it anyway before any
    // time that I call glDrawArrays() so I never use the wrong shader programme
    glUseProgram(shader_programme);

    // TODO: It's good to use callback, or combine those keyboard function together.
    // But I think that use them separately makes the program clearer. 
    RotateTriangles(static_cast<GLfloat>(elapsed_seconds));
    KeyboardCallback(static_cast<GLfloat>(elapsed_seconds));

    // TODO: You know, calling glUniformMatrix4fv is a little bit expensive. Please update those matrix only when necessary. 
    glUniformMatrix4fv(model_mat_id, 1, GL_FALSE, &g_model_mat[0][0]);
    glUniformMatrix4fv(view_mat_id, 1, GL_FALSE, &g_view_mat[0][0]);
    glUniformMatrix4fv(projection_mat_id, 1, GL_FALSE, &g_projection_mat[0][0]);
    // Note: this call is not necessary, but I like to do it anyway before any
    // time that I call glDrawArrays() so I never use the wrong vertex data
    glBindVertexArray(vao);
    // draw points 0-3 from the currently bound VAO with current in-use shader
    glDrawArrays(GL_TRIANGLES, 0, g_num_draw * 3);
    // update other events like input handling 
    glfwPollEvents();

    if (GLFW_PRESS == glfwGetKey(g_window, GLFW_KEY_ESCAPE)) {
      glfwSetWindowShouldClose(g_window, 1);
    }
    // put the stuff we've been drawing onto the display
    glfwSwapBuffers(g_window);
  }

  // Cleanup VBO and shader
  glDeleteBuffers(1, &vertex_buffer);
  glDeleteBuffers(1, &normal_buffer);
  glDeleteShader(vs);
  glDeleteShader(fs);
  glDeleteProgram(shader_programme);

  // close GL context and any other GLFW resources
  glfwTerminate();
  return 0;
}
