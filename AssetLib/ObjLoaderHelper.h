#pragma once
#include <memory>
#include <GameScene.h>


namespace assetlib
{
    std::unique_ptr<gameplay::GamesScene> LoadObj(const std::string& p_file_name);
}