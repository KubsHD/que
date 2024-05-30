#include "pch.h"

#include "level.h"
#include "asset.h"

Level core::load_level(String path)
{
    Level lvl;

    auto file = nlohmann::json::parse(path);
    
    // parse json
    lvl.name = file["name"];

    for (auto& obj : file["objects"])
    {
        //Asset::load_model((String)obj["model"]);
    }

    return lvl;
}
