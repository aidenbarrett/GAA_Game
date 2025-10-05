#include "gaa/match.hpp"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <iostream>
#include <random>
#include <string>
#include <thread>

using namespace std::chrono_literals;

namespace {

void AwaitEnter() {
    std::cout << "\nPress Enter to continue...";
    std::string buffer;
    std::getline(std::cin, buffer);
    std::cout << '\n';
}

void PresentSummary(const std::string &heading, const gaa::MatchSimulator &sim) {
    std::cout << '\n' << heading << '\n';
    std::cout << "  " << gaa::FormatScoreboard(sim.homeTeam(), sim.awayTeam()) << '\n';
    for (const auto &line : gaa::BuildStatsSummary(sim.homeTeam(), sim.awayTeam())) {
        std::cout << "  " << line << '\n';
    }
}

std::string Prompt(const std::string &message, const std::string &fallback) {
    std::cout << message;
    std::string input;
    std::getline(std::cin, input);
    if (input.empty()) {
        return fallback;
    }
    return input;
}

int PromptInt(const std::string &message, int fallback, int min_value, int max_value) {
    std::cout << message;
    std::string input;
    std::getline(std::cin, input);
    if (input.empty()) {
        return fallback;
    }
    try {
        int value = std::stoi(input);
        value = std::clamp(value, min_value, max_value);
        return value;
    } catch (...) {
        return fallback;
    }
}

bool PromptHomeControl() {
    std::cout << "Will you manage the home or away side? (H/A, default H): ";
    std::string input;
    std::getline(std::cin, input);
    if (input.empty()) {
        return true;
    }
    char c = static_cast<char>(std::tolower(input.front()));
    return c != 'a';
}

void IntroduceTeams(const gaa::MatchSimulator &sim) {
    const gaa::Team &home = sim.homeTeam();
    const gaa::Team &away = sim.awayTeam();
    std::cout << "\nWelcome to the Hill!" << '\n';
    std::cout << home.name() << " vs " << away.name() << "\n\n";
    std::cout << "Team attributes:\n";
    std::cout << "  " << home.name() << ": " << home.attributesLine() << '\n';
    std::cout << "  " << away.name() << ": " << away.attributesLine() << '\n';
    std::cout << "\nChoose your play style wisely and try to outscore the opposition in a frenetic, accelerated clash." << '\n';
}

void DisplayCommentary(const gaa::MatchSimulator &sim, const gaa::StepResult &step) {
    if (step.throw_in) {
        std::cout << '\n' << step.throw_in->lead_in << "... " << step.throw_in->message << '\n';
    }

    std::cout << '\n' << '[' << sim.clock().display() << "] " << step.possession.commentary << '\n';
    if (step.possession.show_scoreboard) {
        std::cout << "    " << gaa::FormatScoreboard(sim.homeTeam(), sim.awayTeam()) << '\n';
    }
    for (const auto &line : step.possession.extra) {
        std::cout << "    " << line << '\n';
    }
}

gaa::Tactic PromptTactic(const gaa::Team &attack, const gaa::Team &defence) {
    std::cout << '\n';
    std::cout << "Possession for " << attack.name() << " against " << defence.name() << "." << '\n';
    std::cout << "Choose your approach:" << '\n';
    std::cout << "  1) " << gaa::TacticName(gaa::Tactic::GoForGoal) << " - " << gaa::TacticDetail(gaa::Tactic::GoForGoal) << '\n';
    std::cout << "  2) " << gaa::TacticName(gaa::Tactic::TakeYourPoints) << " - " << gaa::TacticDetail(gaa::Tactic::TakeYourPoints) << '\n';
    std::cout << "  3) " << gaa::TacticName(gaa::Tactic::ControlTempo) << " - " << gaa::TacticDetail(gaa::Tactic::ControlTempo) << '\n';
    std::cout << "> " << std::flush;

    std::string choice;
    std::getline(std::cin, choice);
    if (!choice.empty()) {
        const char c = static_cast<char>(std::tolower(choice.front()));
        if (c == '1') {
            return gaa::Tactic::GoForGoal;
        }
        if (c == '3') {
            return gaa::Tactic::ControlTempo;
        }
    }
    return gaa::Tactic::TakeYourPoints;
}

} // namespace

int main() {
    std::cout << "Gaelic Clash - a lightning-fast GAA match simulator\n";
    std::cout << "---------------------------------------------------\n";

    std::random_device rd;
    std::mt19937 setup_rng(rd());

    const std::string home_name = Prompt("Enter name for the home team (default: Dublin): ", "Dublin");
    const std::string away_name = Prompt("Enter name for the away team (default: Kerry): ", "Kerry");

    gaa::Team home = gaa::Team::CreateRandom(home_name, setup_rng);
    gaa::Team away = gaa::Team::CreateRandom(away_name, setup_rng);

    const bool manage_home = PromptHomeControl();
    const int half_minutes = PromptInt("How long should each half be in simulated minutes? (10-40, default 35): ", 35, 10, 40);

    gaa::MatchConfig config;
    config.user_controls_home = manage_home;
    config.half_minutes = half_minutes;

    gaa::MatchSimulator simulator(std::move(home), std::move(away), config);

    IntroduceTeams(simulator);

    while (!simulator.isFinished()) {
        gaa::StepResult step;
        if (simulator.isUserOnBall()) {
            const gaa::Team &attack = simulator.teamInPossession();
            const gaa::Team &defence = simulator.teamOutOfPossession();
            const gaa::Tactic choice = PromptTactic(attack, defence);
            step = simulator.play(choice);
        } else {
            step = simulator.play();
        }

        DisplayCommentary(simulator, step);

        if (step.halftime && !step.fulltime) {
            std::cout << "\n*** Half-time whistle! ***\n";
            PresentSummary("Half-time summary", simulator);
            AwaitEnter();
        }

        if (!step.fulltime) {
            std::this_thread::sleep_for(320ms);
        }
    }

    std::cout << "\n*** Full-time! ***\n";
    PresentSummary("Full-time report", simulator);
    std::cout << '\n';
    std::cout << "Thanks for playing Gaelic Clash!\n";
    return 0;
}

