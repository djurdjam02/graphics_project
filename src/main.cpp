#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

unsigned int loadTexture(const char *path, bool gammaCorrection);
unsigned int loadCubemap(vector<std::string> faces);
void setSpotLight(Shader& shader);
void renderSnowGround();
void renderQuad();

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

float heightScale = 0.1;
bool blinn = false;
bool blinnKeyPressed = false;
bool spotlight = false;
bool spotlightKeyPressed = false;
float exposure = 0.8;
bool bloom = false;
bool bloomKeyPressed = false;
bool waddle = false;
bool waddleKeyPressed = false;

struct ProgramState {
    glm::vec3 clearColor = glm::vec3(0.1);
    bool ImGuiEnabled = false;
    Camera camera;
    bool CameraMouseMovementUpdateEnabled = true;
    ProgramState()
            : camera(glm::vec3(8.0f, 1.5f, 5.0f)) {}

    void SaveToFile(std::string filename);

    void LoadFromFile(std::string filename);
};

void ProgramState::SaveToFile(std::string filename) {
    std::ofstream out(filename);
    out << clearColor.r << '\n'
        << clearColor.g << '\n'
        << clearColor.b << '\n';
}

void ProgramState::LoadFromFile(std::string filename) {
    std::ifstream in(filename);
    if (in) {
        in >> clearColor.r
           >> clearColor.g
           >> clearColor.b;
    }
}

ProgramState *programState;

void DrawImGui(ProgramState *programState);

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Arctic", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    programState = new ProgramState;
    programState->LoadFromFile("resources/program_state.txt");

    if (programState->ImGuiEnabled) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    // Init Imgui

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // translation vectors for each of the igloo houses
    glm::vec3 iglooPositions[] = {
            glm::vec3(-3.0f, 0.1f, 5.5f),
            glm::vec3(-8.0f, 0.1f, 1.8f),
            glm::vec3(10.0f, 0.1f, -8.1f),
            glm::vec3(6.0f, 0.1f, -3.2f),
            glm::vec3(0.0f, 0.1f, -1.0f),
    };
    // vectors for which we will translate the position of our penguins
    glm::vec3 penguinPositions[] = {
            glm::vec3(-10.0f, 0.3f, 7.5f),
            glm::vec3(-12.0f, 0.3f, 3.0f),
            glm::vec3(14.0f, 0.3f, -6.2f),
            glm::vec3(8.0f, 0.3f, -2.2f),
            glm::vec3(2.0f, 0.3f, -1.0f),
            glm::vec3(-16.0f, 0.3f, 10.0f),
            glm::vec3(-19.0f, 0.3f, 4.8f),
            glm::vec3(10.0f, 0.3f, -6.2f),
            glm::vec3(18.0f, 0.3f, -2.2f),
            glm::vec3(4.5f, 0.3f, -1.0f),
            glm::vec3(-6.0f, 0.3f, -5.0f),
            glm::vec3(-2.8f, 0.3f, 7.8f),
            glm::vec3(5.0f, 0.3f, -5.2f),
            glm::vec3(11.0f, 0.3f, -3.2f),
            glm::vec3(-2.7f, 0.3f, -1.5f),
    };

    glm::vec3 iceBlockPositions[] = {
            glm::vec3(-10.0f, 0.2f, 8.5f),
            glm::vec3(-5.0f, 0.2f, 4.8f),
            glm::vec3(12.0f, 0.2f, -4.2f),
            glm::vec3(3.0f, 0.2f, -2.5f),
            glm::vec3(-2.0f, 0.2f, 1.0f),
    };

    glm::vec3 stonePositions[] = {
            glm::vec3(-12.0f, 0.15f, 6.5f),
            glm::vec3(-3.0f, 0.15f, -7.5f),
            glm::vec3(7.0f, 0.15f, 3.0f),
            glm::vec3(4.5f, 0.15f, 13.5f),
            glm::vec3(1.0f, 0.15f, -3.6f),
            glm::vec3(-7.0f, 0.15f, 9.5f),
    };

    // build and compile shaders
    // -------------------------
    Shader modelShader("resources/shaders/model_lighting.vs", "resources/shaders/model_lighting.fs");
    Shader octahedronShader("resources/shaders/octahedron.vs", "resources/shaders/octahedron.fs");
    Shader blendingShader("resources/shaders/blending.vs", "resources/shaders/blending.fs");
    Shader skyBoxShader("resources/shaders/sky_box.vs", "resources/shaders/sky_box.fs");
    Shader snowShader("resources/shaders/snow.vs", "resources/shaders/snow.fs");
    Shader blurShader("resources/shaders/blur.vs", "resources/shaders/blur.fs");
    Shader finalScreenShader("resources/shaders/final_screen.vs", "resources/shaders/final_screen.fs");
    // load models
    // -----------
    Model penguinModel("resources/objects/pingvin/pingvin.obj");
    penguinModel.SetShaderTextureNamePrefix("material.");
    Model iglooModel("resources/objects/igloo/scene.gltf");
    iglooModel.SetShaderTextureNamePrefix("material.");

    Model iceBlockModel("resources/objects/ice_block/scene.gltf");
    iceBlockModel.SetShaderTextureNamePrefix("material.");
    Model stoneModel("resources/objects/stone/scene.gltf");
    stoneModel.SetShaderTextureNamePrefix("material.");
    // vertices for octahedron that have only one attribute (position attribute) and since many of the vertices are repeated I used EBO.
    float vertices[] = {
            0.0f, 0.5f, 0.0f, // 0
            -0.5f, 0.0f, 0.0f, // 1
            0.0f, 0.0f, 0.5f, // 2
            0.5f, 0.0f, 0.0f,  // 3
            0.0f, 0.0f, -0.5f, // 4
            0.0f, -0.5f, 0.0f, // 5
    };
    unsigned int indices[] {
        0, 1, 2, // first triangle
        0, 2, 3, // second triangle
        0, 3, 4, // 3rd triangle
        0, 4, 1, // 4th triangle
        5, 1, 2, // 5th triangle
        5, 2, 3, // 6th triangle
        5, 3, 4, // 7th triangle
        5, 4, 1 // 8th triangle
    };

    float skyboxVertices[] = {
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f
    };
    float transparentVertices[] = {
            // positions         // texture Coords
        0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
        0.0f, -0.5f,  0.0f,  0.0f,  1.0f,
        1.0f, -0.5f,  0.0f,  1.0f,  1.0f,

        0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
        1.0f, -0.5f,  0.0f,  1.0f,  1.0f,
        1.0f,  0.5f,  0.0f,  1.0f,  0.0f
    };
    vector<glm::vec3> locationOfIcicles
    {
        glm::vec3(-10.38f, 0.28f, 8.508f),
        glm::vec3(-5.38f, 0.28f, 4.808f),
        glm::vec3(11.62f, 0.28f, -4.198f),
        glm::vec3(2.62f, 0.28f, -2.498f),
        glm::vec3(-2.38f, 0.28f, 1.008f),
    };
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    unsigned int transparentVAO, transparentVBO;
    glGenVertexArrays(1, &transparentVAO);
    glGenBuffers(1, &transparentVBO);
    glBindVertexArray(transparentVAO);
    glBindBuffer(GL_ARRAY_BUFFER, transparentVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), transparentVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);


    unsigned int skyBoxVAO, skyBoxVBO;
    glGenVertexArrays(1, &skyBoxVAO);
    glGenBuffers(1, &skyBoxVBO);

    glBindVertexArray(skyBoxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyBoxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    stbi_set_flip_vertically_on_load(false);
    unsigned int transparentTexture = loadTexture("resources/textures/Icicles.png", false);

    stbi_set_flip_vertically_on_load(true);
    unsigned int diffuseMap = loadTexture("resources/textures/snow01_diffuse_4k.jpg", true);
    unsigned int normalMap  = loadTexture("resources/textures/snow01_normal_4k.jpg", false);
    unsigned int heightMap  = loadTexture("resources/textures/snow01_height_4k.jpg", false);

    stbi_set_flip_vertically_on_load(false);

    vector<std::string> faces
    {
        "resources/textures/skybox/right.jpg",
        "resources/textures/skybox/left.jpg",
        "resources/textures/skybox/top.jpg",
        "resources/textures/skybox/bottom.jpg",
        "resources/textures/skybox/front.jpg",
        "resources/textures/skybox/back.jpg"
    };

    unsigned int cubeMap = loadCubemap(faces);
    skyBoxShader.use();
    skyBoxShader.setInt("skybox", 0);

    snowShader.use();
    snowShader.setInt("diffuseMap", 0);
    snowShader.setInt("normalMap", 1);
    snowShader.setInt("depthMap", 2);

    // enabling hdr and bloom--> first we'll need floating point framebuffer
    unsigned int hdrFBO;
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    // create 2 floating point color buffers (1 for normal rendering, other for brightness threshold values)
    unsigned int colorBuffers[2];
    glGenTextures(2, colorBuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // attach texture to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
    }
    // for depth buffer we'll use renderbuffer instead of texture
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);
    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ping-pong-framebuffer for blurring
    unsigned int pingpongFBO[2];
    unsigned int pingpongColorbuffers[2];
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorbuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
        // also check if framebuffers are complete (no need for depth buffer)
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
    }

    blurShader.use();
    blurShader.setInt("image", 0);
    finalScreenShader.use();
    finalScreenShader.setInt("hdrColorBuffer", 0);
    finalScreenShader.setInt("blurColorBuffer", 1);

    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        // render
        glClearColor(programState->clearColor.r, programState->clearColor.g, programState->clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // if we want to render scene into floating point framebuffer we will need to bind our framebuffer before rendering
        // -----------------------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        std::map<float, glm::vec3> sorted;
        for (unsigned int i = 0; i < locationOfIcicles.size(); i++)
        {
            float distance = glm::length(programState->camera.Position - locationOfIcicles[i]);
            sorted[distance] = locationOfIcicles[i];
        }

        modelShader.use();
        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = programState->camera.GetViewMatrix();
        modelShader.setMat4("projection", projection);
        modelShader.setMat4("view", view);

        modelShader.setVec3("viewPos", programState->camera.Position);
        modelShader.setFloat("material.shininess", 8.0);

        modelShader.setVec3("dirLight.direction", -4.0f, -0.5f, -1.5f);
        modelShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
        modelShader.setVec3("dirLight.diffuse", 0.01f, 0.01f, 0.01f);
        modelShader.setVec3("dirLight.specular", 0.4f, 0.4f, 0.4f);

        modelShader.setBool("blinn_phong", blinn);
        if(blinn)
            std::cout << " The scene is currently lit by Blinn-Phong's lighting model" << std::endl;
        else
            std::cout << " The scene is currently lit by Phong's lighting model" << std::endl;

        setSpotLight(modelShader);
        glm::mat4 model;
        unsigned int sign = -1;
        // drawing 5 igloo houses
        for(int i = 0; i < 5; i++) {
            modelShader.setVec3("pointLights[" + std::to_string(i+5) + "].position", iglooPositions[i] + glm::vec3(10.0f, 0.2f, 3.0f));
            modelShader.setVec3("pointLights[" + std::to_string(i+5) + "].ambient", 0.05f, 0.05f, 0.05f);
            modelShader.setVec3("pointLights[" + std::to_string(i+5) + "].diffuse", 18.0f, 18.0f, 0.0f);
            modelShader.setVec3("pointLights[" + std::to_string(i+5) + "].specular", 0.8f, 0.8f, 0.8f);
            modelShader.setFloat("pointLights[" + std::to_string(i+5) + "].constant", 1.0f);
            modelShader.setFloat("pointLights[" + std::to_string(i+5) + "].linear", 0.22f);
            modelShader.setFloat("pointLights[" + std::to_string(i+5) + "].quadratic", 0.20f);
            sign *= -1;
            model = glm::mat4(1.0f);
            model = glm::translate(model, iglooPositions[i] + glm::vec3(10.0f, 0.0f, 3.0f));
            model = glm::rotate(model, (float)glm::radians(-45.0f + sign * (3 * i + 15)), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, (float)glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::rotate(model, (float)glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            modelShader.setMat4("model", model);
            iglooModel.Draw(modelShader);
        }
        // drawing 5 more igloo houses
        for(int i = 0; i < 5; i++) {
            modelShader.setVec3("pointLights[" + std::to_string(i) + "].position", iglooPositions[i] + glm::vec3(0.0f, 0.2f, 0.0f));
            modelShader.setVec3("pointLights[" + std::to_string(i) + "].ambient", 0.05f, 0.05f, 0.05f);
            modelShader.setVec3("pointLights[" + std::to_string(i) + "].diffuse", 18.0f, 18.0f, 0.0f);
            modelShader.setVec3("pointLights[" + std::to_string(i) + "].specular", 0.8f, 0.8f, 0.8f);
            modelShader.setFloat("pointLights[" + std::to_string(i) + "].constant", 1.0f);
            modelShader.setFloat("pointLights[" + std::to_string(i) + "].linear", 0.22f);
            modelShader.setFloat("pointLights[" + std::to_string(i) + "].quadratic", 0.20f);

            model = glm::mat4(1.0f);
            model = glm::translate(model, iglooPositions[i]);
            model = glm::rotate(model, (float)glm::radians(-25.0f + 4 * i), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, (float)glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::rotate(model, (float)glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(1.2f));
            modelShader.setMat4("model", model);
            iglooModel.Draw(modelShader);
        }

        // drawing 15 pinguins
        for(int i = 0; i < 15; i++) {
            sign *= -1;
            model = glm::mat4(1.0f);
            model = glm::translate(model, penguinPositions[i]);
            if(waddle)
                model = glm::rotate(model, (float)(1.5*sin(glfwGetTime())), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, (float)glm::radians(25.0f + sign * 2 * i), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.4f));
            modelShader.setMat4("model", model);
            penguinModel.Draw(modelShader);
        }
        // drawing 6 stones
        for(int i = 0; i < 6; i++) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, stonePositions[i]);
            model = glm::rotate(model, (float)glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::rotate(model, (float)glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.0007f));
            modelShader.setMat4("model", model);
            stoneModel.Draw(modelShader);
        }
        // drawing 5 ice blocks
        for(int i = 0; i < 5; i++) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, iceBlockPositions[i]);
            model = glm::rotate(model, (float)glm::radians(225.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, (float)glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::rotate(model, (float)glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.5f));
            modelShader.setMat4("model", model);
            iceBlockModel.Draw(modelShader);
        };

        glEnable(GL_CULL_FACE);
        octahedronShader.use();
        octahedronShader.setMat4("view", view);
        octahedronShader.setMat4("projection", projection);
        setSpotLight(octahedronShader);
        for(int i = 0; i < 5; i++) {

            model = glm::mat4(1.0f);
            model = glm::translate(model, iglooPositions[i] + glm::vec3(-1.0f, 0.08f, 1.8f));
            model = glm::rotate(model, (float) glm::radians(50.0), glm::vec3(0.7, 0.8, 0.2));
            model = glm::scale(model, glm::vec3(0.2f));
            octahedronShader.setMat4("model", model);

            glBindVertexArray(VAO);

            glm::vec3 colorStart(0.529f, 0.808f, 0.922f);
            glm::vec3 colorEnd(0.2549f, 0.4118f, 0.8824f);

            glm::vec3 colorByTime= glm::mix(colorStart, colorEnd, 0.5f * (1.0f + cos(glfwGetTime())));

            octahedronShader.setVec3("myColor", colorByTime);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glDrawElements(GL_TRIANGLES, 24, GL_UNSIGNED_INT, 0);

            octahedronShader.setVec3("myColor", glm::vec3(0.0f, 0.0f, 0.0f));
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDrawElements(GL_TRIANGLES, 24, GL_UNSIGNED_INT, 0);

            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        glDisable(GL_CULL_FACE);

        blendingShader.use();
        blendingShader.setInt("texture1", 0);
        glBindVertexArray(transparentVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, transparentTexture);

        blendingShader.setMat4("view", view);
        blendingShader.setMat4("projection", projection);
        setSpotLight(blendingShader);

        for (std::map<float, glm::vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, it->second);
            model = glm::rotate(model, (float)glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f));

            model = glm::scale(model, glm::vec3(0.65f));
            blendingShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        };

        glEnable(GL_CULL_FACE);
        snowShader.use();
        setSpotLight(snowShader);
        snowShader.setBool("sl", spotlight);
        snowShader.setMat4("view", view);
        snowShader.setMat4("projection", projection);
        for(int i = 0; i < 5; i++) {
            snowShader.setVec3("pointLights[" + std::to_string(i+5) + "].position", iglooPositions[i] + glm::vec3(10.0f, 0.2f, 4.0f));
            snowShader.setVec3("pointLights[" + std::to_string(i+5) + "].ambient", 0.05f, 0.05f, 0.05f);
            snowShader.setVec3("pointLights[" + std::to_string(i+5) + "].diffuse", 0.8f, 0.8f, 0.0f);
            snowShader.setVec3("pointLights[" + std::to_string(i+5) + "].specular", 0.8f, 0.8f, 0.8f);
            snowShader.setFloat("pointLights[" + std::to_string(i+5) + "].constant", 1.0f);
            snowShader.setFloat("pointLights[" + std::to_string(i+5) + "].linear", 0.22f);
            snowShader.setFloat("pointLights[" + std::to_string(i+5) + "].quadratic", 0.20f);
        }
        for(int i = 0; i < 5; i++) {
            snowShader.setVec3("pointLights[" + std::to_string(i) + "].position", iglooPositions[i] + glm::vec3(0.0f, 0.2f, 1.0f));
            snowShader.setVec3("pointLights[" + std::to_string(i) + "].ambient", 0.05f, 0.05f, 0.05f);
            snowShader.setVec3("pointLights[" + std::to_string(i) + "].diffuse", 0.8f, 0.8f, 0.0f);
            snowShader.setVec3("pointLights[" + std::to_string(i) + "].specular", 0.8f, 0.8f, 0.8f);
            snowShader.setFloat("pointLights[" + std::to_string(i) + "].constant", 1.0f);
            snowShader.setFloat("pointLights[" + std::to_string(i) + "].linear", 0.22f);
            snowShader.setFloat("pointLights[" + std::to_string(i) + "].quadratic", 0.20f);
        }

        snowShader.setVec3("viewPos", programState->camera.Position);

        snowShader.setVec3("dirLight.direction", -4.0f, -0.5f, -1.5f);
        snowShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
        snowShader.setVec3("dirLight.diffuse", 0.01f, 0.01f, 0.01f);
        snowShader.setVec3("dirLight.specular", 0.4f, 0.4f, 0.4f);
        snowShader.setBool("blinn_phong", blinn);

        model = glm::mat4(1.0f);
        snowShader.setMat4("model", model);
        snowShader.setFloat("height_scale", heightScale);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normalMap);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, heightMap);

        renderSnowGround();
        glDisable(GL_CULL_FACE);

        glDepthFunc(GL_LEQUAL);
        skyBoxShader.use();
        skyBoxShader.setMat4("view", glm::mat4(glm::mat3(programState->camera.GetViewMatrix())));
        skyBoxShader.setMat4("projection", projection);
        glBindVertexArray(skyBoxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthFunc(GL_LESS);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // blur bright fragments with two-pass Gaussian Blur

        bool horizontal = true, first_iteration = true;
        unsigned int amount = 10;
        blurShader.use();
        for (unsigned int i = 0; i < amount; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
            blurShader.setBool("horizontal", horizontal);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);
            renderQuad();
            horizontal = !horizontal;
            if (first_iteration)
                first_iteration = false;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // now render floating point color buffer to 2D quad and transform HDR colors using tone mapping algorithm to default framebuffer's (clamped) color range
        // --------------------------------------------------------------------------------------------------------------------------
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        finalScreenShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
        finalScreenShader.setBool("bloom", bloom);
        finalScreenShader.setFloat("exposure", exposure);
        renderQuad();

        std::cout << "bloom: " << (bloom ? "on" : "off") << std::endl;
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        if (programState->ImGuiEnabled)
            DrawImGui(programState);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    programState->SaveToFile("resources/program_state.txt");
    delete programState;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &transparentVAO);
    glDeleteBuffers(1, &transparentVBO);
    glDeleteVertexArrays(1, &skyBoxVAO);
    glDeleteBuffers(1, &skyBoxVBO);

    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(RIGHT, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !blinnKeyPressed)
    {
        blinn = !blinn;
        blinnKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE)
    {
        blinnKeyPressed = false;
    }
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS && !spotlightKeyPressed)
    {
        spotlight = !spotlight;
        spotlightKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_RELEASE)
    {
        spotlightKeyPressed = false;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !bloomKeyPressed)
    {
        bloom = !bloom;
        bloomKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
    {
        bloomKeyPressed = false;
    }
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !waddleKeyPressed)
    {
        waddle = !waddle;
        waddleKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE)
    {
        waddleKeyPressed = false;
    }

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if (programState->CameraMouseMovementUpdateEnabled)
        programState->camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    programState->camera.ProcessMouseScroll(yoffset);
}

void DrawImGui(ProgramState *programState) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();


    {
        static float f = 0.0f;
        ImGui::Begin("Hello window");
        ImGui::Text("Polar night");
        ImGui::SliderFloat("Float slider", &f, 0.0, 1.0);
        ImGui::Checkbox("Blinn-Phong lighting", &blinn);
        ImGui::Checkbox("Spotlight", &spotlight);
        ImGui::Checkbox("Penguins movement", &waddle);

        ImGui::End();
    }

    {
        ImGui::Begin("Camera info");
        const Camera& c = programState->camera;
        ImGui::Text("Camera position: (%f, %f, %f)", c.Position.x, c.Position.y, c.Position.z);
        ImGui::Text("(Yaw, Pitch): (%f, %f)", c.Yaw, c.Pitch);
        ImGui::Text("Camera front: (%f, %f, %f)", c.Front.x, c.Front.y, c.Front.z);
        ImGui::Checkbox("Camera mouse update", &programState->CameraMouseMovementUpdateEnabled);
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {

    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        programState->ImGuiEnabled = !programState->ImGuiEnabled;
        if (programState->ImGuiEnabled) {
            programState->CameraMouseMovementUpdateEnabled = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }

}
unsigned int loadTexture(char const * path, bool gammaCorrection)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum internalFormat;
        GLenum dataFormat;
        if (nrComponents == 1)
        {
            internalFormat = dataFormat = GL_RED;
        }
        else if (nrComponents == 3)
        {
            internalFormat = gammaCorrection ? GL_SRGB : GL_RGB;
            dataFormat = GL_RGB;
        }
        else if (nrComponents == 4)
        {
            internalFormat = gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
            dataFormat = GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
unsigned int loadCubemap(vector<std::string> faces) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void setSpotLight(Shader& shader) {
    shader.setBool("sl", spotlight);
    shader.setVec3("spotLight.position", programState->camera.Position);
    shader.setVec3("spotLight.direction", programState->camera.Front);
    shader.setVec3("spotLight.ambient", 0.1f, 0.1f, 0.1f);
    shader.setVec3("spotLight.diffuse", 0.8f, 0.8f, 0.8f);
    shader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
    shader.setFloat("spotLight.constant", 1.0f);
    shader.setFloat("spotLight.linear", 0.22f);
    shader.setFloat("spotLight.quadratic", 0.020f);
    shader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
    shader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(18.0f)));
}

unsigned int snowVAO = 0;
unsigned int snowVBO;
void renderSnowGround(){

    if (snowVAO == 0){
        // positions
        glm::vec3 pos1(60.0f,  0.08f, 60.0f);
        glm::vec3 pos2(60.0f, 0.08f, -60.0f);
        glm::vec3 pos3( -60.0f, 0.08f, -60.0f);
        glm::vec3 pos4( -60.0f,  0.08f, 60.0f);
        // texture coordinates
        glm::vec2 uv1(0.0f, 1.0f);
        glm::vec2 uv2(0.0f, 0.0f);
        glm::vec2 uv3(1.0f, 0.0f);
        glm::vec2 uv4(1.0f, 1.0f);
        // normal vector
        glm::vec3 nm(0.0f, 0.0f, 1.0f);

        // calculate tangent/bitangent vectors of both triangles
        glm::vec3 tangent1, bitangent1;
        glm::vec3 tangent2, bitangent2;
        // triangle 1
        // ----------
        glm::vec3 edge1 = pos2 - pos1;
        glm::vec3 edge2 = pos3 - pos1;
        glm::vec2 deltaUV1 = uv2 - uv1;
        glm::vec2 deltaUV2 = uv3 - uv1;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent1 = glm::normalize(tangent1);

        bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        bitangent1 = glm::normalize(bitangent1);

        // triangle 2
        // ----------
        edge1 = pos3 - pos1;
        edge2 = pos4 - pos1;
        deltaUV1 = uv3 - uv1;
        deltaUV2 = uv4 - uv1;

        f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent2 = glm::normalize(tangent2);


        bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        bitangent2 = glm::normalize(bitangent2);


        float groundVertices[] = {
                // positions            // normal         // texcoords  // tangent                          // bitangent
                pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
                pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
                pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

                pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
                pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
                pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
        };
        // configure plane VAO
        glGenVertexArrays(1, &snowVAO);
        glGenBuffers(1, &snowVBO);
        glBindVertexArray(snowVAO);
        glBindBuffer(GL_ARRAY_BUFFER, snowVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), &groundVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
    }
    glBindVertexArray(snowVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
                // positions        // texture Coords
                -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}
