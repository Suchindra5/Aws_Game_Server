#ifndef SPATIAL_GRID_HPP
#define SPATIAL_GRID_HPP

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <utility>
#include <cstdint>

struct CellHash {
    template <class T1, class T2>
    std::size_t operator()(const std::pair<T1, T2>& p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);
        return h1 ^ (h2 << 1);
    }
};

struct PlayerPosition {
    float x;
    float y;
};

class SpatialGrid {
private:
    float cell_size;
    std::unordered_map<std::pair<int, int>, std::unordered_set<uint32_t>, CellHash> grid;
    std::unordered_map<uint32_t, std::pair<int, int>> player_cells;
    std::unordered_map<uint32_t, PlayerPosition> player_positions;

public:
    explicit SpatialGrid(float size);

    void update_player(uint32_t player_id, float x, float y);
    void remove_player(uint32_t player_id);
    float get_player_x(uint32_t player_id) const;
    float get_player_y(uint32_t player_id) const;
    std::vector<uint32_t> get_nearby_players(float x, float y, int radius_cells) const;
    size_t get_tracked_player_count() const;
};

#endif // SPATIAL_GRID_HPP