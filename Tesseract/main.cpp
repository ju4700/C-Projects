#include <GL/glut.h>
#include <vector>
#include <chrono>
#include <cmath>

struct Vec4 {
    float x, y, z, w;
    Vec4(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {}
};

struct Vec3 {
    float x, y, z;
    Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
};

std::vector<Vec4> tesseract_vertices = {
    Vec4(-1, -1, -1, -1), Vec4(1, -1, -1, -1), Vec4(-1, 1, -1, -1), Vec4(1, 1, -1, -1),
    Vec4(-1, -1, 1, -1), Vec4(1, -1, 1, -1), Vec4(-1, 1, 1, -1), Vec4(1, 1, 1, -1),
    Vec4(-1, -1, -1, 1), Vec4(1, -1, -1, 1), Vec4(-1, 1, -1, 1), Vec4(1, 1, -1, 1),
    Vec4(-1, -1, 1, 1), Vec4(1, -1, 1, 1), Vec4(-1, 1, 1, 1), Vec4(1, 1, 1, 1)
};

std::vector<std::pair<int, int>> edges = {
    {0,1}, {0,2}, {0,4}, {0,8}, {1,3}, {1,5}, {1,9}, {2,3},
    {2,6}, {2,10}, {3,7}, {3,11}, {4,5}, {4,6}, {4,12}, {5,7},
    {5,13}, {6,7}, {6,14}, {7,15}, {8,9}, {8,10}, {8,12}, {9,11},
    {9,13}, {10,11}, {10,14}, {11,15}, {12,13}, {12,14}, {13,15}, {14,15}
};

float angle_xy = 0.0f, angle_xz = 0.0f, angle_xw = 0.0f;
float angle_yz = 0.0f, angle_yw = 0.0f, angle_zw = 0.0f;

auto last_time = std::chrono::steady_clock::now();

Vec3 project4dTo3d(const Vec4& v, float w_distance = 4.0f, float z_distance = 5.0f) {
    float w_factor = w_distance / (w_distance - v.w);
    float x_3d = v.x * w_factor;
    float y_3d = v.y * w_factor;
    float z_3d = v.z * w_factor;

    float z_factor = z_distance / (z_distance - z_3d);
    return Vec3(x_3d * z_factor, y_3d * z_factor, z_3d);
}

Vec4 rotate4D(const Vec4& v, float theta_xy, float theta_xz, float theta_xw, float theta_yz, float theta_yw, float theta_zw) {
    float x = v.x, y = v.y, z = v.z, w = v.w;

    float new_x = x * cos(theta_xy) - y * sin(theta_xy);
    float new_y = x * sin(theta_xy) + y * cos(theta_xy);
    x = new_x;
    y = new_y;

    new_x = x * cos(theta_xz) - z * sin(theta_xz);
    float new_z = x * sin(theta_xz) + z * cos(theta_xz);
    x = new_x;
    z = new_z;

    new_x = x * cos(theta_xw) - w * sin(theta_xw);
    float new_w = x * sin(theta_xw) + w * cos(theta_xw);
    x = new_x;
    w = new_w;

    new_y = y * cos(theta_yz) - z * sin(theta_yz);
    new_z = y * sin(theta_yz) + z * cos(theta_yz);
    y = new_y;
    z = new_z;

    new_y = y * cos(theta_yw) - w * sin(theta_yw);
    new_w = y * sin(theta_yw) + w * cos(theta_yw);
    y = new_y;
    w = new_w;

    new_z = z * cos(theta_zw) - w * sin(theta_zw);
    new_w = z * sin(theta_zw) + w * cos(theta_zw);
    z = new_z;
    w = new_w;

    return Vec4(x, y, z, w);
}

void drawBackgroundGrid() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLineWidth(1.0f);
    glBegin(GL_LINES);

    float grid_size = 10.0f;
    int grid_steps = 10;
    float step = grid_size / grid_steps;
    float z_plane = -5.0f;

    for (int i = -grid_steps; i <= grid_steps; i++) {
        float x = i * step;
        float y = i * step;

        float distance = 8.0f - z_plane;
        float alpha = 1.0f - (std::abs(i) * step / grid_size);
        alpha = alpha * 0.5f;

        glColor4f(0.4f, 0.4f, 0.4f, alpha);
        glVertex3f(-grid_size, y, z_plane);
        glVertex3f(grid_size, y, z_plane);

        glColor4f(0.4f, 0.4f, 0.4f, alpha);
        glVertex3f(x, -grid_size, z_plane);
        glVertex3f(x, grid_size, z_plane);
    }

    glEnd();
    glDisable(GL_BLEND);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(0, 0, 8, 0, 0, 0, 0, 1, 0); // Camera at z=8

    drawBackgroundGrid();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);

    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glColor4f(1.0f, 1.0f, 1.0f, 0.8f);

    for (const auto& edge : edges) {
        Vec4 v1 = rotate4D(tesseract_vertices[edge.first], angle_xy, angle_xz, angle_xw, angle_yz, angle_yw, angle_zw);
        Vec4 v2 = rotate4D(tesseract_vertices[edge.second], angle_xy, angle_xz, angle_xw, angle_yz, angle_yw, angle_zw);
        Vec3 p1 = project4dTo3d(v1);
        Vec3 p2 = project4dTo3d(v2);
        glVertex3f(p1.x, p1.y, p1.z);
        glVertex3f(p2.x, p2.y, p2.z);
    }

    glEnd();
    glDisable(GL_BLEND);
    glutSwapBuffers();
}

void update(int value) {
    auto current_time = std::chrono::steady_clock::now();
    float delta_time = std::chrono::duration<float>(current_time - last_time).count();
    last_time = current_time;

    angle_xy += 0.5f * delta_time;
    angle_xz += 0.3f * delta_time;
    angle_xw += 0.4f * delta_time;
    angle_yz += 0.2f * delta_time;
    angle_yw += 0.35f * delta_time;
    angle_zw += 0.25f * delta_time;

    const float TWO_PI = 2.0f * M_PI;
    if (angle_xy > TWO_PI) angle_xy -= TWO_PI;
    if (angle_xz > TWO_PI) angle_xz -= TWO_PI;
    if (angle_xw > TWO_PI) angle_xw -= TWO_PI;
    if (angle_yz > TWO_PI) angle_yz -= TWO_PI;
    if (angle_yw > TWO_PI) angle_yw -= TWO_PI;
    if (angle_zw > TWO_PI) angle_zw -= TWO_PI;

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

void init() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    GLfloat light_pos[] = {5.0f, 5.0f, 5.0f, 1.0f};
    GLfloat light_ambient[] = {0.3f, 0.3f, 0.3f, 1.0f};
    GLfloat light_diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat light_specular[] = {0.5f, 0.5f, 0.5f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, light_specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 50.0f);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    glMatrixMode(GL_PROJECTION);
    gluPerspective(45.0, 800.0 / 600.0, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Tesseract");
    init();
    glutDisplayFunc(display);
    glutTimerFunc(0, update, 0);
    glutMainLoop();
    return 0;
}