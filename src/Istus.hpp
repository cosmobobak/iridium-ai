#pragma once

#include "NMSearch.hpp"

template <class State>
class Istus {
   public:
    Negamax<State> search_driver = Negamax<State>();
    State node = State();

    Istus() {
        Istus(99);
    }
    Istus(const long long strength) {
        search_driver.set_time_limit(strength);
    }

    // SETTERS
    void set_time_limit(long long x) {
        search_driver.set_time_limit(x);
    }

    void set_depth_limit(long long x) {
        search_driver.set_depth_limit(x);
    }

    void set_readout(bool b) {
        search_driver.set_readout(b);
    }

    void set_debug(bool b) {
        search_driver.set_debug(b);
    }

    void set_node(State n) {
        node = n;
    }

    void use_time_limit(bool x) {
        search_driver.use_time_limit(x);
    }

    void use_depth_limit(bool x) {
        search_driver.use_depth_limit(x);
    }

    // GETTERS
    auto get_player_move() const {
        return node.get_player_move();
    }

    auto get_node() -> State& {
        return node;
    }

    auto get_turn_modifier() const {
        return node.get_turn();
    }

    auto get_node_eval() const {
        return node.evaluate();
    }

    auto get_node_count() const {
        return search_driver.get_nodes();
    }

    // PREDICATES
    auto is_game_over() const -> bool {
        return node.is_game_over();
    }

    // STATE INTERACTIONS
    void reset_node() {
        node.reset();
    }

    void engine_move() {
        node = search_driver.find_best_next_board(node);
    }

    // I/O
    void print(const std::string input, const std::string end = "\n") {
        std::cout << input << end;
    }

    void show_node() {
        node.show();
    }

    void show_result() const {
        int r;
        r = node.evaluate();
        if (r == 0)
            std::cout << "1/2-1/2" << '\n';
        else if (r == 1)
            std::cout << "1-0" << '\n';
        else
            std::cout << "0-1" << '\n';
    }
};