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

    // Clipboard functionality
    std::string clipboardText;

    // Status tracking
    int currentLine;
    int currentColumn;
    int wordCount;
    int charCount;

public:
    TextEditor() : bufferSize(1024 * 1024), hasUnsavedChanges(false), fontSize(30.0f), showMenu(false),
        currentLine(1), currentColumn(1), wordCount(0), charCount(0) {
        textBuffer = new char[bufferSize];
        textBuffer[0] = '\0';
    }

    ~TextEditor() {
        delete[] textBuffer;
    }

    void NewFile() {
        if (hasUnsavedChanges) {
            int result = tinyfd_messageBox(
                "Unsaved Changes",
                "You have unsaved changes. Do you want to save before creating a new file?",
                "yesnocancel",
                "question",
                1
            );

            if (result == 1) { // Yes
                SaveFile();
            }
            else if (result == 0) { // Cancel
                return;
            }
        }

        textBuffer[0] = '\0';
        currentFilePath.clear();
        hasUnsavedChanges = false;
        UpdateStats();
    }

    void OpenFile() {
        if (hasUnsavedChanges) {
            int result = tinyfd_messageBox(
                "Unsaved Changes",
                "You have unsaved changes. Do you want to save before opening a new file?",
                "yesnocancel",
                "question",
                1
            );

            if (result == 1) { // Yes
                SaveFile();
            }
            else if (result == 0) { // Cancel
                return;
            }
        }

        const char* filterPatterns[1] = { "*.txt" };
        char const* filePath = tinyfd_openFileDialog(
            "Open File",
            "",
            1,
            filterPatterns,
            "Text Files",
            0
        );

        if (filePath) {
            std::ifstream file(filePath);
            if (file.is_open()) {
                std::stringstream buffer;
                buffer << file.rdbuf();
                std::string content = buffer.str();

                if (content.length() < bufferSize - 1) {
                    strcpy(textBuffer, content.c_str());
                    currentFilePath = filePath;
                    hasUnsavedChanges = false;
                    UpdateStats();
                }
                else {
                    tinyfd_messageBox("Error", "File is too large to open!", "ok", "error", 1);
                }
                file.close();
            }
            else {
                tinyfd_messageBox("Error", "Could not open file!", "ok", "error", 1);
            }
        }
    }

    void SaveFile() {
        if (currentFilePath.empty()) {
            SaveAsFile();
        }
        else {
            std::ofstream file(currentFilePath);
            if (file.is_open()) {
                file << textBuffer;
                file.close();
                hasUnsavedChanges = false;
            }
            else {
                tinyfd_messageBox("Error", "Could not save file!", "ok", "error", 1);
            }
        }
    }

    void SaveAsFile() {
        const char* filterPatterns[1] = { "*.txt" };
        char const* filePath = tinyfd_saveFileDialog(
            "Save File As",
            "untitled.txt",
            1,
            filterPatterns,
            "Text Files"
        );

        if (filePath) {
            std::ofstream file(filePath);
            if (file.is_open()) {
                file << textBuffer;
                file.close();
                currentFilePath = filePath;
                hasUnsavedChanges = false;
            }
            else {
                tinyfd_messageBox("Error", "Could not save file!", "ok", "error", 1);
            }
        }
    }

    void CopyText() {
        // In a real implementation, you'd get the selected text
        // For simplicity, we'll copy all text
        clipboardText = std::string(textBuffer);
        glfwSetClipboardString(nullptr, clipboardText.c_str());
    }

    void CutText() {
        // In a real implementation, you'd cut only selected text
        // For simplicity, we'll cut all text
        clipboardText = std::string(textBuffer);
        glfwSetClipboardString(nullptr, clipboardText.c_str());
        textBuffer[0] = '\0';
        hasUnsavedChanges = true;
        UpdateStats();
    }

    void PasteText() {
        const char* clipboard = glfwGetClipboardString(nullptr);
        if (clipboard) {
            size_t currentLen = strlen(textBuffer);
            size_t clipLen = strlen(clipboard);

            if (currentLen + clipLen < bufferSize - 1) {
                strcat(textBuffer, clipboard);
                hasUnsavedChanges = true;
                UpdateStats();
            }
        }
    }

    void ZoomIn() {
        fontSize = std::min(fontSize + 2.0f, 72.0f);
    }

    void ZoomOut() {
        fontSize = std::max(fontSize - 2.0f, 8.0f);
    }

    void UpdateStats() {
        // Count characters (excluding null terminator)
        charCount = strlen(textBuffer);

        // Count words
        wordCount = 0;
        bool inWord = false;
        for (int i = 0; i < charCount; i++) {
            if (isspace(textBuffer[i]) || textBuffer[i] == '\n' || textBuffer[i] == '\t') {
                inWord = false;
            }
            else if (!inWord) {
                inWord = true;
                wordCount++;
            }
        }
    }

    void UpdateCursorPosition() {
        currentLine = 1;
        currentColumn = 1;

        // Count lines and get column position at end of text
        for (int i = 0; i < charCount; i++) {
            if (textBuffer[i] == '\n') {
                currentLine++;
                currentColumn = 1;
            }
            else {
                currentColumn++;
            }
        }
    }

    void Render() {
        // Get window size
        ImGuiIO& io = ImGui::GetIO();
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        // Menu button at top-left
        ImGui::SetNextWindowPos(ImVec2(1, 1));
        ImGui::SetNextWindowSizeConstraints(ImVec2(25, 25), ImVec2(25, 25));
        ImGui::Begin("MenuButton", nullptr,
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoNav);

        if (ImGui::Button("Menu", ImVec2(25, 25))) {
            showMenu = !showMenu;
        }
        ImGui::End();

        // Dropdown menu
        if (showMenu) {
            ImGui::SetNextWindowPos(ImVec2(10, 55));
            ImGui::Begin("DropdownMenu", &showMenu,
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_AlwaysAutoResize);

            // File menu
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New", "Ctrl+N")) {
                    NewFile();
                    showMenu = !showMenu;
                }
                if (ImGui::MenuItem("Open", "Ctrl+O")) {
                    OpenFile();
                    showMenu = !showMenu;
                }
                if (ImGui::MenuItem("Save", "Ctrl+S")) {
                    SaveFile();
                    showMenu = !showMenu;
                }
                if (ImGui::MenuItem("Save As", "Ctrl+Shift+S")) {
                    SaveAsFile();
                    showMenu = !showMenu;
                }
                ImGui::EndMenu();
            }

            // Edit menu
            if (ImGui::BeginMenu("Edit")) {
                if (ImGui::MenuItem("Copy", "Ctrl+C")) {
                    CopyText();
                    showMenu = !showMenu;
                }
                if (ImGui::MenuItem("Cut", "Ctrl+X")) {
                    CutText();
                    showMenu = !showMenu;
                }
                if (ImGui::MenuItem("Paste", "Ctrl+V")) {
                    PasteText();
                    showMenu = !showMenu;
                }
                ImGui::EndMenu();
            }

            // View menu
            if (ImGui::BeginMenu("View")) {
                if (ImGui::MenuItem("Zoom In", "Ctrl++")) {
                    ZoomIn();
                }
                if (ImGui::MenuItem("Zoom Out", "Ctrl+-")) {
                    ZoomOut();
                }
                ImGui::EndMenu();
            }

            // Settings menu (placeholder)
            if (ImGui::BeginMenu("Settings")) {
                ImGui::Text("Settings will be implemented later");
                ImGui::EndMenu();
                showMenu = !showMenu;
            }

            ImGui::End();
        }

        // Main text editor area
        ImGui::SetNextWindowPos(ImVec2(0, 80));
        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y - 130)); // Leave space for status bar

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
        ImGui::Begin("TextEditor", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse);

        // Set font size
        ImGui::SetWindowFontScale(fontSize / 16.0f);

        // Text input area
        ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput |
            ImGuiInputTextFlags_CallbackEdit;

        if (ImGui::InputTextMultiline("##TextEditor", textBuffer, bufferSize,
            ImVec2(-1, -1), flags, [](ImGuiInputTextCallbackData* data) -> int {
                TextEditor* editor = (TextEditor*)data->UserData;
                editor->hasUnsavedChanges = true;
                editor->UpdateStats();
                return 0;
            }, this)) {
            hasUnsavedChanges = true;
            UpdateStats();
        }

        // Update cursor position
        UpdateCursorPosition();

        ImGui::PopStyleVar();
        ImGui::End();

        // Status bar
        ImGui::SetNextWindowPos(ImVec2(0, io.DisplaySize.y - 50));
        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, 50));
        ImGui::Begin("StatusBar", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar);

        // Top row - File status and font size
        std::string status = currentFilePath.empty() ? "Untitled" : currentFilePath;
        if (hasUnsavedChanges) {
            status += " *";
        }
        status += " | Font Size: " + std::to_string((int)fontSize);
        ImGui::Text("%s", status.c_str());

        ImGui::Separator();

        // Bottom row - Three sections
        float windowWidth = ImGui::GetWindowWidth();
        float sectionWidth = windowWidth / 3.0f;

        // Section 1: Line and Column
        ImGui::Text("Ln: %d, Col: %d", currentLine, currentColumn);

        // Section 2: Word Count
        ImGui::SameLine();
        ImGui::SetCursorPosX(sectionWidth);
        ImGui::Text("Words: %d", wordCount);

        // Section 3: Character Count
        ImGui::SameLine();
        ImGui::SetCursorPosX(sectionWidth * 2);
        ImGui::Text("Characters: %d", charCount);

        ImGui::End();
        ImGui::PopStyleVar(); // restore padding
        // Handle keyboard shortcuts
        if (io.KeyCtrl) {
            if (ImGui::IsKeyPressed(ImGuiKey_N)) NewFile();
            if (ImGui::IsKeyPressed(ImGuiKey_O)) OpenFile();
            if (ImGui::IsKeyPressed(ImGuiKey_S)) {
                if (io.KeyShift) SaveAsFile();
                else SaveFile();
            }
            if (ImGui::IsKeyPressed(ImGuiKey_C)) CopyText();
            if (ImGui::IsKeyPressed(ImGuiKey_X)) CutText();
            if (ImGui::IsKeyPressed(ImGuiKey_V)) PasteText();
            if (ImGui::IsKeyPressed(ImGuiKey_Equal)) ZoomIn();
            if (ImGui::IsKeyPressed(ImGuiKey_Minus)) ZoomOut();
        }

        // Close menu when clicking elsewhere
        if (showMenu && ImGui::IsMouseClicked(0)) {
            ImVec2 mousePos = ImGui::GetMousePos();
            ImVec2 menuPos = ImVec2(10, 55);
            if (mousePos.x < menuPos.x || mousePos.x > menuPos.x + 200 ||
                mousePos.y < menuPos.y || mousePos.y > menuPos.y + 150) {
                showMenu = false;
            }
        }
    }

};

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Create window
    GLFWwindow* window = glfwCreateWindow(1200, 800, "Simple Text Editor", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 120");

    // Create text editor instance
    TextEditor editor;

    // Initial stats update
    editor.UpdateStats();

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Render text editor
        editor.Render();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}