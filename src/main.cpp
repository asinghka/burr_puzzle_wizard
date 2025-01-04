#include "application.h"

const auto file_path = std::filesystem::path("res/puzzles/Puzzle18.txt");

int main(int argc, char** argv) {
    
    auto app = new Application();

    app->init_solver(file_path);
    app->run();
    
    delete app;

    return 0;
}