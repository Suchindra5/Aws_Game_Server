#include "SpatialGrid.hpp"

SpatialGrid::SpatialGrid(float size) : cell_size(size) {}

void SpatialGrid::update_player(uint32_t player_id, float x, float y) {
    int new_cell_x = static_cast<int>(x / cell_size);
    int new_cell_y = static_cast<int>(y / cell_size);
    std::pair<int, int> new_cell = {new_cell_x, new_cell_y};

    auto it = player_cells.find(player_id);
    if (it != player_cells.end()) {
        if (it->second == new_cell) {
            return;
        }
        grid[it->second].erase(player_id);
        if (grid[it->second].empty()) {
            grid.erase(it->second);
        }
    }

    grid[new_cell].insert(player_id);
    player_cells[player_id] = new_cell;
}

void SpatialGrid::remove_player(uint32_t player_id) {
    auto it = player_cells.find(player_id);
    if (it != player_cells.end()) {
        grid[it->second].erase(player_id);
        if (grid[it->second].empty()) {
            grid.erase(it->second);
        }
        player_cells.erase(player_id);
    }
}

std::vector<uint32_t> SpatialGrid::get_nearby_players(float x, float y, int radius_cells) const {
    std::vector<uint32_t> nearby_players;

    int center_cell_x = static_cast<int>(x / cell_size);
    int center_cell_y = static_cast<int>(y / cell_size);

    for (int dx = -radius_cells; dx <= radius_cells; ++dx) {
        for (int dy = -radius_cells; dy <= radius_cells; ++dy) {
            std::pair<int, int> neighbor_cell = {center_cell_x + dx, center_cell_y + dy};

            auto it = grid.find(neighbor_cell);
            if (it != grid.end()) {
                for (uint32_t player_id : it->second) {
                    nearby_players.push_back(player_id);
                }
            }
        }
    }

    return nearby_players;
}

size_t SpatialGrid::get_tracked_player_count() const {
    return player_cells.size();
}