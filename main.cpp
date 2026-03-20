#include <cstddef>
#include <cstdint>
#include <atomic>
#include <iostream>
#include <vector>
#include <variant>

static constexpr size_t CACHE_LINE = 64;
template<typename T>
struct alignas(CACHE_LINE) CacheAligned
{
    T value;
    char padding[CACHE_LINE - sizeof(T)];

    T& get() {return value;}
    const T& get() const {return value;}
};


struct RingBufferHeader
{
    CacheAligned<std::atomic<uint64_t>> write_pos;
    CacheAligned<std::atomic<uint64_t>> read_pos;
};


struct LoadTexture
{
    uint32_t slot_id;
    uint32_t width;
    uint32_t height;
};

struct UpdateBones
{
    uint32_t skeleton_id;
    uint32_t frame_number;
};
struct ResizeSwapchain {
    uint32_t width;
    uint32_t height;
};
struct Shutdown{};

using RenderCommand = std::variant<LoadTexture, UpdateBones, Shutdown, ResizeSwapchain>;

void handle_command( RenderCommand& cmd, std::vector<RenderCommand>& v)
{

    std::visit([&](const auto& c)
    {
        using T = std::decay_t<decltype(c)>;

        if constexpr (std::is_same_v<T, LoadTexture>)
        {
            v.push_back(std::move(cmd));
        } 
        else if constexpr (std::is_same_v<T, UpdateBones>) {
            v.push_back(std::move(cmd));

        }
         else if constexpr (std::is_same_v<T, Shutdown>) {
            v.push_back(std::move(cmd));
        }
        else if constexpr(std::is_same_v<T, ResizeSwapchain>)
        {
            v.push_back(std::move(cmd));
        }
            


    }, cmd);
}

void print_command(const std::vector<RenderCommand>& v)
{
    for (const auto& cmd : v) {
        std::visit([&](const auto& c) {
            using T = std::decay_t<decltype(c)>;

            if constexpr (std::is_same_v<T, LoadTexture>) {
                std::cout << "Loading Texture: " << c.slot_id << std::endl;
            } 
            else if constexpr (std::is_same_v<T, UpdateBones>) {
                std::cout << "Updating Bones for ID: " << c.skeleton_id << std::endl;
            }
            else if constexpr (std::is_same_v<T, Shutdown>) {
                std::cout << "Shutdown Command Received" << std::endl;
            }
            else if constexpr(std::is_same_v<T, ResizeSwapchain>)
            {
                std::cout << "Resize swap chain: " << c.width << std::endl;

            }
        }, cmd);
    }
}

int main() {

    std::vector<RenderCommand> vec;
    LoadTexture lt;
    lt.height = 12;
    lt.slot_id = 1;
    lt.width = 12;
    vec.push_back(ResizeSwapchain{2,3});
    RenderCommand rc = lt;
    handle_command(rc, vec);
    print_command(vec);


    static_assert(sizeof(CacheAligned<std::atomic<uint64_t>>) == CACHE_LINE, 
                  "CacheAligned size must equal cache line");
    static_assert(alignof(CacheAligned<std::atomic<uint64_t>>) == CACHE_LINE,
                  "CacheAligned must be cache-line aligned");
    
    RingBufferHeader header;
    
    std::cout << "sizeof(CacheAligned<atomic<uint64_t>>): " 
              << sizeof(CacheAligned<std::atomic<uint64_t>>) << "\n";
    std::cout << "sizeof(RingBufferHeader): " 
              << sizeof(RingBufferHeader) << "\n";
    std::cout << "write_pos addr: " 
              << &header.write_pos << "\n";
    std::cout << "read_pos addr:  " 
              << &header.read_pos << "\n";
    
    return 0;
}