#include <iostream>
#include <cmath>
#include <numbers>
#include <chrono>
#include <thread>
#include <queue>
#include <vector>
#include <boost/program_options.hpp>
#include <cuda_runtime.h>
#include <glm/glm.hpp>

#include "raytrace.h"
#include "tira/image.h"
#include "tira/parser.h"
#include "tira/graphics/camera.h"

namespace po = boost::program_options;

std::string in_inputname, in_outputname = "output.bmp";
unsigned int in_threads = 1, in_blocksize = 128;

tira::camera Camera;
glm::vec3 Background;
std::vector<sphere> Objects;
std::vector<light> Lights;

void loadScene(tira::parser& p) {
    Camera.position({p.get<float>("camera_position", 0),
                     p.get<float>("camera_position", 1),
                     p.get<float>("camera_position", 2)});
    Camera.lookat({p.get<float>("camera_look", 0),
                   p.get<float>("camera_look", 1),
                   p.get<float>("camera_look", 2)});
    Camera.fov(p.get<float>("camera_fov", 0));

    Background = {p.get<float>("background", 0),
                  p.get<float>("background", 1),
                  p.get<float>("background", 2)};

    for (size_t i = 0; i < p.count("sphere"); ++i) {
        Objects.push_back({p.get<float>("sphere", i, 0),
                           {p.get<float>("sphere", i, 1),
                            p.get<float>("sphere", i, 2),
                            p.get<float>("sphere", i, 3)},
                           {p.get<float>("sphere", i, 4),
                            p.get<float>("sphere", i, 5),
                            p.get<float>("sphere", i, 6)}});
    }

    for (size_t i = 0; i < p.count("light"); ++i) {
        Lights.push_back({{p.get<float>("light", i, 0),
                           p.get<float>("light", i, 1),
                           p.get<float>("light", i, 2)},
                          {p.get<float>("light", i, 3),
                           p.get<float>("light", i, 4),
                           p.get<float>("light", i, 5)}});
    }
}

ray pixel2ray(unsigned int res, unsigned int x, unsigned int y) {
    float fx = (float)x / res - 0.5f, fy = -((float)y / res - 0.5f);
    return {Camera.position(), Camera.ray(fx, fy)};
}

void TraceBlock(unsigned bx, unsigned by, unsigned block_size, tira::image<unsigned char>& image) {
    unsigned width = image.width(), height = image.height(), res = std::max(width, height);

    for (unsigned y = 0; y < block_size; ++y) {
        unsigned global_y = by * block_size + y;
        for (unsigned x = 0; x < block_size; ++x) {
            unsigned global_x = bx * block_size + x;
            if (global_x >= width || global_y >= height) continue;

            ray r = pixel2ray(res, global_x, global_y);
            hit closest_hit{.t = 1e9f};
            for (auto& obj : Objects) {
                hit h;
                if (obj.intersect(r, h) && h.t < closest_hit.t) closest_hit = h;
            }

            if (closest_hit.t == 1e9f) {
                image(global_x, global_y, 0) = Background.r * 255;
                image(global_x, global_y, 1) = Background.g * 255;
                image(global_x, global_y, 2) = Background.b * 255;
            } else {
                glm::vec3 finalColor(0.0f);
                for (auto& l : Lights) {
                    glm::vec3 lightDir = glm::normalize(l.position - closest_hit.pos);
                    float dot = glm::dot(lightDir, closest_hit.norm);
                    if (dot > 0) finalColor += dot * l.color * closest_hit.color;
                }
                finalColor = glm::clamp(finalColor, 0.0f, 1.0f);
                image(global_x, global_y, 0) = finalColor.r * 255;
                image(global_x, global_y, 1) = finalColor.g * 255;
                image(global_x, global_y, 2) = finalColor.b * 255;
            }
        }
    }
}

void render(tira::image<unsigned char>& image) {
    unsigned width = image.width(), height = image.height();
    unsigned bx_dim = (width + in_blocksize - 1) / in_blocksize;
    unsigned by_dim = (height + in_blocksize - 1) / in_blocksize;

    if (in_threads > 1) {
        std::queue<std::thread> threads;
        for (unsigned bx = 0; bx < bx_dim; ++bx) {
            for (unsigned by = 0; by < by_dim; ++by) {
                if (threads.size() >= in_threads) {
                    threads.front().join();
                    threads.pop();
                }
                threads.emplace(TraceBlock, bx, by, in_blocksize, std::ref(image));
            }
        }
        while (!threads.empty()) {
            threads.front().join();
            threads.pop();
        }
    } else {
        for (unsigned bx = 0; bx < bx_dim; ++bx)
            for (unsigned by = 0; by < by_dim; ++by)
                TraceBlock(bx, by, in_blocksize, image);
    }
}

int main(int argc, char* argv[]) {
    po::options_description desc("Options");
    desc.add_options()
        ("help,h", "Show help")
        ("input,i", po::value<std::string>(&in_inputname)->required(), "Input scene file")
        ("output,o", po::value<std::string>(&in_outputname)->default_value("output.bmp"), "Output image")
        ("threads,t", po::value<unsigned>(&in_threads)->default_value(1), "Thread count")
        ("blocksize,b", po::value<unsigned>(&in_blocksize)->default_value(128), "Block size");

    try {
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        if (vm.count("help")) {
            std::cout << desc << std::endl;
            return 0;
        }
        po::notify(vm);
    } catch (const po::error& ex) {
        std::cerr << "Error: " << ex.what() << "\n" << desc << std::endl;
        return 1;
    }

    tira::parser scene(in_inputname);
    loadScene(scene);

    tira::image<unsigned char> image(scene.get<unsigned>("resolution", 0), scene.get<unsigned>("resolution", 1), 3);
    render(image);
    image.save(in_outputname);

    return 0;
}