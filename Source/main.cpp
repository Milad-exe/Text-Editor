#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <tinyfiledialogs.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>

class TextEditor {
private:
    char* textBuffer;
    size_t bufferSize;
    std::string currentFilePath;
    bool hasUnsavedChanges;
    float fontSize;
    bool showMenu;

    std::string clipboardText;

    int currentLine;
    int currentColumn;
    int wordCount;
    int charCount;

public:
    TextEditor() : bufferSize(1024 * 1024), hasUnsavedChanges(false), fontSize(20.0f), showMenu(false),
        currentLine(1), currentColumn(1), wordCount(0), charCount(0) {
        textBuffer = new char[bufferSize];
        textBuffer[0] = '\0';
    }

    ~TextEditor() {
        delete[] textBuffer;
    }

    void NewFile() {
        if (hasUnsavedChanges && ConfirmSave()) SaveFile();
        textBuffer[0] = '\0';
        currentFilePath.clear();
        hasUnsavedChanges = false;
        UpdateStats();
    }

    bool ConfirmSave() {
        int result = tinyfd_messageBox("Unsaved Changes", "Save before continuing?", "yesnocancel", "question", 1);
        return result == 1;
    }

    void OpenFile() {
        if (hasUnsavedChanges && ConfirmSave()) SaveFile();

        const char* filter[1] = { "*.txt" };
        const char* path = tinyfd_openFileDialog("Open File", "", 1, filter, "Text Files", 0);
        if (path) {
            std::ifstream file(path);
            if (file) {
                std::stringstream buf;
                buf << file.rdbuf();
                std::string content = buf.str();
                if (content.length() < bufferSize - 1) {
                    strcpy(textBuffer, content.c_str());
                    currentFilePath = path;
                    hasUnsavedChanges = false;
                    UpdateStats();
                }
                else {
                    tinyfd_messageBox("Error", "File too large!", "ok", "error", 1);
                }
            }
        }
    }

    void SaveFile() {
        if (currentFilePath.empty()) SaveAsFile();
        else {
            std::ofstream file(currentFilePath);
            if (file) {
                file << textBuffer;
                hasUnsavedChanges = false;
            }
        }
    }

    void SaveAsFile() {
        const char* filter[1] = { "*.txt" };
        const char* path = tinyfd_saveFileDialog("Save File As", "untitled.txt", 1, filter, "Text Files");
        if (path) {
            std::ofstream file(path);
            if (file) {
                file << textBuffer;
                currentFilePath = path;
                hasUnsavedChanges = false;
            }
        }
    }

    void CopyText() {
        clipboardText = std::string(textBuffer);
        glfwSetClipboardString(nullptr, clipboardText.c_str());
    }

    void CutText() {
        CopyText();
        textBuffer[0] = '\0';
        hasUnsavedChanges = true;
        UpdateStats();
    }

    void PasteText() {
        const char* clip = glfwGetClipboardString(nullptr);
        if (clip && strlen(textBuffer) + strlen(clip) < bufferSize - 1) {
            strcat(textBuffer, clip);
            hasUnsavedChanges = true;
            UpdateStats();
        }
    }

    void ZoomIn() { fontSize = std::min(fontSize + 2.0f, 48.0f); }
    void ZoomOut() { fontSize = std::max(fontSize - 2.0f, 8.0f); }

    void UpdateStats() {
        charCount = strlen(textBuffer);
        wordCount = 0; bool inWord = false;
        for (int i = 0; i < charCount; i++) {
            if (isspace(textBuffer[i])) inWord = false;
            else if (!inWord) { inWord = true; wordCount++; }
        }
    }

    void UpdateCursorPosition() {
        currentLine = 1; currentColumn = 1;
        for (int i = 0; i < charCount; i++) {
            if (textBuffer[i] == '\n') { currentLine++; currentColumn = 1; }
            else currentColumn++;
        }
    }

    void Render(ImFont* font) {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::PushFont(font);

        // Custom title bar
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, 50));
        ImGui::Begin("TitleBar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground);
        ImGui::SetCursorPosY(6);
        ImGui::Text("  Text Editor");
        ImGui::SameLine(io.DisplaySize.x - 130);
        if (ImGui::Button("_")) glfwIconifyWindow(glfwGetCurrentContext());
        ImGui::SameLine();
        if (ImGui::Button("[ ]")) {
        }
        ImGui::SameLine();
        if (ImGui::Button("X")) glfwSetWindowShouldClose(glfwGetCurrentContext(), GLFW_TRUE);
        ImGui::End();

        // Menu Button properly aligned
        ImGui::SetNextWindowPos(ImVec2(0, 30));
        ImGui::SetNextWindowSize(ImVec2(50, 50));
        ImGui::Begin("MenuButton", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground);
        ImGui::SetCursorPos(ImVec2(10, 5));
        if (ImGui::Button("\xef\x84\xa3##menu")) showMenu = !showMenu;
        ImGui::End();

        if (showMenu) {
            ImGui::SetNextWindowPos(ImVec2(10, 65));
            ImGui::Begin("Menu", &showMenu, ImGuiWindowFlags_NoDecoration);
            if (ImGui::MenuItem("New")) {
                NewFile();
                showMenu = false;
            }
            if (ImGui::MenuItem("Open")) {
                OpenFile();
                showMenu = false;
            }
            if (ImGui::MenuItem("Save")) {
                SaveFile();
                showMenu = false;
            }
            if (ImGui::MenuItem("Save As")) {
                SaveAsFile();
                showMenu = false;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Copy")) {
                CopyText();
                showMenu = false;
            }
            if (ImGui::MenuItem("Cut")) {
                CutText();
                showMenu = false;
            }
            if (ImGui::MenuItem("Paste")) {
                PasteText();
                showMenu = false;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Zoom In")) {
                ZoomIn();
            }
            if (ImGui::MenuItem("Zoom Out")) {
                ZoomOut();
            }

            ImGui::End();
        }

        ImGui::SetNextWindowPos(ImVec2(10, 100));
        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x - 20, io.DisplaySize.y - 160));
        ImGui::Begin("Editor", nullptr, ImGuiWindowFlags_NoTitleBar);

        ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_CallbackEdit;
        if (ImGui::InputTextMultiline("##text", textBuffer, bufferSize,
            ImVec2(io.DisplaySize.x - 40, io.DisplaySize.y - 200), flags,
            [](ImGuiInputTextCallbackData* data) -> int {
                TextEditor* ed = (TextEditor*)data->UserData;
                ed->hasUnsavedChanges = true;
                ed->UpdateStats();
                return 0;
            }, this)) {
            showMenu = false;
            hasUnsavedChanges = true;
            UpdateStats();
        }
        UpdateCursorPosition();
        ImGui::End();

        ImGui::SetNextWindowPos(ImVec2(0, io.DisplaySize.y - 50));
        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, 50));
        ImGui::Begin("Status", nullptr, ImGuiWindowFlags_NoDecoration);
        std::string status = (currentFilePath.empty() ? "Untitled" : currentFilePath);
        if (hasUnsavedChanges) status += " *";
        ImGui::Text("%s | Ln %d, Col %d | Words: %d | Chars: %d | Font: %.0fpx",
            status.c_str(), currentLine, currentColumn, wordCount, charCount, fontSize);
        ImGui::End();

        ImGui::PopFont();
    }
};

int main() {
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(1200, 800, "Text Editor", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsLight();

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 12.0f;
    style.FrameRounding = 6.0f;
    style.ScrollbarRounding = 12.0f;
    style.GrabRounding = 6.0f;
    style.FramePadding = ImVec2(10, 6);
    style.ItemSpacing = ImVec2(10, 10);
    style.WindowPadding = ImVec2(20, 20);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.95f, 0.95f, 0.96f, 1.0f);
    style.Colors[ImGuiCol_Text] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 120");

    ImFont* font = io.Fonts->AddFontFromFileTTF("../resources/Roboto-VariableFont_wdth,wght.ttf", 20.0f);
    if (!font) font = io.Fonts->AddFontDefault();

    TextEditor editor;
    editor.UpdateStats();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        editor.Render(font);

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.95f, 0.95f, 0.96f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
