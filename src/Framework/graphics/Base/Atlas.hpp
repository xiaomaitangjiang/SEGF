#pragma once
#include <filesystem>

#include <vector>


#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "SDL_render.h"

using std::filesystem::path;
using std::filesystem::directory_iterator;

    class Atlas {
public:
  Atlas() = default;

  Atlas(Atlas& other)=default;
  Atlas(Atlas&& other) noexcept :tex_list(std::move(other.tex_list)){}

  ~Atlas() {
    for (SDL_Texture* texture : tex_list) {
      SDL_DestroyTexture(texture);
    }
  }

  void load(SDL_Renderer* render,path file_path)
  {
    if(file_path.empty())
    {
      return;
    }
    for(const auto& dir: directory_iterator{file_path})
    {
      SDL_Texture* texture=IMG_LoadTexture(render,dir.path().string().c_str());
      tex_list.push_back(texture);
    }
  }

private:
  std::vector<SDL_Texture*> tex_list;
};
