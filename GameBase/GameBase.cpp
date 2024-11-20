#include "GameBase.h"

#include <SDL.h>
#include <SDL_mixer.h>
#include <filesystem>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <unistd.h>
#endif

struct PlatformState {
    GameMemory game_memory = {};

    bool is_recording = false;
    bool is_replaying = false;
    
    uint8 *record_ptr = nullptr;
    uint8 *record_end_ptr = nullptr;
};

static bool closing = false;
static void *game_lib_handle;
static uint64 game_lib_time;
static char *save_path;
static uint32 gamepad_index = 0;

SDL_GameController *gamepad_handles[Config::System::MAX_CONTROLLERS];

static void *allocate_memory(void *memory_begin, size_t total_size) {
    void *ptr = nullptr;
#ifdef _WIN32
    ptr = VirtualAlloc(memory_begin, total_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
    ptr = mmap(memory_begin, total_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED) ptr = nullptr;
#endif
    return ptr;
}

#ifdef DEBUG
static void begin_recording(PlatformState &platform_state) {
    platform_state.is_recording = true;
    SDL_memcpy(platform_state.game_memory.record_storage, platform_state.game_memory.permanent_storage, platform_state.game_memory.permanent_storage_size);
    platform_state.record_ptr = (uint8*)platform_state.game_memory.record_storage + platform_state.game_memory.permanent_storage_size;
}

static void end_recording(PlatformState &platform_state) {
    platform_state.is_recording = false;
    platform_state.record_end_ptr = platform_state.record_ptr;
    platform_state.record_ptr = nullptr;
}

static void begin_replaying(PlatformState &platform_state) {
    platform_state.is_replaying = true;
    SDL_memcpy(platform_state.game_memory.permanent_storage, platform_state.game_memory.record_storage, platform_state.game_memory.permanent_storage_size);
    platform_state.record_ptr = (uint8 *)platform_state.game_memory.record_storage + platform_state.game_memory.permanent_storage_size;
}

static void end_replaying(PlatformState &platform_state) {
    platform_state.is_replaying = false;
    platform_state.record_ptr = nullptr;
}

static void handle_record_replay(ControllerInput &controller, PlatformState &platform_state) {
    if (platform_state.is_recording) {
        SDL_memcpy(platform_state.record_ptr, &controller, sizeof(controller));
        platform_state.record_ptr += sizeof(controller);
    } else if (platform_state.is_replaying) {
        if (platform_state.record_ptr >= platform_state.record_end_ptr) {
            end_replaying(platform_state);
            begin_replaying(platform_state);
        } else {
            SDL_memcpy(&controller, platform_state.record_ptr, sizeof(controller));
            if (platform_state.record_ptr == (uint8*)platform_state.game_memory.record_storage + platform_state.game_memory.permanent_storage_size) {
                // Refresh chunks on the first frame of replay
                controller.button_f5 = true;
            }
            platform_state.record_ptr += sizeof(controller);
        }
    }
}
#endif

void handle_events(ControllerInput &controller, PlatformState &platform_state) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            closing = true;
        } else if (event.type == SDL_CONTROLLERDEVICEADDED) {
            LogInfo("Controller added: %d\n", event.cdevice.which);
            const int32 device_index = event.cdevice.which;
            if (SDL_IsGameController(device_index)) {
                SDL_GameControllerOpen(device_index);
            }
        } else if (event.type == SDL_CONTROLLERDEVICEREMOVED) {
            LogInfo("Controller removed: %d\n", event.cdevice.which);
        } else if ((event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) && !event.key.repeat) {
            const bool is_down = event.type == SDL_KEYDOWN;

            switch (event.key.keysym.sym) {
                case SDLK_UP:
                    controller.dir_up = is_down ? 1.0f : 0;
                    break;
                case SDLK_DOWN:
                    controller.dir_down = is_down ? 1.0f : 0;
                    break;
                case SDLK_LEFT:
                    controller.dir_left = is_down ? 1.0f : 0;
                    break;
                case SDLK_RIGHT:
                    controller.dir_right = is_down ? 1.0f : 0;
                    break;
                case SDLK_z:
                case SDLK_s:
                    controller.button_a = is_down;
                    break;
                case SDLK_x:
                case SDLK_w:
                    controller.button_b = is_down;
                    break;
                case SDLK_c:
                case SDLK_a:
                    controller.button_c = is_down;
                    break;
                case SDLK_v:
                case SDLK_d:
                    controller.button_d = is_down;
                    break;
                case SDLK_e:
                    controller.button_r = is_down;
                    break;
                case SDLK_q:
                    controller.button_l = is_down;
                    break;
                case SDLK_SPACE:
                    controller.button_r2 = is_down;
                    break;
                case SDLK_LSHIFT:
                    controller.button_l2 = is_down;
                    break;
                case SDLK_f:
                    controller.button_l3 = is_down;
                    break;
                case SDLK_g:
                    controller.button_r3 = is_down;
                    break;
                case SDLK_ESCAPE:
                    controller.button_select = is_down;
                    break;
                case SDLK_F1:
                    controller.button_f1 = is_down;
                    break;
                case SDLK_F2:
                    controller.button_f2 = is_down;
                    break;
                case SDLK_F3:
                    controller.button_f3 = is_down;
                    break;
                case SDLK_F4:
                    controller.button_f4 = is_down;
                    break;
                case SDLK_F5:
                    controller.button_f5 = is_down;
                    break;
#ifdef DEBUG
                case SDLK_r:
                    if (is_down) {
                        if (!platform_state.is_recording && !platform_state.is_replaying) {
                            begin_recording(platform_state);
                        } else if (platform_state.is_recording && !platform_state.is_replaying) {
                            end_recording(platform_state);
                            begin_replaying(platform_state);
                        } else if (!platform_state.is_recording && platform_state.is_replaying) {
                            end_replaying(platform_state);
                            controller = {};
                        } else {
                            LogError("Impossible recording state!");
                        }
                    }
                    break;
#endif
                default:;
            }
        } else if (event.type == SDL_CONTROLLERAXISMOTION) {
            float32 value;
            if (event.caxis.value > 0) {
                value = ((float32)event.caxis.value) / 32767.f;
            } else {
                value = ((float32)event.caxis.value) / 32768.f;
            }

            switch (event.caxis.axis) {
                case SDL_CONTROLLER_AXIS_LEFTX:
                    if (value < 0) {
                        controller.dir_left = -value;
                        controller.dir_right = 0;
                    } else {
                        controller.dir_right = value;
                        controller.dir_left = 0;
                    }
                    break;
                case SDL_CONTROLLER_AXIS_LEFTY:
                    if (value < 0) {
                        controller.dir_up = -value;
                        controller.dir_down = 0;
                    } else {
                        controller.dir_down = value;
                        controller.dir_up = 0;
                    }
                    break;
                default: ;
            }
        } else if (event.type == SDL_CONTROLLERBUTTONDOWN || event.type == SDL_CONTROLLERBUTTONUP) {
            const bool is_pressed = event.cbutton.state == SDL_PRESSED;

            switch (event.cbutton.button) {
                case SDL_CONTROLLER_BUTTON_DPAD_UP:
                    controller.dir_up = is_pressed ? 1.0f : 0;
                    break;
                case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                    controller.dir_down = is_pressed ? 1.0f : 0;
                    break;
                case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
                    controller.dir_left = is_pressed ? 1.0f : 0;
                    break;
                case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
                    controller.dir_right = is_pressed ? 1.0f : 0;
                    break;
                case SDL_CONTROLLER_BUTTON_A:
                    controller.button_a = is_pressed;
                    break;
                case SDL_CONTROLLER_BUTTON_B:
                    controller.button_b = is_pressed;
                    break;
                case SDL_CONTROLLER_BUTTON_X:
                    controller.button_c = is_pressed;
                    break;
                case SDL_CONTROLLER_BUTTON_Y:
                    controller.button_d = is_pressed;
                    break;
                case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
                    controller.button_l = is_pressed;
                    break;
                case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
                    controller.button_r = is_pressed;
                    break;
                case SDL_CONTROLLER_BUTTON_BACK:
                    controller.button_select = is_pressed;
                    break;
                default:
                    break;
            }
        } else if (event.type == SDL_MOUSEMOTION) {
            controller.mouse_move_x += event.motion.xrel;
            controller.mouse_move_y += event.motion.yrel;

        } else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
            switch (event.button.button) {
                case SDL_BUTTON_LEFT:
                    controller.button_mouse_l = (event.button.state == SDL_PRESSED);
                    break;
                case SDL_BUTTON_RIGHT:
                    controller.button_mouse_r = (event.button.state == SDL_PRESSED);
                    break;
                case SDL_BUTTON_MIDDLE:
                    controller.button_mouse_m = (event.button.state == SDL_PRESSED);
                    break;
                default:
                    break;
            }
        } else if (event.type == SDL_MOUSEWHEEL) {
            controller.mouse_wheel += event.wheel.y;
        }
    }
}

static void sdl_init_audio(int32 samples_per_second, uint16 buffer_sample_size) {
    SDL_AudioSpec audio_settings = {};

    audio_settings.freq = samples_per_second;
    audio_settings.format = AUDIO_S16LSB;
    audio_settings.channels = 2;
    audio_settings.samples = buffer_sample_size;

    SDL_OpenAudio(&audio_settings, nullptr);

    LogInfo("Initialised an Audio device at frequency %d Hz, %d Channels\n", audio_settings.freq, audio_settings.channels);

    if (audio_settings.format != AUDIO_S16LSB) {
        LogError("Oops! We didn't get AUDIO_S16LSB as our sample format!\n");
        SDL_CloseAudio();
    }

    SDL_PauseAudio(0);
}

static void sdl_init_gamepads() {
    SDL_GameControllerAddMapping(
        "030000001008000001e5000000000000,NEXT SNES "
        "Controller,a:b2,b:b1,back:b8,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b4,rightshoulder:b5,start:b9,x:b3,y:b0,");

    const int32 max_joysticks = SDL_NumJoysticks();
    for (int32 joystick_index = 0; joystick_index < max_joysticks; ++joystick_index) {
        if (!SDL_IsGameController(joystick_index)) {
            continue;
        }
        if (gamepad_index >= Config::System::MAX_CONTROLLERS) {
            break;
        }
        gamepad_handles[gamepad_index] = SDL_GameControllerOpen(joystick_index);

        char *mapping = SDL_GameControllerMapping(gamepad_handles[gamepad_index]);
        SDL_Log("Controller %i is mapped as \"%s\".", joystick_index, mapping);

        gamepad_index++;
    }
}

bool is_game_lib_out_of_date(char *base_path) {
    char org_filename[Config::System::MAX_PATH_LEN];
    ASSERT(snprintf(org_filename, Config::System::MAX_PATH_LEN, "%s%s", base_path, "GameCode.dll") > 0);

    struct _stat64 file_stat = {};
    _stati64(((const char *)org_filename), &file_stat);
    return game_lib_time != file_stat.st_mtime;
}

bool load_functions_from_game_lib(const char *lib_path, InitializeFuncType *initialize_func, ReloadInitFuncType *reload_init_func,
                                  UpdateFuncType *update_func, FinalizeFuncType *finalize_func) {
    game_lib_handle = SDL_LoadObject(lib_path);
    if (!game_lib_handle) {
        LogError("Could not load the game lib!");
        return false;
    }
    (*initialize_func) = (InitializeFuncType)SDL_LoadFunction(game_lib_handle, "initialize");
    if (!(*initialize_func)) {
        LogError("Could not load the initialize function from the game lib!");
        return false;
    }
    (*reload_init_func) = (ReloadInitFuncType)SDL_LoadFunction(game_lib_handle, "reload_init");
    if (!(*reload_init_func)) {
        LogError("Could not load the reload_init function from the game lib!");
        return false;
    }
    (*update_func) = (UpdateFuncType)SDL_LoadFunction(game_lib_handle, "game_loop");
    if (!(*update_func)) {
        LogError("Could not load the update function from the game lib!");
        return false;
    }
    (*finalize_func) = (FinalizeFuncType)SDL_LoadFunction(game_lib_handle, "finalize");
    if (!(*finalize_func)) {
        LogError("Could not load the finalize function from the game lib!");
        return false;
    }
    return true;
}

bool copy_file(char *src_path, char *dst_path) {
    try {
        if (std::filesystem::copy_file(src_path, dst_path, std::filesystem::copy_options::overwrite_existing)) {
            return true;
        }
    } catch (const std::filesystem::filesystem_error &e) {
        LogWarn("Filesystem error: %s", e.what());
    }
    return false;
}

bool load_game_lib(char *base_path, InitializeFuncType *initialize_func, ReloadInitFuncType *reload_init_func, UpdateFuncType *update_func, FinalizeFuncType *finalize_func) {
#ifndef DEBUG
    return load_functions_from_game_lib("GameCode.dll", initialize_func, reload_init_func, update_func, finalize_func);
#else
    char org_filename[Config::System::MAX_PATH_LEN];
    char new_filename[Config::System::MAX_PATH_LEN];

    ASSERT(snprintf(org_filename, Config::System::MAX_PATH_LEN, "%s%s", base_path, "GameCode.dll") > 0);
    ASSERT(snprintf(new_filename, Config::System::MAX_PATH_LEN, "%s%s", base_path, "GameCode_temp.dll") > 0);

    int32 tries = 50;
    while (true) {
        if (copy_file(org_filename, new_filename)) {
            const bool result = load_functions_from_game_lib("GameCode_temp.dll", initialize_func, reload_init_func, update_func, finalize_func);
            if (!result) {
                return false;
            }

            struct _stat64 file_stat = {};
            _stati64(((const char *)org_filename), &file_stat);
            game_lib_time = file_stat.st_mtime;

            return true;
        } else {
            LogWarn("Could not find the game lib!");
            if (tries <= 0) {
                LogError("Giving up!");
                return false;
            } else {
                LogInfo("Will try again for the next %dth times!", tries);
                tries--;
                SDL_Delay(10);
            }
        }
    }
#endif
}

void unload_game_lib() {
    if (game_lib_handle) {
        SDL_UnloadObject(game_lib_handle);
        game_lib_handle = nullptr;
    }
}

static float32 sdl_get_seconds_elapsed(uint64 old_counter, uint64 current_counter, uint64 perf_frequency) {
    return ((float32)(current_counter - old_counter) / (float32)(perf_frequency));
}

void handle_hot_reload(char *base_path, InitializeFuncType initialize_func, ReloadInitFuncType reload_init_func, FinalizeFuncType finalize_func,
                       UpdateFuncType game_loop_func, PlatformState &platform_state) {
    if (is_game_lib_out_of_date(base_path)) {
        unload_game_lib();
        const bool result = load_game_lib(base_path, &initialize_func, &reload_init_func, &game_loop_func, &finalize_func);
        if (!result) {
            exit(1);
        }
        reload_init_func(&platform_state.game_memory);
    }
}

void log_fps(uint64 last_counter, uint64 before_sleep_counter, uint64 end_counter, uint64 perf_frequency) {
    const uint64 work_elapsed = before_sleep_counter - last_counter;
    const uint64 total_elapsed = end_counter - last_counter;
    const float64 ms_per_frame_work = (((1000.0 * (float64)work_elapsed) / (float64)perf_frequency));
    const float64 ms_per_frame_total = (((1000.0 * (float64)total_elapsed) / (float64)perf_frequency));
    const float64 fps = (float64)perf_frequency / (float64)total_elapsed;
    const float64 work_fps = (float64)perf_frequency / (float64)work_elapsed;
    LogDebug("%.02f ms/f (%.02f work, %.02f sleep), %.01f fps (%0.1f ideal fps)", ms_per_frame_total, ms_per_frame_work,
             ms_per_frame_total - ms_per_frame_work, fps, work_fps);
}

bool init_memory(GameMemory &memory) {
    memory.permanent_storage_size = Megabytes(128);
#ifdef DEBUG
    // 2 MB is more than 10 minutes of input record
    memory.record_storage_size = memory.permanent_storage_size + Megabytes(2);
#endif
    memory.transient_storage_size = Megabytes(64);
    const uint64 total_size = memory.permanent_storage_size + memory.record_storage_size + memory.transient_storage_size;
    void* memory_begin = (void*)Terabytes(2);
    void *memory_pointer = allocate_memory(memory_begin, total_size);
    if (!memory_pointer) {
        LogError("Could not allocate memory!");
        return false;
    }
    memory.permanent_storage = memory_pointer;
    memory.record_storage = (uint8 *)memory_pointer + memory.permanent_storage_size;
    memory.transient_storage = (uint8 *)memory.record_storage + memory.record_storage_size;
    return true;
}

bool sdl_init_graphics(SDL_Window *&window, SDL_Surface *&screen_surface) {
    int32 screen_width = Config::Graphics::DEBUG_WINDOW_WIDTH;
    int32 screen_height = Config::Graphics::DEBUG_WINDOW_HEIGHT;
    Uint32 window_properties = SDL_WINDOW_OPENGL;
#ifdef DEBUG
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG);
#else
    SDL_DisplayMode display_mode;
    const int32 should_be_zero = SDL_GetCurrentDisplayMode(0, &display_mode);
    window_properties |= SDL_WINDOW_BORDERLESS;
    if (should_be_zero != 0) {
        LogError("Could not get display mode");
    } else {
        screen_width = display_mode.w;
        screen_height = display_mode.h;
    }

    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_WARN);
#endif

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // Multi-sampling for anti aliasing
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);

    window = SDL_CreateWindow(GameName, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, window_properties);
    if (!window) {
        LogError("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }
    LogInfo("Window is created");

    /* Creating OpenGL context */
    const SDL_GLContext gl_ctx = SDL_GL_CreateContext(window);
    if (!gl_ctx) {
        LogError("Could not create OpenGL context: %s\n", SDL_GetError());
        return false;
    }
    SDL_GL_SetSwapInterval(1);

    screen_surface = SDL_GetWindowSurface(window);
    SDL_ShowCursor(SDL_DISABLE);
    sdl_init_gamepads();
    sdl_init_audio(48000, 4096);

    SDL_SetRelativeMouseMode(SDL_TRUE);
    return true;
}

void sleep(float32 seconds, uint64 last_counter, uint64 perf_frequency, float32 TARGET_SPF) {
    const int32 time_to_sleep = (int32)((seconds) * 1000) - 1;
    if (time_to_sleep > 0) {
        SDL_Delay(time_to_sleep);
    }
    while (sdl_get_seconds_elapsed(last_counter, SDL_GetPerformanceCounter(), perf_frequency) < TARGET_SPF) {
        // Precise waiting for the last 1 ms...
    }
}

int32 main(int32, char **) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    ASSERT(atexit(SDL_Quit) == 0);

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        LogError("SDL_mixer could not initialize! SDL_mixer Error: %s", Mix_GetError());
    }

    char *base_path = SDL_GetBasePath();
    save_path = new char[Config::System::MAX_PATH_LEN];
    strcpy_s(save_path, Config::System::MAX_PATH_LEN, base_path);
    strcat_s(save_path, Config::System::MAX_PATH_LEN, "saves\\");
    LogInfo("Base path: %s", base_path);
    LogWarn("Save path: %s", save_path);
    create_dir(save_path);

    InitializeFuncType game_initialize;
    ReloadInitFuncType game_on_reload;
    FinalizeFuncType game_finalize;
    UpdateFuncType game_loop;
    if (!load_game_lib(base_path, &game_initialize, &game_on_reload, &game_loop, &game_finalize)) {
        return 1;
    }

    SDL_Window *window = nullptr;
    SDL_Surface *screen_surface = nullptr;
    if(!sdl_init_graphics(window, screen_surface)) {
        return 1;
    }

    PlatformState platform_state = {};
    if (!init_memory(platform_state.game_memory)) {
        return 1;
    }

    constexpr float32 TARGET_SPF = 1.0f / Config::Game::TARGET_FPS;
    const uint64 perf_frequency = SDL_GetPerformanceFrequency();

    ControllerInput controller = {};
    uint64 last_counter = SDL_GetPerformanceCounter();
    uint64 update_counter = last_counter;
    uint64 frame_count = 0;

    game_initialize(&platform_state.game_memory, save_path);

    /* Main loop */
    while (true) {
        handle_events(controller, platform_state);
        if (controller.button_select) {
            closing = true;
        }

#ifdef DEBUG
        handle_hot_reload(base_path, game_initialize, game_on_reload, game_finalize, game_loop, platform_state);
        handle_record_replay(controller, platform_state);
#endif

        const uint64 new_update_counter = SDL_GetPerformanceCounter();
        const float32 time_delta = sdl_get_seconds_elapsed(update_counter, new_update_counter, perf_frequency);
        update_counter = new_update_counter;

        game_loop(&platform_state.game_memory, screen_surface, window, &controller, time_delta);
        
        const uint64 before_sleep_counter = SDL_GetPerformanceCounter();
        const float32 seconds_elapsed = sdl_get_seconds_elapsed(last_counter, before_sleep_counter, perf_frequency);
        if (seconds_elapsed < TARGET_SPF) {
            sleep(TARGET_SPF - seconds_elapsed, last_counter, perf_frequency, TARGET_SPF);
        }

        const uint64 end_counter = SDL_GetPerformanceCounter();

#ifdef DEBUG
        if (frame_count % 120 == 0) {
            log_fps(last_counter, before_sleep_counter, end_counter, perf_frequency);
        }
#endif

        last_counter = end_counter;
        if (closing) {
            game_finalize(&platform_state.game_memory);
            break;
        }
        frame_count++;
    }

    return 0;
}
