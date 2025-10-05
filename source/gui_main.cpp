#include "gaa/match.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <deque>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include <raylib.h>

namespace {

struct Button {
    Rectangle bounds{};
    std::string text;

    bool draw(Color base, Color hover, Color text_color, int font_size = 20) const {
        const Vector2 mouse = GetMousePosition();
        const bool highlighted = CheckCollisionPointRec(mouse, bounds);
        DrawRectangleRounded(bounds, 0.2f, 8, highlighted ? hover : base);
        const int text_width = MeasureText(text.c_str(), font_size);
        const float text_x = bounds.x + (bounds.width - text_width) * 0.5f;
        const float text_y = bounds.y + (bounds.height - font_size) * 0.5f;
        DrawText(text.c_str(), static_cast<int>(text_x), static_cast<int>(text_y), font_size, text_color);
        return highlighted && IsMouseButtonReleased(MOUSE_LEFT_BUTTON);
    }
};

struct TextField {
    Rectangle bounds{};
    std::string label;
    std::string value;
    bool active = false;

    void draw() {
        DrawText(label.c_str(), static_cast<int>(bounds.x), static_cast<int>(bounds.y - 24), 20, RAYWHITE);
        DrawRectangleRounded(bounds, 0.15f, 6, active ? Fade(LIGHTGRAY, 0.9f) : Fade(DARKGRAY, 0.6f));
        const std::string display = value.empty() ? "Click to edit" : value;
        DrawText(display.c_str(), static_cast<int>(bounds.x + 12), static_cast<int>(bounds.y + bounds.height / 2 - 10), 20,
                 value.empty() ? Fade(RAYWHITE, 0.6f) : RAYWHITE);
    }
};

void HandleTextFieldInput(TextField &field) {
    if (!field.active) {
        return;
    }
    int key = GetCharPressed();
    while (key > 0) {
        if (key >= 32 && key <= 125 && field.value.size() < 20) {
            field.value.push_back(static_cast<char>(key));
        }
        key = GetCharPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE) && !field.value.empty()) {
        field.value.pop_back();
    }
}

void DrawPanel(const Rectangle &bounds, Color colour) {
    DrawRectangleRounded(bounds, 0.1f, 8, colour);
}

void PushCommentary(std::deque<std::string> &log, const std::string &line) {
    if (line.empty()) {
        return;
    }
    log.push_front(line);
    while (log.size() > 12) {
        log.pop_back();
    }
}

std::string FormatThrowIn(const gaa::MatchSimulator &sim, const gaa::ThrowInInfo &info) {
    return info.lead_in + "... " + info.message;
}

std::string FormatTimeLine(const gaa::MatchSimulator &sim, const gaa::StepResult &step) {
    return '[' + sim.clock().display() + "] " + step.possession.commentary;
}

struct GuiMatch {
    std::unique_ptr<gaa::MatchSimulator> simulator;
    bool awaiting_user_choice = false;
    bool halftime_pause = false;
    bool finished = false;
    std::deque<std::string> commentary;
    std::vector<std::string> halftime_summary;
    std::vector<std::string> fulltime_summary;
    float timer = 0.0f;
};

enum class ScreenState {
    Setup,
    Playing,
    Halftime,
    Finished,
};

struct SetupData {
    TextField home{Rectangle{120, 160, 320, 48}, "Home County", "Dublin"};
    TextField away{Rectangle{120, 260, 320, 48}, "Away County", "Kerry"};
    bool manage_home = true;
    int half_minutes = 20;
};

Color Blend(const Color &a, const Color &b, float t) {
    return Color{static_cast<unsigned char>(a.r + (b.r - a.r) * t),
                 static_cast<unsigned char>(a.g + (b.g - a.g) * t),
                 static_cast<unsigned char>(a.b + (b.b - a.b) * t),
                 static_cast<unsigned char>(a.a + (b.a - a.a) * t)};
}

} // namespace

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(1280, 720, "Gaelic Clash Arena");
    SetTargetFPS(60);

    std::mt19937 rng(std::random_device{}());
    SetupData setup;
    GuiMatch match;
    ScreenState state = ScreenState::Setup;

    while (!WindowShouldClose()) {
        const float dt = GetFrameTime();
        BeginDrawing();
        ClearBackground(Color{30, 70, 40, 255});

        if (state == ScreenState::Setup) {
            HandleTextFieldInput(setup.home);
            HandleTextFieldInput(setup.away);

            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                const Vector2 mouse = GetMousePosition();
                setup.home.active = CheckCollisionPointRec(mouse, setup.home.bounds);
                setup.away.active = CheckCollisionPointRec(mouse, setup.away.bounds);
                if (!setup.home.active) {
                    setup.home.active = false;
                }
                if (!setup.away.active) {
                    setup.away.active = false;
                }
            }

            DrawText("Gaelic Clash Arena", 120, 80, 48, RAYWHITE);
            DrawText("Craft your showdown, then dive into a living match presentation.", 120, 120, 20, Fade(RAYWHITE, 0.7f));

            setup.home.draw();
            setup.away.draw();

            Button random_home{Rectangle{460, 160, 180, 48}, "Randomise"};
            if (random_home.draw(Fade(DARKGREEN, 0.7f), Fade(LIME, 0.7f), BLACK)) {
                setup.home.value = gaa::RandomCountyName();
            }

            Button random_away{Rectangle{460, 260, 180, 48}, "Randomise"};
            if (random_away.draw(Fade(DARKGREEN, 0.7f), Fade(LIME, 0.7f), BLACK)) {
                setup.away.value = gaa::RandomCountyName();
            }

            DrawText("Who will you manage?", 120, 340, 22, RAYWHITE);
            Button home_btn{Rectangle{120, 370, 140, 46}, "Home"};
            Button away_btn{Rectangle{280, 370, 140, 46}, "Away"};
            if (home_btn.draw(setup.manage_home ? Fade(LIME, 0.8f) : Fade(DARKGRAY, 0.6f), Fade(LIME, 0.9f), BLACK)) {
                setup.manage_home = true;
            }
            if (away_btn.draw(!setup.manage_home ? Fade(LIME, 0.8f) : Fade(DARKGRAY, 0.6f), Fade(LIME, 0.9f), BLACK)) {
                setup.manage_home = false;
            }

            DrawText("Half length (mins)", 120, 440, 22, RAYWHITE);
            Button minus{Rectangle{120, 470, 60, 46}, "-"};
            Button plus{Rectangle{320, 470, 60, 46}, "+"};
            if (minus.draw(Fade(DARKGRAY, 0.6f), Fade(RAYWHITE, 0.6f), BLACK) && setup.half_minutes > 10) {
                setup.half_minutes -= 1;
            }
            if (plus.draw(Fade(DARKGRAY, 0.6f), Fade(RAYWHITE, 0.6f), BLACK) && setup.half_minutes < 40) {
                setup.half_minutes += 1;
            }
            DrawText(TextFormat("%d", setup.half_minutes), 210, 480, 32, RAYWHITE);

            Button start{Rectangle{120, 550, 220, 56}, "Kick Off"};
            if (start.draw(Fade(GOLD, 0.8f), Fade(YELLOW, 0.9f), BLACK)) {
                std::string home_name = setup.home.value.empty() ? "Dublin" : setup.home.value;
                std::string away_name = setup.away.value.empty() ? "Kerry" : setup.away.value;
                gaa::Team home = gaa::Team::CreateRandom(home_name, rng);
                gaa::Team away = gaa::Team::CreateRandom(away_name, rng);
                gaa::MatchConfig cfg;
                cfg.user_controls_home = setup.manage_home;
                cfg.half_minutes = setup.half_minutes;
                match.simulator = std::make_unique<gaa::MatchSimulator>(std::move(home), std::move(away), cfg, rng());
                match.awaiting_user_choice = false;
                match.halftime_pause = false;
                match.finished = false;
                match.commentary.clear();
                match.halftime_summary.clear();
                match.fulltime_summary.clear();
                match.timer = 0.0f;
                state = ScreenState::Playing;
            }

            DrawText("Tip: you can replay instantly after full-time to try new strategies.", 120, 630, 18, Fade(RAYWHITE, 0.6f));
        } else if (state == ScreenState::Playing || state == ScreenState::Halftime || state == ScreenState::Finished) {
            if (!match.simulator) {
                state = ScreenState::Setup;
            } else {
                const float scoreboard_height = 90.0f;
                DrawPanel(Rectangle{40, 30, GetScreenWidth() - 80.0f, scoreboard_height}, Fade(BLACK, 0.4f));
                const std::string scoreboard_text =
                    gaa::FormatScoreboard(match.simulator->homeTeam(), match.simulator->awayTeam());
                DrawText(scoreboard_text.c_str(), 60, 46, 28, RAYWHITE);
                DrawText(TextFormat("Clock %s", match.simulator->clock().display().c_str()), 60, 76, 20, Fade(RAYWHITE, 0.8f));

                const Rectangle log_bounds{40, 140, GetScreenWidth() - 80.0f, GetScreenHeight() - 220.0f};
                DrawPanel(log_bounds, Fade(BLACK, 0.35f));

                float y = log_bounds.y + 12.0f;
                for (const auto &line : match.commentary) {
                    DrawText(line.c_str(), static_cast<int>(log_bounds.x + 16), static_cast<int>(y), 20, RAYWHITE);
                    y += 26.0f;
                    if (y > log_bounds.y + log_bounds.height - 28.0f) {
                        break;
                    }
                }

                if (state == ScreenState::Playing) {
                    if (!match.simulator->isFinished()) {
                        if (!match.awaiting_user_choice) {
                            match.timer += dt;
                            const float update_interval = 0.65f;
                            if (match.timer >= update_interval) {
                                const gaa::StepResult step = match.simulator->play();
                                if (step.throw_in) {
                                    PushCommentary(match.commentary, FormatThrowIn(*match.simulator, *step.throw_in));
                                }
                                PushCommentary(match.commentary, FormatTimeLine(*match.simulator, step));
                                if (step.possession.show_scoreboard) {
                                    PushCommentary(match.commentary, "Score: " +
                                                                       gaa::FormatScoreboard(match.simulator->homeTeam(),
                                                                                            match.simulator->awayTeam()));
                                }
                                for (auto it = step.possession.extra.rbegin(); it != step.possession.extra.rend(); ++it) {
                                    PushCommentary(match.commentary, *it);
                                }
                                if (step.halftime && !step.fulltime) {
                                    match.halftime_summary = gaa::BuildStatsSummary(match.simulator->homeTeam(), match.simulator->awayTeam());
                                    state = ScreenState::Halftime;
                                }
                                if (step.fulltime) {
                                    match.fulltime_summary = gaa::BuildStatsSummary(match.simulator->homeTeam(), match.simulator->awayTeam());
                                    match.finished = true;
                                    state = ScreenState::Finished;
                                }
                                match.timer = 0.0f;
                                match.awaiting_user_choice = match.simulator->isUserOnBall() && !match.simulator->isFinished();
                            }
                        }

                        if (match.simulator->isUserOnBall() && !match.simulator->isFinished()) {
                            match.awaiting_user_choice = true;
                        }

                        if (match.awaiting_user_choice && state == ScreenState::Playing) {
                            const float panel_height = 140.0f;
                            const Rectangle panel{40, GetScreenHeight() - panel_height - 30.0f, GetScreenWidth() - 80.0f, panel_height};
                            DrawPanel(panel, Fade(BLACK, 0.55f));
                            DrawText("Your possession - pick a tactic", static_cast<int>(panel.x + 20), static_cast<int>(panel.y + 20),
                                     24, YELLOW);
                            std::array<Button, 3> tactic_buttons{
                                Button{Rectangle{panel.x + 20, panel.y + 60, (panel.width - 80) / 3, 56},
                                       gaa::TacticName(gaa::Tactic::GoForGoal)},
                                Button{Rectangle{panel.x + 40 + (panel.width - 80) / 3, panel.y + 60, (panel.width - 80) / 3, 56},
                                       gaa::TacticName(gaa::Tactic::TakeYourPoints)},
                                Button{Rectangle{panel.x + 60 + 2 * (panel.width - 80) / 3, panel.y + 60, (panel.width - 80) / 3, 56},
                                       gaa::TacticName(gaa::Tactic::ControlTempo)},
                            };

                            const std::array<gaa::Tactic, 3> tactics{
                                gaa::Tactic::GoForGoal,
                                gaa::Tactic::TakeYourPoints,
                                gaa::Tactic::ControlTempo,
                            };

                            for (std::size_t i = 0; i < tactic_buttons.size(); ++i) {
                                const std::string detail = gaa::TacticDetail(tactics[i]);
                                const bool clicked = tactic_buttons[i].draw(Fade(DARKGREEN, 0.6f), Fade(LIME, 0.7f), BLACK, 20);
                                DrawText(detail.c_str(), static_cast<int>(tactic_buttons[i].bounds.x),
                                         static_cast<int>(tactic_buttons[i].bounds.y + tactic_buttons[i].bounds.height + 6), 18,
                                         Fade(RAYWHITE, 0.8f));
                                if (clicked) {
                                    const gaa::StepResult step = match.simulator->play(tactics[i]);
                                    if (step.throw_in) {
                                        PushCommentary(match.commentary, FormatThrowIn(*match.simulator, *step.throw_in));
                                    }
                                    PushCommentary(match.commentary, FormatTimeLine(*match.simulator, step));
                                    if (step.possession.show_scoreboard) {
                                        PushCommentary(match.commentary, "Score: " + gaa::FormatScoreboard(match.simulator->homeTeam(),
                                                                                                             match.simulator->awayTeam()));
                                    }
                                    for (auto it = step.possession.extra.rbegin(); it != step.possession.extra.rend(); ++it) {
                                        PushCommentary(match.commentary, *it);
                                    }
                                    if (step.halftime && !step.fulltime) {
                                        match.halftime_summary = gaa::BuildStatsSummary(match.simulator->homeTeam(), match.simulator->awayTeam());
                                        state = ScreenState::Halftime;
                                    }
                                    if (step.fulltime) {
                                        match.fulltime_summary = gaa::BuildStatsSummary(match.simulator->homeTeam(), match.simulator->awayTeam());
                                        match.finished = true;
                                        state = ScreenState::Finished;
                                    }
                                    match.awaiting_user_choice = false;
                                    match.timer = 0.0f;
                                    break;
                                }
                            }
                        }
                    }
                }

                if (state == ScreenState::Halftime) {
                    const Rectangle overlay{GetScreenWidth() * 0.2f, GetScreenHeight() * 0.2f,
                                             GetScreenWidth() * 0.6f, GetScreenHeight() * 0.6f};
                    DrawPanel(overlay, Fade(DARKGREEN, 0.85f));
                    DrawText("Half-time whistle!", static_cast<int>(overlay.x + 40), static_cast<int>(overlay.y + 30), 32, YELLOW);
                    float y_off = overlay.y + 80;
                    for (const auto &line : match.halftime_summary) {
                        DrawText(line.c_str(), static_cast<int>(overlay.x + 40), static_cast<int>(y_off), 20, RAYWHITE);
                        y_off += 28;
                    }
                    Button resume{Rectangle{overlay.x + overlay.width / 2 - 100, overlay.y + overlay.height - 80, 200, 50},
                                  "Second half"};
                    if (resume.draw(Fade(GOLD, 0.85f), Fade(YELLOW, 0.9f), BLACK)) {
                        state = ScreenState::Playing;
                        match.awaiting_user_choice = match.simulator->isUserOnBall();
                        match.timer = 0.0f;
                    }
                }

                if (state == ScreenState::Finished) {
                    const Rectangle overlay{GetScreenWidth() * 0.2f, GetScreenHeight() * 0.18f,
                                             GetScreenWidth() * 0.6f, GetScreenHeight() * 0.64f};
                    DrawPanel(overlay, Fade(DARKGREEN, 0.88f));
                    DrawText("Full-time!", static_cast<int>(overlay.x + 40), static_cast<int>(overlay.y + 30), 36, YELLOW);
                    DrawText(gaa::FormatScoreboard(match.simulator->homeTeam(), match.simulator->awayTeam()).c_str(),
                             static_cast<int>(overlay.x + 40), static_cast<int>(overlay.y + 70), 26, RAYWHITE);
                    float y_off = overlay.y + 120;
                    for (const auto &line : match.fulltime_summary) {
                        DrawText(line.c_str(), static_cast<int>(overlay.x + 40), static_cast<int>(y_off), 20, RAYWHITE);
                        y_off += 28;
                    }
                    Button restart{Rectangle{overlay.x + 40, overlay.y + overlay.height - 80, 200, 50}, "New match"};
                    if (restart.draw(Fade(GOLD, 0.85f), Fade(YELLOW, 0.9f), BLACK)) {
                        state = ScreenState::Setup;
                    }
                    Button quit{Rectangle{overlay.x + overlay.width - 240, overlay.y + overlay.height - 80, 200, 50}, "Quit"};
                    if (quit.draw(Fade(DARKGRAY, 0.7f), Fade(RAYWHITE, 0.8f), BLACK)) {
                        break;
                    }
                }
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

