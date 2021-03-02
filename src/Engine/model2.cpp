#include "model2.h"

Model::Model(const std::string &path)
{
    Log::separator();

    // TODO: wrap in internal? is it really relative path?
    // convert to relative path.
    auto p = std::filesystem::proximate(path);
    std::string p_string{p.u8string()};

    this->directory = p_string.substr(0, p_string.find_last_of('/'));
    this->filename = p_string.substr(this->directory.length() + 1, p_string.length());

    std::string error = ASSET_load_meshes(this->directory, this->filename, &this->meshes);
    if (error != "")
    {
        this->directory = "";
        this->filename = "";

        Log::error("asset_load_model: '%s'\n", error.c_str());
        Log::separator();
    }


    Log::info("load model [directory]: %s\n", this->directory.c_str());
    Log::info("load model [file]: %s\n", this->filename.c_str());
    Log::separator();
}
