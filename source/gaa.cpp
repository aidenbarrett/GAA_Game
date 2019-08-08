#include <iostream>
#include <math.h>
#include <string.h>
using namespace std;

/*

 - Game Structure/Design

Time Class - Controls the game time and duration
  GenerateMatchClock()
  StartTime()
  HalfTime()
  Full-Time()
  Extra-Time()
  PenaltyTime()

Game Class - Starts match competition

  Team Class -
    TeamStructure()
    TeamInformation()
    TeamQuality()

    Player Class -
      NumberOfPlayers()
      PlayerNames()
      PlayerHealth()
      PlayerFitness()

    Level Class -
      PlayerXP()
      PlayerLevel()

  Attack Class -
    Attack();
    AttackResult()

  Score Class -
    DisplayScore()
    UpdateScore()

  Stats Class -
    RecordTeamGoals()
    RecordTeamPoints()
    RecordTeamMisses()
    RecordTeamFouls()

      RecordPlayerGoals()
      RecordPlayerPoints()
      RecordPlayerMisses()
      RecordPlayerFouls()

  Reports Class -
    DisplayPlayerFouls()
    DisplayPlayerInjuries()
    DisplayPlayerScoreStreaks()

  Results Class -
    HalfTimeResult()
    FullTimeResult()

*/

class Time
{
  public:
  void GenerateMatchClock();
  void StartTime();
  void HalfTime();
  void FullTime();
  void ExtraTime();
  void PenaltyTime();
  void GeneratePrimeNumber(int upperbound);

  private:
};

class Game // Controller Class
{
  public:

  private:
};

class Team : public Game
{
  public:
  void TeamStructure();
  void TeamInformation();
  void TeamQuality();

  private:
};

class Player : public Team
{
  public:

  void NumberOfPlayers();
  void PlayerNames();
  void PlayerHealth();

  private:
};

class Level : Team
{
  public:

  void PlayerXP();
  void PlayerLevel();

  private:
};

class Attack : public Game
{
  public:

  void AttackNow();
  void AttackResult();

  private:
};

class Score : public Game
{
  public:

  void DisplayScore();
  void UpdateScore();

  private:
};

class Stats : public Game
{
  public:

  void RecordTeamGoals();
  void RecordTeamPoints();
  void RecordTeamMisses();
  void RecordTeamFouls();

    void RecordPlayerGoals();
    void RecordPlayerPoints();
    void RecordPlayerMisses();
    void RecordPlayerFouls();

  private:
};

class Reports : public Game
{
  public:

  void DisplayPlayerFouls();
  void DisplayPlayerInjuries();
  void DisplayPlayerScoreStreaks();

  private:
};

class Results : public Game
{
  public:

  void HalfTimeResult();
  void FullTimeResult();

  private:
};


int main()
{


}
