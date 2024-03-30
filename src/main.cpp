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
unsigned int loadTexture(const char *path);
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

struct ProgramState {
    glm::vec3 clearColor = glm::vec3(0.1);
    bool ImGuiEnabled = false;
    Camera camera;
    bool CameraMouseMovementUpdateEnabled = true;
    ProgramState()
            : camera(glm::vec3(8.0f, 2.0f, 4.0f)) {}

    void SaveToFile(std::string filename);

    void LoadFromFile(std::string filename);
};

void ProgramState::SaveToFile(std::string filename) {
    std::ofstream out(filename);
    out << clearColor.r << '\n'
        << clearColor.g << '\n'
        << clearColor.b << '\n'
        << ImGuiEnabled << '\n';
        /*<< camera.Position.x << '\n'
        << camera.Position.y << '\n'
        << camera.Position.z << '\n'
        << camera.Front.x << '\n'
        << camera.Front.y << '\n'
        << camera.Front.z
        << '\n'; */
}

void ProgramState::LoadFromFile(std::string filename) {
    std::ifstream in(filename);
    if (in) {
        in >> clearColor.r
           >> clearColor.g
           >> clearColor.b
           >> ImGuiEnabled;
           /*
           >> camera.Position.x
           >> camera.Position.y
           >> camera.Position.z
           >> camera.Front.x
           >> camera.Front.y
           >> camera.Front.z;
            */
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
    /*
    if (programState->ImGuiEnabled) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
     */
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // da mogu da isprobam lepo
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


    // translation vectors for each of igloo house
    glm::vec3 iglooPositions[] = {
            glm::vec3(-3.0f, 0.2f, 5.5f),
            glm::vec3(-8.0f, 0.2f, 1.8f),
            glm::vec3(10.0f, 0.2f, -8.1f),
            glm::vec3(6.0f, 0.2f, -3.2f),
            glm::vec3(0.0f, 0.2f, -1.0f),
    };
    // vectors for which we will translate the position of our penguins
    glm::vec3 pinguinPositions[] = {
            glm::vec3(-10.0f, 0.2f, 7.5f),
            glm::vec3(-12.0f, 0.2f, 3.0f),
            glm::vec3(14.0f, 0.2f, -6.2f),
            glm::vec3(8.0f, 0.2f, -2.2f),
            glm::vec3(2.0f, 0.2f, -1.0f),
            glm::vec3(-16.0f, 0.2f, 10.0f),
            glm::vec3(-19.0f, 0.2f, 4.8f),
            glm::vec3(10.0f, 0.2f, -6.2f),
            glm::vec3(18.0f, 0.2f, -2.2f),
            glm::vec3(4.5f, 0.2f, -1.0f),
            glm::vec3(-6.0f, 0.2f, -5.0f),
            glm::vec3(-2.8f, 0.2f, 7.8f),
            glm::vec3(5.0f, 0.2f, -5.2f),
            glm::vec3(11.0f, 0.2f, -3.2f),
            glm::vec3(-2.7f, 0.2f, -1.5f),
    };

    glm::vec3 iceBlockPositions[] = {
            glm::vec3(-10.0f, 0.2f, 8.5f),
            glm::vec3(-5.0f, 0.2f, 4.8f),
            glm::vec3(12.0f, 0.2f, -4.2f),
            glm::vec3(3.0f, 0.2f, -2.5f),
            glm::vec3(-2.0f, 0.2f, 1.0f),
    };
    // build and compile shaders
    // -------------------------
    Shader modelShader("resources/shaders/model_lighting.vs", "resources/shaders/model_lighting.fs");

    // load models
    // -----------
    Model iglooModel("resources/objects/igloo/scene.gltf");
    iglooModel.SetShaderTextureNamePrefix("material.");
    Model pinguinModel("resources/objects/pingvin/pingvin.obj");
    pinguinModel.SetShaderTextureNamePrefix("material.");
    Model igloo1Model("resources/objects/igloo1/scene.gltf");
    igloo1Model.SetShaderTextureNamePrefix("material.");

    Model iceBlockModel("resources/objects/ice_block/scene.gltf");
    iceBlockModel.SetShaderTextureNamePrefix("material.");

    float vertices[] = {
            0.0f, 0.5f, 0.0f,
            -0.5f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.5f,

            0.0f, 0.5f, 0.0f,
            0.0f, 0.0f, 0.5f,
            0.5f, 0.0f, 0.0f,

            0.0f, 0.5f, 0.0f,
            0.5f, 0.0f, 0.0f,
            0.0f, 0.0f, -0.5f,

            0.0f, 0.5f, 0.0f,
            0.0f, 0.0f, -0.5f,
            -0.5f, 0.0f, 0.0f,

            0.0f, -0.5f, 0.0f,
            -0.5f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.5f,

            0.0f, -0.5f, 0.0f,
            0.0f, 0.0f, 0.5f,
            0.5f, 0.0f, 0.0f,

            0.0f, -0.5f, 0.0f,
            0.5f, 0.0f, 0.0f,
            0.0f, 0.0f, -0.5f,

            0.0f, -0.5f, 0.0f,
            0.0f, 0.0f, -0.5f,
            -0.5f, 0.0f, 0.0f

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
        glm::vec3(-10.4f, 0.28f, 8.51f),
        glm::vec3(-5.4f, 0.28f, 4.81f),
        glm::vec3(11.6f, 0.28f, -4.19f),
        glm::vec3(2.6f, 0.28f, -2.49f),
        glm::vec3(-2.4f, 0.28f, 1.01f),


    };
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

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
    stbi_set_flip_vertically_on_load(false);
    unsigned int transparentTexture = loadTexture("resources/textures/Icicles.png");


    Shader octahedronShader("resources/shaders/octahedron.vs", "resources/shaders/octahedron.fs");
    Shader blendingShader("resources/shaders/blending.vs", "resources/shaders/blending.fs");

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        std::map<float, glm::vec3> sorted;
        for (unsigned int i = 0; i < locationOfIcicles.size(); i++)
        {
            float distance = glm::length(programState->camera.Position - locationOfIcicles[i]);
            sorted[distance] = locationOfIcicles[i];
        }
        // render
        // ------
        glClearColor(programState->clearColor.r, programState->clearColor.g, programState->clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // don't forget to enable shader before setting uniforms
        modelShader.use();
        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = programState->camera.GetViewMatrix();
        modelShader.setMat4("projection", projection);
        modelShader.setMat4("view", view);

        modelShader.setVec3("viewPos", programState->camera.Position);
        modelShader.setFloat("material.shininess", 32.0);

        modelShader.setVec3("dirLight.direction", -4.0f, -0.5f, -1.5f);
        modelShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
        modelShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
        modelShader.setVec3("dirLight.specular", 0.4f, 0.4f, 0.4f);

        glm::mat4 model;
        unsigned int sign = -1;
        // drawing 5 igloo houses
        for(int i = 0; i < 5; i++) {
            modelShader.setVec3("pointLights[" + std::to_string(i+5) + "].position", iglooPositions[i] + glm::vec3(10.0f, 0.2f, 3.0f));
            modelShader.setVec3("pointLights[" + std::to_string(i+5) + "].ambient", 0.05f, 0.05f, 0.05f);
            modelShader.setVec3("pointLights[" + std::to_string(i+5) + "].diffuse", 1.0f, 1.0f, 0.0f);
            modelShader.setVec3("pointLights[" + std::to_string(i+5) + "].specular", 0.8f, 0.8f, 0.8f);
            modelShader.setFloat("pointLights[" + std::to_string(i+5) + "].constant", 1.0f);
            modelShader.setFloat("pointLights[" + std::to_string(i+5) + "].linear", 0.22f);
            modelShader.setFloat("pointLights[" + std::to_string(i+5) + "].quadratic", 0.20f);
            sign *= -1;
            model = glm::mat4(1.0f);
            model = glm::translate(model, iglooPositions[i] + glm::vec3(10.0f, 0.0f, 3.0f));
            model = glm::rotate(model, (float)glm::radians(-45.0f + sign * (3 * i + 15)), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(1.2f));
            modelShader.setMat4("model", model);
            iglooModel.Draw(modelShader);
        }
        // drawing 5 more igloo houses with a different igloo model
        for(int i = 0; i < 5; i++) {
            modelShader.setVec3("pointLights[" + std::to_string(i) + "].position", iglooPositions[i] + glm::vec3(0.0f, 0.2f, 0.0f));
            modelShader.setVec3("pointLights[" + std::to_string(i) + "].ambient", 0.05f, 0.05f, 0.05f);
            modelShader.setVec3("pointLights[" + std::to_string(i) + "].diffuse", 0.8f, 0.8f, 0.0f);
            modelShader.setVec3("pointLights[" + std::to_string(i) + "].specular", 0.8f, 0.8f, 0.8f);
            modelShader.setFloat("pointLights[" + std::to_string(i) + "].constant", 1.0f);
            modelShader.setFloat("pointLights[" + std::to_string(i) + "].linear", 0.14f);
            modelShader.setFloat("pointLights[" + std::to_string(i) + "].quadratic", 0.07f);

            model = glm::mat4(1.0f);
            model = glm::translate(model, iglooPositions[i]);
            model = glm::rotate(model, (float)glm::radians(-25.0f + 4 * i), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, (float)glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::rotate(model, (float)glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(1.2f));
            modelShader.setMat4("model", model);
            igloo1Model.Draw(modelShader);
        }

        // drawing 15 pinguins
        for(int i = 0; i < 15; i++) {
            sign *= -1;
            model = glm::mat4(1.0f);
            model = glm::translate(model, pinguinPositions[i]);
            model = glm::rotate(model, (float)glm::radians(25.0f + sign * 2 * i), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.4f));
            modelShader.setMat4("model", model);
            pinguinModel.Draw(modelShader);
        }
        //if (programState->ImGuiEnabled)
            //DrawImGui(programState);

        // drawing 5 ice blocks
        for(int i = 0; i < 5; i++) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, iceBlockPositions[i]);
            model = glm::rotate(model, (float)glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, (float)glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::rotate(model, (float)glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.5f));
            modelShader.setMat4("model", model);
            iceBlockModel.Draw(modelShader);
        };


        octahedronShader.use();
        octahedronShader.setMat4("view", view);
        octahedronShader.setMat4("projection", projection);
        for(int i = 0; i < 5; i++) {

            model = glm::mat4(1.0f);
            model = glm::translate(model, iglooPositions[i] + glm::vec3(-1.0f, 0.0f, 1.8f));
            model = glm::rotate(model, (float) glm::radians(40.0), glm::vec3(0.7, 0.8, 0.2));
            model = glm::scale(model, glm::vec3(0.2f));
            octahedronShader.setMat4("model", model);

            glBindVertexArray(VAO);

            glm::vec3 colorStart(0.529f, 0.808f, 0.922f);
            glm::vec3 colorEnd(0.2549f, 0.4118f, 0.8824f);

            glm::vec3 colorByTime= glm::mix(colorStart, colorEnd, 0.5f * (1.0f + cos(glfwGetTime())));

            octahedronShader.setVec3("myColor", colorByTime);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glDrawArrays(GL_TRIANGLES, 0, 24);

            octahedronShader.setVec3("myColor", glm::vec3(0.0f, 0.0f, 0.0f));
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDrawArrays(GL_TRIANGLES, 0, 24);

            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        blendingShader.use();
        blendingShader.setInt("texture1", 0);
        glBindVertexArray(transparentVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, transparentTexture);

        blendingShader.setMat4("view", view);
        blendingShader.setMat4("projection", projection);


        for (unsigned int i = 0; i < locationOfIcicles.size(); i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, locationOfIcicles[i]);
            model = glm::rotate(model, (float)glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f));

            model = glm::scale(model, glm::vec3(0.65f));
            blendingShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        };


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
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
        ImGui::Text("Hello text");
        ImGui::SliderFloat("Float slider", &f, 0.0, 1.0);
        ImGui::ColorEdit3("Background color", (float *) &programState->clearColor);

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
unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
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
