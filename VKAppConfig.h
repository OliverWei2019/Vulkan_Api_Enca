#pragma once
#ifndef _VK_APP_CONFIG_H
#define _VK_APP_CONFIG_H
#include <string>
#include <functional>

struct VKAppConfig {
    int width = 800;
    int height = 600;
    std::string name = "VK_Demo";
    bool debug = false;
    std::function<void(int, int, int)> mouseCallback;
   
    VKAppConfig() = default;
    VKAppConfig(const VKAppConfig& config) :
        name(config.name),
        debug(config.debug),
        mouseCallback(config.mouseCallback)
    {}
};
struct Config {
    int maxFramsInFlight = 2;
    std::string pipelineCacheFile = "pipelineCache";
    std::vector<const char*> enabledExtensions;
};
#endif // !_VK_APP_CONFIG_H
