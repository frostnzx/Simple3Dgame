#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <sstream>

const unsigned int SCR_WIDTH  = 1280;
const unsigned int SCR_HEIGHT = 720;
const float MAP_SIZE          = 25.0f;
const float PLAYER_SPEED      = 5.0f;
const float PLAYER_ROT_SPEED  = 120.0f;
const float PLAYER_RADIUS     = 0.6f;
const float COLLECT_RADIUS    = 0.7f;
const float OBSTACLE_RADIUS   = 0.5f;
const int   NUM_COLLECTIBLES  = 8;
const int   NUM_OBSTACLES     = 15;
const float CAM_DIST          = 6.0f;
const float CAM_HEIGHT        = 3.5f;

struct Player {
    glm::vec3 position = glm::vec3(0.0f);
    float rotY = 0.0f;
};

struct Collectible {
    glm::vec3 position = glm::vec3(0.0f);
    bool active = true;
    float phase = 0.0f;  
};

struct Obstacle {
    glm::vec3 position = glm::vec3(0.0f);
    float rotY  = 0.0f;
    float scale = 0.5f;
};

Player player;
std::vector<Collectible> collectibles;
std::vector<Obstacle>    obstacles;
int   score    = 0;
int   level    = 1;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
unsigned int groundVAO = 0, groundVBO = 0;
unsigned int groundTexID = 0;

void framebuffer_size_callback(GLFWwindow* w, int width, int height);
void processInput(GLFWwindow* w);
void setupGround();
unsigned int createGroundTexture();
void initGame();
void spawnCollectible(Collectible& c);
bool sphereHit(glm::vec3 a, float ra, glm::vec3 b, float rb);
void updateGame(GLFWwindow* w);
glm::vec3 playerForward();

glm::vec3 playerForward() {
    float r = glm::radians(player.rotY);
    return glm::vec3(sinf(r), 0.0f, cosf(r));
}

bool sphereHit(glm::vec3 a, float ra, glm::vec3 b, float rb) {
    return glm::length(a - b) < (ra + rb);
}

float randFloat() { return (float)rand() / (float)RAND_MAX; }

glm::vec3 randMapPos(float margin) {
    float m = MAP_SIZE - margin;
    return glm::vec3(randFloat() * 2.0f * m - m, 0.0f, randFloat() * 2.0f * m - m);
}

void spawnCollectible(Collectible& c) {
    for (int i = 0; i < 100; i++) {
        c.position = randMapPos(2.0f);
        if (glm::length(c.position - player.position) > 4.0f) break;
    }
    c.active = true;
    c.phase  = randFloat() * 360.0f;
}

void initGame() {
    player.position = glm::vec3(0.0f);
    player.rotY = 0.0f;
    score = 0;
    level = 1;

    // Spawn obstacles (away from center)
    obstacles.resize(NUM_OBSTACLES);
    for (auto& o : obstacles) {
        for (int i = 0; i < 100; i++) {
            o.position = randMapPos(2.0f);
            if (glm::length(o.position) > 4.0f) break;
        }
        o.rotY  = (float)(rand() % 360);
        o.scale = 0.3f + randFloat() * 0.5f;
    }

    // Spawn collectibles
    collectibles.resize(NUM_COLLECTIBLES);
    for (auto& c : collectibles) spawnCollectible(c);
}

void processInput(GLFWwindow* w) {
    if (glfwGetKey(w, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(w, true);

    if (glfwGetKey(w, GLFW_KEY_R) == GLFW_PRESS) initGame();

    glm::vec3 fwd = playerForward();
    if (glfwGetKey(w, GLFW_KEY_W) == GLFW_PRESS)
        player.position += fwd * PLAYER_SPEED * deltaTime;
    if (glfwGetKey(w, GLFW_KEY_S) == GLFW_PRESS)
        player.position -= fwd * PLAYER_SPEED * deltaTime;
    if (glfwGetKey(w, GLFW_KEY_A) == GLFW_PRESS)
        player.rotY += PLAYER_ROT_SPEED * deltaTime;
    if (glfwGetKey(w, GLFW_KEY_D) == GLFW_PRESS)
        player.rotY -= PLAYER_ROT_SPEED * deltaTime;

    // Clamp to map
    float b = MAP_SIZE - 1.0f;
    player.position.x = glm::clamp(player.position.x, -b, b);
    player.position.z = glm::clamp(player.position.z, -b, b);
    player.position.y = 0.0f;
}

void updateGame(GLFWwindow* w) {
    bool changed = false;

    // Collect planets
    for (auto& c : collectibles) {
        if (!c.active) continue;
        if (sphereHit(player.position, PLAYER_RADIUS, c.position, COLLECT_RADIUS)) {
            c.active = false;
            score += 10;
            changed = true;
        }
    }

    // Obstacle pushback
    for (auto& o : obstacles) {
        if (sphereHit(player.position, PLAYER_RADIUS, o.position, OBSTACLE_RADIUS * o.scale)) {
            glm::vec3 dir = player.position - o.position;
            if (glm::length(dir) > 0.001f) dir = glm::normalize(dir);
            else dir = glm::vec3(1, 0, 0);
            player.position = o.position + dir * (PLAYER_RADIUS + OBSTACLE_RADIUS * o.scale + 0.05f);
            player.position.y = 0.0f;
        }
    }

    // All collected -> next wave
    bool allDone = true;
    for (auto& c : collectibles)
        if (c.active) { allDone = false; break; }
    if (allDone) {
        level++;
        for (auto& c : collectibles) spawnCollectible(c);
        changed = true;
    }

    if (changed) {
        std::stringstream ss;
        ss << "Cyborg Collector 3D  |  Score: " << score << "  |  Level: " << level
           << "  |  W/S=Move  A/D=Turn  R=Restart";
        glfwSetWindowTitle(w, ss.str().c_str());
    }
}

void setupGround() {
    float s = MAP_SIZE;
    float t = MAP_SIZE / 2.0f;
    float v[] = {
        // pos                 normal              uv
        -s, 0.f, -s,   0.f, 1.f, 0.f,   0.f, 0.f,
         s, 0.f, -s,   0.f, 1.f, 0.f,    t,  0.f,
         s, 0.f,  s,   0.f, 1.f, 0.f,    t,   t,
        -s, 0.f, -s,   0.f, 1.f, 0.f,   0.f, 0.f,
         s, 0.f,  s,   0.f, 1.f, 0.f,    t,   t,
        -s, 0.f,  s,   0.f, 1.f, 0.f,   0.f,  t,
    };
    glGenVertexArrays(1, &groundVAO);
    glGenBuffers(1, &groundVBO);
    glBindVertexArray(groundVAO);
    glBindBuffer(GL_ARRAY_BUFFER, groundVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(6*sizeof(float)));
    glBindVertexArray(0);
}

unsigned int createGroundTexture() {
    const int SZ = 128;
    unsigned char data[SZ * SZ * 3];
    for (int i = 0; i < SZ; i++) {
        for (int j = 0; j < SZ; j++) {
            bool light = ((i / 16) + (j / 16)) % 2 == 0;
            int idx = (i * SZ + j) * 3;
            if (light) { data[idx]=100; data[idx+1]=170; data[idx+2]=80; }
            else       { data[idx]=70;  data[idx+1]=130; data[idx+2]=55; }
        }
    }
    unsigned int tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SZ, SZ, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    return tex;
}

int main()
{
    srand((unsigned int)time(NULL));

    // GLFW init
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT,
        "Cyborg Collector 3D  |  W/S=Move  A/D=Turn  R=Restart", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    stbi_set_flip_vertically_on_load(true);
    glEnable(GL_DEPTH_TEST);

    // Shaders
    Shader shader("game.vs", "game.fs");

    // Models
    std::cout << "Loading models..." << std::endl;
    Model playerModel(FileSystem::getPath("resources/objects/cyborg/cyborg.obj"));
    Model collectModel(FileSystem::getPath("resources/objects/planet/planet.obj"));
    Model rockModel(FileSystem::getPath("resources/objects/rock/rock.obj"));
    std::cout << "Models loaded!" << std::endl;

    // Ground
    setupGround();
    groundTexID = createGroundTexture();

    // Game init
    initGame();

    // Constant uniforms
    shader.use();
    shader.setVec3("lightDir",   glm::vec3(-0.3f, -1.0f, -0.5f));
    shader.setVec3("lightColor", glm::vec3(1.0f, 0.95f, 0.85f));
    shader.setFloat("ambientStrength", 0.35f);

    // ---- Render loop ----
    while (!glfwWindowShouldClose(window))
    {
        float now = (float)glfwGetTime();
        deltaTime = now - lastFrame;
        lastFrame = now;
        if (deltaTime > 0.1f) deltaTime = 0.1f;

        processInput(window);
        updateGame(window);

        // Sky color
        glClearColor(0.42f, 0.58f, 0.90f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();

        glm::vec3 fwd = playerForward();
        glm::vec3 camPos = player.position - fwd * CAM_DIST + glm::vec3(0.0f, CAM_HEIGHT, 0.0f);
        glm::vec3 camTgt = player.position + glm::vec3(0.0f, 1.0f, 0.0f);
        glm::mat4 view = glm::lookAt(camPos, camTgt, glm::vec3(0, 1, 0));
        glm::mat4 proj = glm::perspective(glm::radians(45.0f),
            (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 200.0f);
        shader.setMat4("view", view);
        shader.setMat4("projection", proj);
        shader.setVec3("viewPos", camPos);

        glm::mat4 mdl;

        shader.setBool("useTexture", true);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, groundTexID);
        shader.setInt("texture_diffuse1", 0);
        mdl = glm::mat4(1.0f);
        shader.setMat4("model", mdl);
        glBindVertexArray(groundVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        shader.setBool("useTexture", true);
        mdl = glm::mat4(1.0f);
        mdl = glm::translate(mdl, player.position);
        mdl = glm::rotate(mdl, glm::radians(player.rotY + 180.0f), glm::vec3(0, 1, 0));
        mdl = glm::scale(mdl, glm::vec3(0.8f));
        shader.setMat4("model", mdl);
        playerModel.Draw(shader);

        for (auto& c : collectibles) {
            if (!c.active) continue;
            shader.setBool("useTexture", true);
            float bob = sinf(now * 2.0f + c.phase) * 0.3f;
            mdl = glm::mat4(1.0f);
            mdl = glm::translate(mdl, c.position + glm::vec3(0, 0.8f + bob, 0));
            mdl = glm::rotate(mdl, glm::radians(c.phase + now * 60.0f), glm::vec3(0, 1, 0));
            mdl = glm::scale(mdl, glm::vec3(0.15f));
            shader.setMat4("model", mdl);
            collectModel.Draw(shader);
        }
        for (auto& o : obstacles) {
            shader.setBool("useTexture", true);
            mdl = glm::mat4(1.0f);
            mdl = glm::translate(mdl, o.position);
            mdl = glm::rotate(mdl, glm::radians(o.rotY), glm::vec3(0, 1, 0));
            mdl = glm::scale(mdl, glm::vec3(o.scale));
            shader.setMat4("model", mdl);
            rockModel.Draw(shader);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &groundVAO);
    glDeleteBuffers(1, &groundVBO);
    glDeleteTextures(1, &groundTexID);
    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow*, int w, int h) {
    glViewport(0, 0, w, h);
}
