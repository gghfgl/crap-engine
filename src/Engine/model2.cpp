#include "model2.h"

Model::Model(const std::string &path)
{
    Log::separator();
    Log::info("loading model: '%s'\n", path.c_str());

    // TODO: wrap in internal? is it really relative path?
    // convert to relative path.
    auto p = std::filesystem::proximate(path);
    std::string p_string{p.u8string()};

    this->directory = p_string.substr(0, p_string.find_last_of('/'));
    this->filename = p_string.substr(this->directory.length() + 1, p_string.length());

    std::string error = ASSET_load_meshes(this->directory, this->filename, this->meshes);
    if (error != "")
    {
        this->directory = "";
        this->filename = "";

        Log::error("asset_load_model: '%s'\n", error.c_str());
        Log::separator();
        return;
    }


    Log::info("done!\n");
    Log::separator();
}

Model::~Model()
{
    for (auto& m : this->meshes)
        delete m;
    this->meshes.clear();
}
