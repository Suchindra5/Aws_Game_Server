#include "SpatialGrid.hpp"

SpatialGrid::SpatialGrid(float size) : cell_size(size) {}

void SpatialGrid::update_player(uint32_t player_id, float x, float y) {
    player_positions[player_id] = {x, y};

    int new_cell_x = static_cast<int>(x / cell_size);
    int new_cell_y = static_cast<int>(y / cell_size);
    std::pair<int, int> new_cell(new_cell_x, new_cell_y);

    if (player_cells.find(player_id) != player_cells.end()) {
        std::pair<int, int> old_cell = player_cells[player_id];
        if (old_cell != new_cell) {
            grid[old_cell].erase(player_id);
            if (grid[old_cell].empty()) grid.erase(old_cell);
            player_cells[player_id] = new_cell;
            grid[new_cell].insert(player_id);
        }
    } else {
        player_cells[player_id] = new_cell;
        grid[new_cell].insert(player_id);
    }
}

void SpatialGrid::remove_player(uint32_t player_id) {
    if (player_cells.find(player_id) != player_cells.end()) {
        std::pair<int, int> cell = player_cells[player_id];
        grid[cell].erase(player_id);
        if (grid[cell].empty()) grid.erase(cell);
        player_cells.erase(player_id);
    }
    player_positions.erase(player_id);
}

float SpatialGrid::get_player_x(uint32_t player_id) const {
    auto it = player_positions.find(player_id);
    return (it != player_positions.end()) ? it->second.x : 0.0f;
}

float SpatialGrid::get_player_y(uint32_t player_id) const {
    auto it = player_positions.find(player_id);
    return (it != player_positions.end()) ? it->second.y : 0.0f;
}

std::vector<uint32_t> SpatialGrid::get_nearby_players(float x, float y, int radius_cells) const {
    std::vector<uint32_t> nearby;
    int center_cx = static_cast<int>(x / cell_size);
    int center_cy = static_cast<int>(y / cell_size);

    for (int cx = center_cx - radius_cells; cx <= center_cx + radius_cells; ++cx) {
        for (int cy = center_cy - radius_cells; cy <= center_cy + radius_cells; ++cy) {
            std::pair<int, int> cell(cx, cy);
            auto it = grid.find(cell);
            if (it != grid.end()) {
                for (uint32_t pid : it->second) {
                    nearby.push_back(pid);
                }
            }
        }
    }
    return nearby;
}

size_t SpatialGrid::get_tracked_player_count() const {
    return player_cells.size();
}