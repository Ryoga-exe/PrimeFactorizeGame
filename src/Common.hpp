#pragma once
#include <Siv3D.hpp> // OpenSiv3D v0.4.3

enum class SceneState {
    Title,
    Game,

};

struct GameData {
    int32 highScore = 0;
};

using MyApp = SceneManager<SceneState, GameData>;