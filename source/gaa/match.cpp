#include "match.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace gaa {

namespace {

std::string RandomPlayerName(std::mt19937 &rng) {
    static const std::vector<std::string> first_names{
        "Cian",      "Seán",      "Padraig",   "Eoghan",   "Conor",    "Darragh",
        "Micheál",   "Diarmuid",  "Fionn",     "Cathal",   "Odhrán",   "Tiernan",
        "Niall",     "Colm",      "Ronan",     "Tadhg",    "Declan",   "Ruairí",
        "Shane",     "Fergal",    "Gearóid",   "Eoin",     "Lorcán",   "Eamon",
        "Ciarán",    "Oisín",     "Ultan",     "Finn",     "Darach",   "Pádraic"};

    static const std::vector<std::string> last_names{
        "O'Connor",   "McCarthy",   "O'Shea",      "Clifford",   "O'Donnell",   "O'Neill",
        "O'Dwyer",    "O'Rourke",   "Keegan",      "Walsh",      "Flynn",        "O'Sullivan",
        "O'Callaghan", "Buckley",    "Fitzgerald", "McGrath",    "O'Hanlon",     "McHugh",
        "O'Donoghue", "Brennan",    "O'Leary",     "Foley",      "McLoughlin",   "Byrne",
        "O'Malley",   "O'Gara",     "Fitzsimons",  "McKenna",    "O'Boyle",      "Heaney"};

    std::uniform_int_distribution<std::size_t> first_dist(0, first_names.size() - 1);
    std::uniform_int_distribution<std::size_t> last_dist(0, last_names.size() - 1);
    return first_names[first_dist(rng)] + " " + last_names[last_dist(rng)];
}

} // namespace

std::string Score::toString() const {
    std::ostringstream oss;
    oss << goals << '-' << std::setw(2) << std::setfill('0') << points;
    return oss.str();
}

std::string Player::scoringLine() const {
    std::ostringstream oss;
    oss << goals << '-' << std::setw(2) << std::setfill('0') << points;
    return oss.str();
}

Team Team::CreateRandom(std::string name, std::mt19937 &rng) {
    std::uniform_int_distribution<int> rating_dist(68, 92);
    Attributes attr{rating_dist(rng), rating_dist(rng), rating_dist(rng)};

    std::vector<Player> squad;
    squad.reserve(20);
    for (int i = 0; i < 18; ++i) {
        squad.push_back(GeneratePlayer(attr, rng));
    }

    return Team(std::move(name), attr, std::move(squad));
}

Player &Team::randomAttacker(std::mt19937 &rng) {
    std::vector<std::size_t> active_indices;
    active_indices.reserve(squad_.size());
    for (std::size_t i = 0; i < squad_.size(); ++i) {
        if (!squad_[i].injured) {
            active_indices.push_back(i);
        }
    }
    if (active_indices.empty()) {
        return squad_.front();
    }
    std::uniform_int_distribution<std::size_t> dist(0, active_indices.size() - 1);
    return squad_[active_indices[dist(rng)]];
}

void Team::recordGoal(Player &player) {
    ++score_.goals;
    ++player.goals;
}

void Team::recordPoint(Player &player) {
    ++score_.points;
    ++player.points;
}

void Team::recordWide() { ++wides_; }
void Team::recordTurnover() { ++turnovers_; }
void Team::recordFoulDrawn() { ++fouls_won_; }
void Team::recordFoulCommitted() { ++fouls_conceded_; }

std::optional<std::string> Team::handleInjury(Player &player, std::mt19937 &rng) {
    if (player.injured) {
        return std::nullopt;
    }
    player.injured = true;
    ++injuries_;
    Player substitute = GeneratePlayer(attributes_, rng);
    squad_.push_back(substitute);
    return squad_.back().name;
}

std::string Team::attributesLine() const {
    std::ostringstream oss;
    oss << "Attack " << attributes_.attack << ", Defence " << attributes_.defence << ", Discipline " << attributes_.discipline;
    return oss.str();
}

std::string Team::statsLine() const {
    std::ostringstream oss;
    oss << "Wides: " << wides_ << ", Turnovers: " << turnovers_ << ", Fouls won: " << fouls_won_
        << ", Fouls conceded: " << fouls_conceded_ << ", Injuries: " << injuries_;
    return oss.str();
}

std::vector<Player> Team::topScorers(std::size_t count) const {
    std::vector<Player> scorers;
    scorers.reserve(squad_.size());
    for (const auto &player : squad_) {
        if (player.goals > 0 || player.points > 0) {
            scorers.push_back(player);
        }
    }
    std::sort(scorers.begin(), scorers.end(), [](const Player &lhs, const Player &rhs) {
        if (lhs.totalContribution() != rhs.totalContribution()) {
            return lhs.totalContribution() > rhs.totalContribution();
        }
        if (lhs.goals != rhs.goals) {
            return lhs.goals > rhs.goals;
        }
        return lhs.points > rhs.points;
    });
    if (scorers.size() > count) {
        scorers.resize(count);
    }
    return scorers;
}

Team::Team(std::string name, Attributes attributes, std::vector<Player> squad)
    : name_(std::move(name)), attributes_(attributes), squad_(std::move(squad)) {}

Player Team::GeneratePlayer(const Attributes &attr, std::mt19937 &rng) {
    std::uniform_int_distribution<int> flair_delta(-8, 10);
    std::uniform_int_distribution<int> shooting_delta(-12, 12);
    std::uniform_int_distribution<int> toughness_delta(-15, 10);

    auto clamp_rating = [](int value) { return std::clamp(value, 45, 99); };

    Player player;
    player.name = RandomPlayerName(rng);
    player.flair = clamp_rating(attr.attack + flair_delta(rng));
    player.shooting = clamp_rating(attr.attack + shooting_delta(rng));
    player.toughness = clamp_rating(attr.discipline + toughness_delta(rng));
    return player;
}

MatchClock::MatchClock(int half_minutes)
    : half_duration_seconds_(half_minutes * 60), total_duration_seconds_(half_duration_seconds_ * 2) {}

MatchClock::AdvanceResult MatchClock::advance(int seconds) {
    AdvanceResult result;
    result.previous_seconds = elapsed_seconds_;
    const int target = std::min(elapsed_seconds_ + seconds, total_duration_seconds_);
    result.progressed = target - elapsed_seconds_;
    elapsed_seconds_ = target;
    result.current_seconds = elapsed_seconds_;
    result.reached_halftime = result.previous_seconds < half_duration_seconds_ && elapsed_seconds_ >= half_duration_seconds_;
    result.reached_full_time = elapsed_seconds_ >= total_duration_seconds_;
    return result;
}

std::string MatchClock::display() const {
    const int minutes = elapsed_seconds_ / 60;
    const int seconds = elapsed_seconds_ % 60;
    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << minutes << ':' << std::setw(2) << std::setfill('0') << seconds;
    return oss.str();
}

std::string TacticName(Tactic tactic) {
    switch (tactic) {
    case Tactic::GoForGoal:
        return "Go for goal";
    case Tactic::TakeYourPoints:
        return "Take your points";
    case Tactic::ControlTempo:
        return "Control the tempo";
    }
    return "";
}

std::string TacticDetail(Tactic tactic) {
    switch (tactic) {
    case Tactic::GoForGoal:
        return "Direct running at goal. Big reward but a turnover beckons if it goes wrong.";
    case Tactic::TakeYourPoints:
        return "Keep the scoreboard ticking with measured shots from range.";
    case Tactic::ControlTempo:
        return "Slow things down, recycle the ball and try to draw a handy free.";
    }
    return "";
}

MatchSimulator::MatchSimulator(Team home, Team away, MatchConfig config, std::mt19937::result_type seed)
    : clock_(config.half_minutes),
      home_(std::move(home)),
      away_(std::move(away)),
      user_controls_home_(config.user_controls_home),
      pending_throw_in_(PendingThrowIn{.opening = true, .lead_in = "The ball is in the air to get us underway"}),
      rng_(seed) {}

bool MatchSimulator::isUserOnBall() const {
    return (home_possession_ && user_controls_home_) || (!home_possession_ && !user_controls_home_);
}

Tactic MatchSimulator::chooseAiTactic() const {
    const Team &attack = home_possession_ ? home_ : away_;
    const Team &defence = home_possession_ ? away_ : home_;
    const int score_diff = attack.currentScore().total() - defence.currentScore().total();
    const int remaining = clock_.remainingSeconds();
    if (clock_.isSecondHalf() && score_diff <= -3) {
        return Tactic::GoForGoal;
    }
    if (clock_.isSecondHalf() && score_diff >= 4 && remaining < 10 * 60) {
        return Tactic::ControlTempo;
    }
    if (remaining < 5 * 60 && score_diff > 1) {
        return Tactic::ControlTempo;
    }
    if (score_diff < 0) {
        return Tactic::GoForGoal;
    }
    return Tactic::TakeYourPoints;
}

StepResult MatchSimulator::play(std::optional<Tactic> user_choice) {
    StepResult step;
    if (finished_) {
        step.fulltime = true;
        return step;
    }

    if (pending_throw_in_) {
        ThrowInInfo info = executeThrowIn(*pending_throw_in_);
        step.throw_in = info;
        home_possession_ = info.home_won;
        pending_throw_in_.reset();
    }

    Team &attacking = home_possession_ ? home_ : away_;
    Team &defending = home_possession_ ? away_ : home_;

    step.home_attacked = home_possession_;
    step.user_attacked = isUserOnBall();

    const bool user_on_ball = step.user_attacked;
    Tactic tactic = user_on_ball && user_choice ? *user_choice : chooseAiTactic();

    Player &shooter = attacking.randomAttacker(rng_);
    step.possession = resolvePossession(attacking, defending, shooter, tactic);
    applyOutcome(step.possession, attacking, defending, shooter);

    step.clock = clock_.advance(step.possession.seconds_elapsed);
    step.halftime = step.clock.reached_halftime && !step.clock.reached_full_time;
    step.fulltime = step.clock.reached_full_time;

    if (!step.possession.retain_possession) {
        home_possession_ = !home_possession_;
    }

    if (step.halftime) {
        pending_throw_in_ = PendingThrowIn{.opening = false, .lead_in = "Second half throw-in"};
    }

    if (step.fulltime) {
        finished_ = true;
    }

    return step;
}

ThrowInInfo MatchSimulator::executeThrowIn(const PendingThrowIn &request) {
    std::uniform_int_distribution<int> dist(0, 1);
    const bool home_wins = dist(rng_) == 0;
    ThrowInInfo info;
    info.opening = request.opening;
    info.home_won = home_wins;
    info.lead_in = request.lead_in;
    info.message = request.opening ? (home_wins ? home_.name() + " claim the first possession!"
                                               : away_.name() + " control it off the throw-in!")
                                   : (home_wins ? home_.name() + " take the ball!" : away_.name() + " win it this time!");
    return info;
}

PossessionResult MatchSimulator::resolvePossession(Team &attack, Team &defence, const Player &shooter, Tactic tactic) {
    PossessionResult result;

    const double attack_value = static_cast<double>(attack.attackRating()) * 0.6 + static_cast<double>(shooter.shooting) * 0.4;
    const double defence_value = static_cast<double>(defence.defenceRating());
    const double rating_diff = attack_value - defence_value;

    const double base_goal = 0.09;
    const double base_point = 0.46;
    const double base_turnover = 0.18;
    const double base_foul = 0.11;
    const double base_injury = 0.04;

    double goal_chance = base_goal;
    double point_chance = base_point;
    double turnover_chance = base_turnover;
    double foul_chance = base_foul;
    double injury_chance = base_injury;

    switch (tactic) {
    case Tactic::GoForGoal:
        goal_chance += 0.12;
        turnover_chance += 0.08;
        point_chance -= 0.1;
        break;
    case Tactic::TakeYourPoints:
        point_chance += 0.12;
        goal_chance -= 0.04;
        turnover_chance -= 0.05;
        break;
    case Tactic::ControlTempo:
        foul_chance += 0.09;
        point_chance -= 0.08;
        injury_chance += 0.02;
        break;
    }

    goal_chance += rating_diff * 0.0028;
    point_chance += rating_diff * 0.0045;
    turnover_chance -= rating_diff * 0.003;
    foul_chance += (static_cast<double>(attack.disciplineRating()) - defence.disciplineRating()) * 0.002;

    goal_chance = std::clamp(goal_chance, 0.01, 0.35);
    point_chance = std::clamp(point_chance, 0.08, 0.7);
    turnover_chance = std::clamp(turnover_chance, 0.05, 0.4);
    foul_chance = std::clamp(foul_chance, 0.02, 0.35);
    injury_chance = std::clamp(injury_chance, 0.0, 0.2);

    std::array<double, 6> weights{
        goal_chance,
        point_chance,
        1.0 - goal_chance - point_chance - turnover_chance - foul_chance - injury_chance,
        turnover_chance,
        foul_chance,
        injury_chance,
    };

    if (weights[2] < 0.02) {
        weights[2] = 0.02;
    }

    std::discrete_distribution<int> dist(weights.begin(), weights.end());
    const int outcome = dist(rng_);

    switch (static_cast<PossessionOutcome>(outcome)) {
    case PossessionOutcome::Goal:
        result.outcome = PossessionOutcome::Goal;
        result.retain_possession = false;
        result.show_scoreboard = true;
        result.seconds_elapsed = 70 + (rng_() % 50);
        break;
    case PossessionOutcome::Point:
        result.outcome = PossessionOutcome::Point;
        result.retain_possession = false;
        result.show_scoreboard = true;
        result.seconds_elapsed = 40 + (rng_() % 40);
        break;
    case PossessionOutcome::Wide:
        result.outcome = PossessionOutcome::Wide;
        result.retain_possession = false;
        result.seconds_elapsed = 35 + (rng_() % 35);
        break;
    case PossessionOutcome::Turnover:
        result.outcome = PossessionOutcome::Turnover;
        result.retain_possession = false;
        result.seconds_elapsed = 30 + (rng_() % 30);
        break;
    case PossessionOutcome::FoulWon:
        result.outcome = PossessionOutcome::FoulWon;
        result.retain_possession = true;
        result.seconds_elapsed = 25 + (rng_() % 25);
        break;
    case PossessionOutcome::Injury:
        result.outcome = PossessionOutcome::Injury;
        result.retain_possession = true;
        result.seconds_elapsed = 55 + (rng_() % 30);
        break;
    }

    switch (result.outcome) {
    case PossessionOutcome::Goal: {
        static const std::vector<std::string> goal_phrases{
            "buries it low!",
            "rifles it to the top corner!",
            "rounds the keeper and slots home!",
            "sells a dummy and finishes clinically!"};
        std::uniform_int_distribution<std::size_t> phrase_dist(0, goal_phrases.size() - 1);
        result.commentary = "GOAL for " + attack.name() + "! " + shooter.name + ' ' + goal_phrases[phrase_dist(rng_)];
        break;
    }
    case PossessionOutcome::Point: {
        static const std::vector<std::string> point_phrases{
            "splits the posts.",
            "floats it over effortlessly.",
            "clips a beauty from distance.",
            "nails the free with aplomb."};
        std::uniform_int_distribution<std::size_t> phrase_dist(0, point_phrases.size() - 1);
        result.commentary = shooter.name + ' ' + point_phrases[phrase_dist(rng_)];
        break;
    }
    case PossessionOutcome::Wide: {
        static const std::vector<std::string> wide_phrases{
            "drags it wide of the near post.",
            "sees it tail off to the left.",
            "can't keep it between the sticks.",
            "slices the shot badly wide."};
        std::uniform_int_distribution<std::size_t> phrase_dist(0, wide_phrases.size() - 1);
        result.commentary = shooter.name + ' ' + wide_phrases[phrase_dist(rng_)];
        break;
    }
    case PossessionOutcome::Turnover: {
        static const std::vector<std::string> turnover_phrases{
            "is stripped and ",
            "loses it under pressure and ",
            "is dispossessed and ",
            "has the hand-pass cut out and "};
        std::uniform_int_distribution<std::size_t> phrase_dist(0, turnover_phrases.size() - 1);
        result.commentary = shooter.name + ' ' + turnover_phrases[phrase_dist(rng_)] + defence.name() + " break away!";
        break;
    }
    case PossessionOutcome::FoulWon: {
        static const std::vector<std::string> foul_phrases{
            "is dragged back and wins a free.",
            "draws the foul in a scorable position.",
            "is cynically hauled down for a free.",
            "wins a handy free after smart play."};
        std::uniform_int_distribution<std::size_t> phrase_dist(0, foul_phrases.size() - 1);
        result.commentary = shooter.name + ' ' + foul_phrases[phrase_dist(rng_)];
        break;
    }
    case PossessionOutcome::Injury: {
        static const std::vector<std::string> injury_phrases{
            "takes a heavy knock and stays down.",
            "lands awkwardly and needs attention.",
            "is clattered; the medics rush on.",
            "collides mid-air and comes off worse."};
        std::uniform_int_distribution<std::size_t> phrase_dist(0, injury_phrases.size() - 1);
        result.commentary = shooter.name + ' ' + injury_phrases[phrase_dist(rng_)];
        break;
    }
    }

    return result;
}

void MatchSimulator::applyOutcome(PossessionResult &result, Team &attack, Team &defence, Player &shooter) {
    switch (result.outcome) {
    case PossessionOutcome::Goal: {
        attack.recordGoal(shooter);
        std::ostringstream oss;
        oss << shooter.name << " now has " << shooter.scoringLine() << ".";
        result.extra.push_back(oss.str());
        break;
    }
    case PossessionOutcome::Point: {
        attack.recordPoint(shooter);
        std::ostringstream oss;
        oss << shooter.name << " moves to " << shooter.scoringLine() << '.';
        result.extra.push_back(oss.str());
        break;
    }
    case PossessionOutcome::Wide:
        attack.recordWide();
        break;
    case PossessionOutcome::Turnover:
        attack.recordTurnover();
        break;
    case PossessionOutcome::FoulWon:
        attack.recordFoulDrawn();
        defence.recordFoulCommitted();
        result.extra.push_back("Free awarded in their favour.");
        break;
    case PossessionOutcome::Injury: {
        defence.recordFoulCommitted();
        auto sub = attack.handleInjury(shooter, rng_);
        if (sub) {
            result.extra.push_back("Injury blow! " + shooter.name + " must depart. " + *sub + " is on.");
        } else {
            result.extra.push_back("He's patched up but looks shaken.");
        }
        break;
    }
    }

    if (result.outcome == PossessionOutcome::Goal || result.outcome == PossessionOutcome::Point) {
        const int diff = home_.currentScore().total() - away_.currentScore().total();
        if (diff == 0) {
            result.extra.push_back("Level game once more!");
        } else {
            const Team &leader = diff > 0 ? home_ : away_;
            const int margin = std::abs(diff);
            std::ostringstream oss;
            oss << leader.name() << " lead by " << margin << (margin == 1 ? " point." : " points.");
            result.extra.push_back(oss.str());
        }
    }
}

std::string FormatScoreboard(const Team &home, const Team &away) {
    const Score &home_score = home.currentScore();
    const Score &away_score = away.currentScore();
    std::ostringstream oss;
    oss << home.name() << ' ' << home_score.toString() << " (" << home_score.total() << ")  |  " << away.name() << ' '
        << away_score.toString() << " (" << away_score.total() << ")";
    return oss.str();
}

std::vector<std::string> BuildTopScorerLines(const Team &team) {
    std::vector<std::string> lines;
    auto scorers = team.topScorers(3);
    if (scorers.empty()) {
        lines.push_back("No scorers yet.");
        return lines;
    }
    for (const auto &player : scorers) {
        std::ostringstream oss;
        oss << player.name << ' ' << player.scoringLine() << " (" << player.totalContribution()
            << (player.totalContribution() == 1 ? " point)" : " points)");
        lines.push_back(oss.str());
    }
    return lines;
}

std::vector<std::string> BuildStatsSummary(const Team &home, const Team &away) {
    std::vector<std::string> lines;
    lines.push_back(home.name() + " - " + home.statsLine());
    for (const auto &line : BuildTopScorerLines(home)) {
        lines.push_back("  " + line);
    }
    lines.push_back(away.name() + " - " + away.statsLine());
    for (const auto &line : BuildTopScorerLines(away)) {
        lines.push_back("  " + line);
    }
    return lines;
}

std::string RandomCountyName() {
    static const std::vector<std::string> counties{
        "Dublin",   "Kerry",     "Cork",      "Galway",   "Mayo",      "Tyrone",    "Donegal",
        "Kildare",  "Armagh",    "Monaghan",  "Limerick", "Clare",     "Tipperary", "Waterford",
        "Kilkenny", "Wexford",   "Meath",     "Roscommon", "Louth",    "Down",      "Offaly",
        "Westmeath", "Antrim",   "Derry",     "Laois",    "Longford",  "Sligo",     "Leitrim",
        "Carlow",   "Fermanagh", "Cavan"};

    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<std::size_t> dist(0, counties.size() - 1);
    return counties[dist(rng)];
}

} // namespace gaa

