#include <GL/glut.h>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>

// Simulation parameters
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const float PARTICLE_RADIUS = 0.02f;
const int NUM_PARTICLES = 200;
const float ATTRACTION_RANGE = 0.15f;
const float GROWTH_RATE = 0.005f;
const float BROWNIAN_MOTION = 0.001f;

// Particle structure
struct Particle {
    float x, y, z;      // Position
    float radius;       // Size
    bool isAttached;    // Attached to cluster or free
    float dx, dy, dz;   // Velocity

    // Constructor for easy initialization
    Particle(float x, float y, float z, bool attached = false)
        : x(x), y(y), z(z), radius(PARTICLE_RADIUS),
        isAttached(attached), dx(0), dy(0), dz(0) {
    }
};

std::vector<Particle> particles;

void initializeSimulation() {
    // Set random seed
    srand(time(0));

    // Create central seed particle
    particles.emplace_back(0.0f, 0.0f, 0.0f, true);
    particles[0].radius = PARTICLE_RADIUS * 2; // Make seed slightly larger

    // Create free particles
    for (int i = 0; i < NUM_PARTICLES; ++i) {
        // Random position in cube (-1 to 1 range)
        float x = (rand() % 200 - 100) / 100.0f;
        float y = (rand() % 200 - 100) / 100.0f;
        float z = (rand() % 200 - 100) / 100.0f;

        // Small random initial velocity
        float dx = (rand() % 100 - 50) / 50000.0f;
        float dy = (rand() % 100 - 50) / 50000.0f;
        float dz = (rand() % 100 - 50) / 50000.0f;

        particles.emplace_back(x, y, z);
        particles.back().dx = dx;
        particles.back().dy = dy;
        particles.back().dz = dz;
    }
}

float distance(const Particle& a, const Particle& b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    float dz = a.z - b.z;
    return sqrt(dx * dx + dy * dy + dz * dz);
}

void updateParticles() {
    for (auto& p : particles) {
        if (p.isAttached) continue; // Skip already attached particles

        // Apply Brownian motion (random movement)
        p.dx += (rand() % 100 - 50) * BROWNIAN_MOTION;
        p.dy += (rand() % 100 - 50) * BROWNIAN_MOTION;
        p.dz += (rand() % 100 - 50) * BROWNIAN_MOTION;

        // Update position
        p.x += p.dx;
        p.y += p.dy;
        p.z += p.dz;

        // Boundary checks (soft bounce)
        const float BOUNDARY = 1.0f;
        if (p.x < -BOUNDARY) { p.x = -BOUNDARY; p.dx *= -0.5f; }
        if (p.x > BOUNDARY) { p.x = BOUNDARY;  p.dx *= -0.5f; }
        if (p.y < -BOUNDARY) { p.y = -BOUNDARY; p.dy *= -0.5f; }
        if (p.y > BOUNDARY) { p.y = BOUNDARY;  p.dy *= -0.5f; }
        if (p.z < -BOUNDARY) { p.z = -BOUNDARY; p.dz *= -0.5f; }
        if (p.z > BOUNDARY) { p.z = BOUNDARY;  p.dz *= -0.5f; }

        // Find nearest attached particle
        float minDist = INFINITY;
        Particle* nearestAttached = nullptr;

        for (auto& other : particles) {
            if (!other.isAttached) continue;

            float dist = distance(p, other);
            if (dist < minDist) {
                minDist = dist;
                nearestAttached = &other;
            }
        }

        // If near an attached particle, get attracted
        if (nearestAttached && minDist < ATTRACTION_RANGE) {
            // Calculate direction to nearest attached particle
            float dirX = nearestAttached->x - p.x;
            float dirY = nearestAttached->y - p.y;
            float dirZ = nearestAttached->z - p.z;

            // Normalize direction
            float length = sqrt(dirX * dirX + dirY * dirY + dirZ * dirZ);
            if (length > 0) {
                dirX /= length;
                dirY /= length;
                dirZ /= length;
            }

            // Apply attraction force
            const float ATTRACTION_FORCE = 0.0005f;
            p.dx += dirX * ATTRACTION_FORCE;
            p.dy += dirY * ATTRACTION_FORCE;
            p.dz += dirZ * ATTRACTION_FORCE;

            // Check for attachment (collision)
            if (minDist < (nearestAttached->radius + p.radius) * 0.9f) {
                p.isAttached = true;
                p.radius += GROWTH_RATE;
                p.dx = p.dy = p.dz = 0.0f; // Stop moving when attached
            }
        }
    }
}

void renderScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Set camera position
    gluLookAt(2.0, 2.0, 2.0,  // Eye position
        0.0, 0.0, 0.0,   // Look at position
        0.0, 1.0, 0.0);  // Up vector

    // Render particles
    for (const auto& p : particles) {
        glPushMatrix();
        glTranslatef(p.x, p.y, p.z);

        // Set color (blue for attached, red for free)
        float color[4];
        if (p.isAttached) {
            color[0] = 0.2f; color[1] = 0.2f; color[2] = 1.0f; color[3] = 1.0f;
        }
        else {
            color[0] = 1.0f; color[1] = 0.2f; color[2] = 0.2f; color[3] = 1.0f;
        }
        glMaterialfv(GL_FRONT, GL_DIFFUSE, color);

        glutSolidSphere(p.radius, 20, 20);
        glPopMatrix();
    }

    glutSwapBuffers();
}

void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (float)width / height, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

void update(int value) {
    updateParticles();
    glutPostRedisplay();
    glutTimerFunc(16, update, 0); // ~60 FPS
}

int main(int argc, char** argv) {
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("Nanoparticle Aggregation Simulation");

    // Enable lighting and depth testing
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // Initialize simulation
    initializeSimulation();

    // Set callback functions
    glutDisplayFunc(renderScene);
    glutReshapeFunc(reshape);
    glutTimerFunc(0, update, 0);

    // Start main loop
    glutMainLoop();
    return 0;
}