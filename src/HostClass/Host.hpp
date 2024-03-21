/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include "../Includes.hpp"

class VM_Host final {
public:
    bool machineLoaded{ false };
    bool programLoaded{ false };
    bool benchmarking{ false };
    [[maybe_unused]] u64 cycles{};

    VM_Host(const char*);
    ~VM_Host();

    class FileInfo final {
        VM_Host& Host;
    public:
        std::filesystem::path homeDirectory{};
        std::filesystem::path permRegs{};
        std::string path{};
        std::string name{};
        std::string type{};
        std::string sha1{};
        std::size_t size{};
        
        explicit FileInfo(VM_Host&);
        void reset();
        bool verifyHome();
        bool verifyFile(const char*);
    } File{ *this };

    struct AudioSettings final {
        const u32 outFrequency;
        SDL_AudioDeviceID device{};
        u32 volume{};
        s16 amplitude{};
        SDL_AudioSpec spec{};
        std::function<void(s16*, u32)> handler{};

        AudioSettings();
        void setSpec(VM_Host*);
        void setVolume(s32);
        static void audioCallback(void*, u8*, s32);
    } Audio;

    struct RenderSettings final {
        SDL_Window*   window{};
        SDL_Renderer* renderer{};
        SDL_Texture*  texture{};

        const std::string emuName{ "[21.03.24]"sv };

        const std::string emuVersion{};
              std::string title{};

        bool createWindow();
        bool createRenderer();

        void changeTitle(std::string_view);
        void errorMessage(std::string&&);

        //SDL_Rect screen{};
        float aspect{};
        u32* pixels{};
        s32  pitch{};

        s32 window_W{ 800 };
        s32 window_H{ 400 };

        RenderSettings();
        void lockTexture();
        void unlockTexture();
        void createTexture(s32, s32);
        void setTextureAlpha(usz);
        void setTextureBlend(SDL_BlendMode);
        void setAspectRatio(float);

        void present(bool);
    } Render;

    void runMachine(VM_Guest&);
    void addMessage(std::string_view, bool = true, usz = 0);

    [[nodiscard]] bool machineValid() const;
    [[nodiscard]] bool programValid() const;
};
