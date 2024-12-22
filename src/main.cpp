#include "application.h"

// const auto file_path = std::filesystem::path("E:\\Data\\Code\\masterarbeit-teufelsknoten-refactor\\res\\puzzles\\Puzzle6.txt");
const auto file_path = std::filesystem::path("res/puzzles/Puzzle6.txt");

int main(int argc, char** argv) {
    
    auto app = new Application();

    app->init_wizard(file_path);
    app->run();
    
    delete app;

    return 0;
}