#include "gui.h"

#include "app.h"

bool GUI::menuEnable = false;

void GUI::init(SDL_Window* window, SDL_Renderer* renderer)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavNoCaptureKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);
}

void GUI::free()
{
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void GUI::processEvent(SDL_Event& event)
{
    ImGui_ImplSDL2_ProcessEvent(&event);
}

void GUI::drawMenu(App& app)
{
    if(ImGui::BeginMainMenuBar())
    {
        if(ImGui::BeginMenu("System"))
        {
            ImGui::SeparatorText("State");

            ImGui::Checkbox("Skip Boot ROM", &GameBoy::skipBootROM);

            if(ImGui::MenuItem("Reboot"))
                app.gameboy.reboot();

            ImGui::SeparatorText("Configuration");

            ImGui::DragFloat("Refresh Rate Period", &App::refreshRatePeriod, 1, 0.1, 100);

            if(ImGui::MenuItem("Reset Refresh Rate Period"))
            {
                App::refreshRatePeriod = 16.67;
            }

            // ImGui::SeparatorText("Hardware");

            // if(ImGui::MenuItem("Memory Viewer"))
            // {
                
            // }

            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Game"))
        {
            ImGui::Text("%s", app.gameboy.getTitle().c_str());
            ImGui::Spacing();

            // ImGui::SeparatorText("Saving");

            // if(ImGui::MenuItem("Save"))
            // {
                
            // }

            // if(ImGui::MenuItem("Load"))
            // {

            // }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void GUI::draw(SDL_Renderer* renderer, App& app)
{
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    // ImGui::ShowDemoWindow();

    if(GUI::menuEnable)
        GUI::drawMenu(app);

    ImGui::Render();

    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
}