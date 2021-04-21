#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "Connect4.hpp"
#include "MCSearch.hpp"

using namespace MCSearch;

using intmatrix3 = std::vector<std::vector<std::vector<int>>>;
using intmatrix2 = std::vector<std::vector<int>>;
using doublematrix3 = std::vector<std::vector<std::vector<double>>>;
using doublematrix2 = std::vector<std::vector<double>>;

double winrate_to_tanh(double wr) {
    return (wr / 50.0) - 1.0;
}

auto deepcopytodouble(intmatrix3 in) {
    // create top level space
    doublematrix3 out(in.size());

    for (int i = 0; i < in.size(); i++) {
        // create second level space
        doublematrix2 x(in[i].size());
        out[i] = x;
        for (int j = 0; j < in[i].size(); j++) {
            // create third level space
            std::vector<double> y(in[i][j].size());
            out[i][j] = y;
        }
    }

    for (int i = 0; i < in.size(); i++) {
        for (int j = 0; j < in[i].size(); j++) {
            for (int k = 0; k < in[i][k].size(); k++) {
                // at three deep, copy values
                out[i][j][k] = in[i][j][k];
            }
        }
    }

    return out;
}

auto mpnorm(intmatrix3 mp) {
    auto out = deepcopytodouble(mp);

    for (int game = 0; game < mp.size(); game++) {
        for (int move = 0; move < mp[game].size(); move++) {
            int total = 0;
            for (int pred : mp[game][move]) {
                total += pred;
            }
            for (int i = 0; i < mp[game][move].size(); i++) {
                out[game][move][i] = (double)mp[game][move][i] / (double)total;
            }
        }
    }

    return out;
}

int main(int argc, char const *argv[]) {
    if (argc <= 3) {
        std::cout << "Run with arg1: num_games, arg2: time_per_move, arg3: output filename.";
        return 0;
    }

    int num_games = atoi(argv[1]);
    int rollouts = atoi(argv[2]);
    std::string CSV_FILE_NAME = argv[3];

    std::cout << "generating training data on " << num_games << " games, at " << rollouts << " rollouts per move.\n";
    std::cout << "generation should be done in about " << ((double)num_games * 30.0 * (350.0/50000.0) * rollouts) / (1000.0 * 60.0 * 60.0) << " hours. good luck.\n";

    // index by GAME, MOVE, SLOT
    intmatrix3 game_states(num_games, intmatrix2(42, std::vector<int>(42)));
    // index by GAME, MOVE, COLUMN
    intmatrix3 move_predictions(num_games, intmatrix2(42, std::vector<int>(7)));
    // index by GAME, MOVE
    doublematrix2 state_ratings(num_games, std::vector<double>(42));

    std::vector<int> game_lengths(num_games);
    std::vector<int> game_results(num_games);

    auto engine = Zero<Connect4::State, 8>();
    engine.choose_rollout_limit();
    engine.set_rollout_limit(rollouts);
    engine.set_readout(false);

    for (int game = 0; game < num_games; game++) {
        engine.reset_node();

        int move = 0;
        while (!engine.is_game_over()) {
            // store the current board
            game_states[game][move] = engine.get_node().vectorise_board();

            // get the ratings for moves from this position from the AI
            // and store them
            move_predictions[game][move] = engine.rollout_vector(engine.get_node());

            // get the rating for the current board and store it.
            state_ratings[game][move] = engine.turn_modifier() == 1 ? engine.get_win_prediction() : 100 - engine.get_win_prediction();

            // make a move at random, weighted by how good the moves are.
            engine.set_node(engine.make_sample_move(
                move_predictions[game][move],
                engine.get_node()));

            move++;
        }
        game_lengths[game] = engine.get_node().get_move_count();
        game_results[game] = engine.get_node().evaluate();
        if (num_games > 100 && game % (num_games / 100) == 0) {
            std::cout << ((int)(game / ((double)num_games / 100))) << "\% done!\r";
        }
    }

    // we're done generating data

    doublematrix3 normalisedMovePredictions = mpnorm(move_predictions);

    std::ofstream writer;
    writer.open(CSV_FILE_NAME);

    std::stringstream sb;

    sb << "Board Vector,";
    for (int i = 0; i < 41; i++) sb << "-,";
    sb << "Move Predictions,";
    for (int i = 0; i < 6; i++) sb << "-,";
    sb << "Position Rating,";
    sb << "Game Result,";
    sb << "Game Length,";
    sb << "Game Index\n";

    for (int game = 0; game < num_games; game++) {
        for (int move = 0; move < game_lengths[game]; move++) {
            for (int slot : game_states[game][move]) {
                sb << slot;
                sb << ",";
            }
            for (double pred : normalisedMovePredictions[game][move]) {
                sb << pred;
                sb << ",";
            }
            sb << winrate_to_tanh(state_ratings[game][move]);
            sb << ",";
            sb << game_results[game];
            sb << ",";
            sb << game_lengths[game];
            sb << ",";
            sb << game;
            sb << '\n';
        }
    }
    writer << sb.str();
    writer.close();

    std::cout << "done!       \n";
    return 0;
}
