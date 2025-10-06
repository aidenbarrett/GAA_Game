#pragma once

#include <chrono>
#include <optional>
#include <random>
#include <string>
#include <vector>

namespace gaa {

struct Score {
    int goals = 0;
    int points = 0;

    [[nodiscard]] int total() const { return goals * 3 + points; }
    [[nodiscard]] std::string toString() const;
};

struct Player {
    std::string name;
    int flair = 70;
    int shooting = 70;
    int toughness = 70;
    bool injured = false;
    int goals = 0;
    int points = 0;

    [[nodiscard]] int totalContribution() const { return goals * 3 + points; }
    [[nodiscard]] std::string scoringLine() const;
};

class Team {
public:
    struct Attributes {
        int attack = 70;
        int defence = 70;
        int discipline = 70;
    };

    static Team CreateRandom(std::string name, std::mt19937 &rng);

    const std::string &name() const { return name_; }
    int attackRating() const { return attributes_.attack; }
    int defenceRating() const { return attributes_.defence; }
    int disciplineRating() const { return attributes_.discipline; }

    const Score &currentScore() const { return score_; }

    [[nodiscard]] Player &randomAttacker(std::mt19937 &rng);

    void recordGoal(Player &player);
    void recordPoint(Player &player);
    void recordWide();
    void recordTurnover();
    void recordFoulDrawn();
    void recordFoulCommitted();
    std::optional<std::string> handleInjury(Player &player, std::mt19937 &rng);

    [[nodiscard]] std::string attributesLine() const;
    [[nodiscard]] std::string statsLine() const;
    [[nodiscard]] std::vector<Player> topScorers(std::size_t count) const;

private:
    Team(std::string name, Attributes attributes, std::vector<Player> squad);

    static Player GeneratePlayer(const Attributes &attr, std::mt19937 &rng);

    std::string name_;
    Attributes attributes_{};
    Score score_{};
    int wides_ = 0;
    int turnovers_ = 0;
    int fouls_won_ = 0;
    int fouls_conceded_ = 0;
    int injuries_ = 0;
    std::vector<Player> squad_{};
};

class MatchClock {
public:
    explicit MatchClock(int half_minutes);

    struct AdvanceResult {
        int previous_seconds = 0;
        int current_seconds = 0;
        int progressed = 0;
        bool reached_halftime = false;
        bool reached_full_time = false;
    };

    [[nodiscard]] AdvanceResult advance(int seconds);
    [[nodiscard]] bool isFullTime() const { return elapsed_seconds_ >= total_duration_seconds_; }
    [[nodiscard]] bool isSecondHalf() const { return elapsed_seconds_ >= half_duration_seconds_; }
    [[nodiscard]] int elapsedSeconds() const { return elapsed_seconds_; }
    [[nodiscard]] int remainingSeconds() const { return total_duration_seconds_ - elapsed_seconds_; }
    [[nodiscard]] int halfDuration() const { return half_duration_seconds_; }
    [[nodiscard]] std::string display() const;

private:
    int half_duration_seconds_ = 0;
    int total_duration_seconds_ = 0;
    int elapsed_seconds_ = 0;
};

enum class Tactic {
    GoForGoal = 1,
    TakeYourPoints = 2,
    ControlTempo = 3,
};

std::string TacticName(Tactic tactic);
std::string TacticDetail(Tactic tactic);

enum class PossessionOutcome {
    Goal,
    Point,
    Wide,
    Turnover,
    FoulWon,
    Injury,
};

struct PossessionResult {
    PossessionOutcome outcome = PossessionOutcome::Wide;
    bool retain_possession = false;
    bool show_scoreboard = false;
    int seconds_elapsed = 0;
    std::string commentary;
    std::vector<std::string> extra;
};

struct ThrowInInfo {
    bool opening = false;
    bool home_won = false;
    std::string lead_in;
    std::string message;
};

struct MatchConfig {
    bool user_controls_home = true;
    int half_minutes = 35;
};

struct StepResult {
    bool home_attacked = false;
    bool user_attacked = false;
    PossessionResult possession;
    MatchClock::AdvanceResult clock;
    bool halftime = false;
    bool fulltime = false;
    std::optional<ThrowInInfo> throw_in;
};

class MatchSimulator {
public:
    MatchSimulator(Team home, Team away, MatchConfig config, std::mt19937::result_type seed = std::random_device{}());

    const Team &homeTeam() const { return home_; }
    const Team &awayTeam() const { return away_; }
    Team &homeTeam() { return home_; }
    Team &awayTeam() { return away_; }

    bool userControlsHome() const { return user_controls_home_; }
    bool homeHasBall() const { return home_possession_; }

    Team &teamInPossession() { return home_possession_ ? home_ : away_; }
    Team &teamOutOfPossession() { return home_possession_ ? away_ : home_; }
    const Team &teamInPossession() const { return home_possession_ ? home_ : away_; }
    const Team &teamOutOfPossession() const { return home_possession_ ? away_ : home_; }

    const MatchClock &clock() const { return clock_; }
    bool isFinished() const { return finished_; }
    bool isWaitingForThrowIn() const { return pending_throw_in_.has_value(); }

    bool isUserOnBall() const;

    Tactic chooseAiTactic() const;

    StepResult play(std::optional<Tactic> user_choice = std::nullopt);

private:
    struct PendingThrowIn {
        bool opening = false;
        std::string lead_in;
    };

    ThrowInInfo executeThrowIn(const PendingThrowIn &request);
    PossessionResult resolvePossession(Team &attack, Team &defence, const Player &shooter, Tactic tactic);
    void applyOutcome(PossessionResult &result, Team &attack, Team &defence, Player &shooter);

    MatchClock clock_;
    Team home_;
    Team away_;
    bool user_controls_home_ = true;
    bool home_possession_ = true;
    bool finished_ = false;
    std::optional<PendingThrowIn> pending_throw_in_;
    std::mt19937 rng_;
};

std::string FormatScoreboard(const Team &home, const Team &away);
std::vector<std::string> BuildStatsSummary(const Team &home, const Team &away);
std::vector<std::string> BuildTopScorerLines(const Team &team);

std::string RandomCountyName();

} // namespace gaa

