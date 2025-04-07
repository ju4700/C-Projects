#include <GL/glut.h>
#include <array>
#include <string>
#include <chrono>
#include <cmath>
#include <vector>

struct Vec4 {
    float x, y, z, w;
    constexpr Vec4(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {}
};

struct Particle {
    float x, y, z;
    float vx, vy, vz;
    float life;
};

struct Star {
    float x, y, z;
    float brightness;
};

constexpr std::array<Vec4, 16> vertices = {{
    Vec4(-1, -1, -1, -1), Vec4(1, -1, -1, -1), Vec4(-1, 1, -1, -1), Vec4(1, 1, -1, -1),
    Vec4(-1, -1, 1, -1), Vec4(1, -1, 1, -1), Vec4(-1, 1, 1, -1), Vec4(1, 1, 1, -1),
    Vec4(-1, -1, -1, 1), Vec4(1, -1, -1, 1), Vec4(-1, 1, -1, 1), Vec4(1, 1, -1, 1),
    Vec4(-1, -1, 1, 1), Vec4(1, -1, 1, 1), Vec4(-1, 1, 1, 1), Vec4(1, 1, 1, 1)
}};

constexpr std::array<std::array<int, 2>, 32> edges = {{
    {0,1}, {0,2}, {0,4}, {1,3}, {1,5}, {2,3}, {2,6}, {3,7},
    {4,5}, {4,6}, {5,7}, {6,7}, {8,9}, {8,10}, {8,12}, {9,11},
    {9,13}, {10,11}, {10,14}, {11,15}, {12,13}, {12,14}, {13,15},
    {14,15}, {0,8}, {1,9}, {2,10}, {3,11}, {4,12}, {5,13}, {6,14}, {7,15}
}};

float angle_xy = 0.0f, angle_xz = 0.0f, angle_xw = 0.0f;
float angle_yz = 0.0f, angle_yw = 0.0f, angle_zw = 0.0f;
float rotation_speed = 1.0f;
float color_time = 0.0f;
std::vector<Particle> particles;
std::vector<Star> stars;
auto last_time = std::chrono::steady_clock::now();

Vec4 rotate4D(const Vec4& v, float theta_xy, float theta_xz, float theta_xw, float theta_yz, float theta_yw, float theta_zw) {
    float x = v.x, y = v.y, z = v.z, w = v.w;

    float new_x = x * cos(theta_xy) - y * sin(theta_xy);
    float new_y = x * sin(theta_xy) + y * cos(theta_xy);
    x = new_x; y = new_y;

    new_x = x * cos(theta_xz) - z * sin(theta_xz);
    float new_z = x * sin(theta_xz) + z * cos(theta_xz);
    x = new_x; z = new_z;

    new_x = x * cos(theta_xw) - w * sin(theta_xw);
    float new_w = x * sin(theta_xw) + w * cos(theta_xw);
    x = new_x; w = new_w;

    new_y = y * cos(theta_yz) - z * sin(theta_yz);
    new_z = y * sin(theta_yz) + z * cos(theta_yz);
    y = new_y; z = new_z;

    new_y = y * cos(theta_yw) - w * sin(theta_yw);
    new_w = y * sin(theta_yw) + w * cos(theta_yw);
    y = new_y; w = new_w;

    new_z = z * cos(theta_zw) - w * sin(theta_zw);
    new_w = z * sin(theta_zw) + w * cos(theta_zw);
    z = new_z; w = new_w;

    return Vec4(x, y, z, w);
}

std::array<float, 3> project4Dto3D(const Vec4& v) {
    float w_factor = 4.0f / (4.0f - v.w);
    float x_3d = v.x * w_factor;
    float y_3d = v.y * w_factor;
    float z_3d = v.z * w_factor;
    float z_factor = 5.0f / (5.0f - z_3d);
    return {x_3d * z_factor, y_3d * z_factor, z_3d};
}

void drawTextAlongEdge(const std::array<float, 3>& start, const std::array<float, 3>& end) {
    std::string_view text = "eidmubarak";
    float dx = (end[0] - start[0]) / static_cast<float>(text.length());
    float dy = (end[1] - start[1]) / static_cast<float>(text.length());
    float dz = (end[2] - start[2]) / static_cast<float>(text.length());

    for (size_t i = 0; i < text.length(); ++i) {
        float x = start[0] + dx * i;
        float y = start[1] + dy * i;
        float z = start[2] + dz * i;
        glRasterPos3f(x, y, z);
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, text[i]);
    }
}

void drawExtendedEdge(const std::array<float, 3>& v) {
    glBegin(GL_LINES);
    float r = (sin(color_time) + 1.0f) / 2.0f;
    float g = (sin(color_time + 2.0f) + 1.0f) / 2.0f;
    float b = (sin(color_time + 4.0f) + 1.0f) / 2.0f;
    glColor4f(r, g, b, 0.8f);
    glVertex3fv(v.data());
    std::array<float, 3> extended = {v[0] * 1.5f, v[1] * 1.5f, v[2] * 1.5f};
    glVertex3fv(extended.data());
    glEnd();
}

void initStars() {
    for (int i = 0; i < 200; ++i) {
        Star s{
            static_cast<float>(rand()) / RAND_MAX * 20.0f - 10.0f,  // x
            static_cast<float>(rand()) / RAND_MAX * 20.0f - 10.0f,  // y
            static_cast<float>(rand()) / RAND_MAX * -10.0f - 5.0f,  // z (behind)
            static_cast<float>(rand()) / RAND_MAX * 0.5f + 0.5f     // brightness
        };
        stars.push_back(s);
    }
}

void drawStarrySky() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    for (const auto& s : stars) {
        float twinkle = sin(color_time * 2.0f + s.x + s.y) * 0.2f + 0.8f;
        glColor4f(1.0f, 1.0f, 1.0f, s.brightness * twinkle);
        glVertex3f(s.x, s.y, s.z);
    }
    glEnd();
    glDisable(GL_BLEND);
}

void updateParticles(float delta_time) {
    for (auto& p : particles) {
        p.x += p.vx * delta_time;
        p.y += p.vy * delta_time;
        p.z += p.vz * delta_time;
        p.life -= delta_time;
    }
    particles.erase(std::remove_if(particles.begin(), particles.end(),
        [](const Particle& p) { return p.life <= 0; }), particles.end());
}

void spawnParticles() {
    for (const auto& v : vertices) {
        auto p3d = project4Dto3D(rotate4D(v, angle_xy, angle_xz, angle_xw, angle_yz, angle_yw, angle_zw));
        Particle p{p3d[0], p3d[1], p3d[2],
                  static_cast<float>(rand()) / RAND_MAX * 0.2f - 0.1f,
                  static_cast<float>(rand()) / RAND_MAX * 0.2f - 0.1f,
                  static_cast<float>(rand()) / RAND_MAX * 0.2f - 0.1f,
                  1.0f};
        particles.push_back(p);
    }
}

void drawParticles() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPointSize(3.0f);
    glBegin(GL_POINTS);
    for (const auto& p : particles) {
        float r = (sin(color_time + p.x) + 1.0f) / 2.0f;
        float g = (sin(color_time + p.y) + 1.0f) / 2.0f;
        float b = (sin(color_time + p.z) + 1.0f) / 2.0f;
        glColor4f(r, g, b, p.life);
        glVertex3f(p.x, p.y, p.z);
    }
    glEnd();
    glDisable(GL_BLEND);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(0.0, 0.0, 8.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    drawStarrySky();
    drawParticles();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float r = (sin(color_time) + 1.0f) / 2.0f;
    float g = (sin(color_time + 2.0f) + 1.0f) / 2.0f;
    float b = (sin(color_time + 4.0f) + 1.0f);
    glColor3f(r, g, b);  // Colorful text
    for (const auto& edge : edges) {
        Vec4 v1 = rotate4D(vertices[edge[0]], angle_xy, angle_xz, angle_xw, angle_yz, angle_yw, angle_zw);
        Vec4 v2 = rotate4D(vertices[edge[1]], angle_xy, angle_xz, angle_xw, angle_yz, angle_yw, angle_zw);
        auto p1 = project4Dto3D(v1);
        auto p2 = project4Dto3D(v2);
        drawTextAlongEdge(p1, p2);
        drawExtendedEdge(p1);
        drawExtendedEdge(p2);
    }

    glutSwapBuffers();
}

void update(int value) {
    auto current_time = std::chrono::steady_clock::now();
    float delta_time = std::chrono::duration<float>(current_time - last_time).count();
    last_time = current_time;

    angle_xy += 0.5f * delta_time * rotation_speed;
    angle_xz += 0.3f * delta_time * rotation_speed;
    angle_xw += 0.4f * delta_time * rotation_speed;
    angle_yz += 0.2f * delta_time * rotation_speed;
    angle_yw += 0.35f * delta_time * rotation_speed;
    angle_zw += 0.25f * delta_time * rotation_speed;

    color_time += delta_time;
    const float TWO_PI = 2.0f * M_PI;
    if (angle_xy > TWO_PI) angle_xy -= TWO_PI;
    if (angle_xz > TWO_PI) angle_xz -= TWO_PI;
    if (angle_xw > TWO_PI) angle_xw -= TWO_PI;
    if (angle_yz > TWO_PI) angle_yz -= TWO_PI;
    if (angle_yw > TWO_PI) angle_yw -= TWO_PI;
    if (angle_zw > TWO_PI) angle_zw -= TWO_PI;

    updateParticles(delta_time);
    if (rand() % 10 < 2) spawnParticles();

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case '+': rotation_speed += 0.2f; break;
        case '-': rotation_speed = std::max(0.2f, rotation_speed - 0.2f); break;
        case 27: exit(0);
    }
}

void init() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    GLfloat light_pos[] = {5.0f, 5.0f, 5.0f, 1.0f};
    GLfloat light_ambient[] = {0.2f, 0.2f, 0.3f, 1.0f};
    GLfloat light_diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat light_specular[] = {0.5f, 0.5f, 0.5f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

    glEnable(GL_COLOR_MATERIAL);
    glClearColor(0.0f, 0.0f, 0.1f, 1.0f);
    initStars();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, static_cast<float>(w)/h, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1200, 900);
    glutCreateWindow("Eid Mubarak Tesseract");

    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutTimerFunc(0, update, 0);
    glutKeyboardFunc(keyboard);
    glutMainLoop();
    return 0;
}