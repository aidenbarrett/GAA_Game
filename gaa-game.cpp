#include <iostream>
#include <cstdlib>
#include <string>
#include <stdio.h> 
#include <time.h>

/*
Objective:
Simulate a game of GAA, i.e. Donegal vs Derry.

1) Set up teams and assign properties to them.

2) Use a randon number to determine which team will attack.
   Attack until the game is over at 70 minutes.
   Random number between 1 and 5 (minutes) will determine how frequent the attacks will be.

3) Each attack will result in either a Goal/Point/Miss. 
   A goal is worth 3, a point is worth 1, a miss is worth 0. 
   Use a randon number to determine result of the attack.

4) Update Score: Update score after every attack. 

5) Winner: Print out the name of the winning team.
   The team with the most points after 70 minutes is the winner.

// create the class
class Player 
{
  public:
  // Player(const string& new_name, const int& healthX) :
  //   name {new_name},
  //   health {healthX}
  // {
  // }

*/
using namespace std;

class Game { 

public:
	// Team teamObj = Team( TeamName, TeamLevel = 1); 
  Game(const string& TeamName, const int& TeamLevel = 1, const int& TeamScore = 0, const int& Attack = 0) 
  : teamName (TeamName), teamLevel (TeamLevel), teamScore (TeamScore), attack (Attack) { }

  // Class Methods
  void TeamInfo(Game& teamInstance1, Game& teamInstance2); 

  void Attack(); 
  void AttackResult();
  void UpdateScore();

  void DisplayScore(Game& teamInstance1, Game& teamInstance2);

  void GameStart(Game& teamInstance2);
  void CoinToss(Game& teamInstance1, Game& teamInstance2);

  void GameOver(Game& teamInstance1, Game& teamInstance2);
  void GetStats(Game& teamInstance1, Game& teamInstance2);


  // Setter Functions
  void setTeamName(string tn);
  void setTeamLevel(int tl);
  void setTeamScore(int ts);
  void setTeamAttack(int ta);
  

  // Getter Functions
  string getTeamName();
  int getTeamLevel();
  int getTeamScore();
  int getTeamAttack();

private:
	string teamName;
  int teamLevel; // think of a way that this would be useful
  int teamScore;
  int attack;
};

void Game::TeamInfo(Game& teamInstance1, Game& teamInstance2) 
{
	cout << "\nTeams:  " << this->getTeamName() << "    vs     " << teamInstance2.getTeamName() << endl;
  cout << "Score:  " << this->getTeamScore() << "                 " << teamInstance2.getTeamScore() << endl;
}

void Game::Attack() 
{
  // cout << "\nWell hello there... I'm Attack()" << endl;
  AttackResult();
  // UpdateScore();

  // TODO: Maybe add further capability here or merge with AttackResult()
  // Maybe use TeamLevel value to have an effect on the AttackResult.
  // Higher TeamLevel results in better chance of scoring.
  // Goal results in small increase of TeamLevel.
  // Missed shots result in TeamLevel decrementing.
}

void Game::AttackResult()
{
  // cout << "\nHello there... I'm AttackResult()" << endl;

  // Generate a random number between 1 and 3
  int randomNumber = 1 * ((rand() % 3) + 1);
  // cout << "\nRandom Number from AttackResult: " << randomNumber2 << endl;

  if (randomNumber == 1 || randomNumber == 3) // maybe think about how you would change that here
    this->setTeamAttack(randomNumber);
  // Assign the random number to getTeamAttack()
  else if (randomNumber == 2)
  { 
    randomNumber = 0;
    this->setTeamAttack(randomNumber);
    //cout << "\nMiss " << this->getTeamAttack();
  } 
  // cout << "\nNEW_setTeamAttack:  " << this->getTeamAttack() << endl;
  UpdateScore();
}

// Update Score
// -------------------------------
void Game::UpdateScore() // will be used to update the level as well based on the action (goal, miss)
{
  // cout << "\nWell hello there... I'm UpdateScore()" << endl;
  int tempValue = 0;
  tempValue = (this->getTeamScore()) + (this->getTeamAttack());
  this->setTeamScore(tempValue);
  // cout << "Scores in Game: " << getTeamAttack() << endl;;
}

// Display Score 
// -------------------------------
void Game::DisplayScore(Game& teamInstance1, Game& teamInstance2) 
{
	cout << "Score:  " << this->getTeamScore() << "                 " << teamInstance2.getTeamScore() << endl;
  //TODO: Seperate score into goals and points
}

// Game Start
// -------------------------------
void Game::GameStart(Game& teamInstance2) 
{
  cout << "\n-------------------------------" << endl;
  cout << "---------Begin Match-----------" << endl;
  cout << "-------------------------------" << endl;

  TeamInfo(*this, teamInstance2); // prints the initial team stats

	for (int gameTime = 0; gameTime <= 70; ++gameTime)
	{
		CoinToss(*this, teamInstance2); // either teaminstance 1 attacked or teaminstance 2 attacked

		int temp = 1 * ((rand() % 5) + 1);
		gameTime = gameTime + temp;
		// cout << "GameTime: " << gameTime << endl;
	}

	this->setTeamScore(this->getTeamScore()); // Updates the team score
	teamInstance2.setTeamScore(teamInstance2.getTeamScore());

  cout << "\n\n-------------------------------" << endl;
  cout << "---------End of Match----------" << endl;
  cout << "-------------------------------" << endl;
}

// CoinToss Generator
// -------------------------------
void Game::CoinToss(Game& teamInstance1, Game& teamInstance2) 
{
	int coinToss = 1 * ((rand() % 2) + 1);

	if (coinToss == 1)
	{
		// teamInstance1.TeamInfo();
		teamInstance1.Attack();
	}
	else if (coinToss == 2)
	{
		//  teamInstance2.TeamInfo();
		teamInstance2.Attack();
	}
}

// Game Over Print Info
// -------------------------------
void Game::GameOver(Game& teamInstance1, Game& teamInstance2)
{
  TeamInfo(*this, teamInstance2); // prints the final team stats
  GetStats(teamInstance1, teamInstance2);

}

void Game::GetStats(Game& teamInstance1, Game& teamInstance2)
{
  // Print result of game
    if( teamInstance1.getTeamScore() > teamInstance2.getTeamScore() )
  {
    cout << "\n\n-------------------------------" << endl;
    cout << "---------Winner: " << teamInstance1.getTeamName() << "-------" << endl;
    cout << "-------------------------------" << endl;
  }
  else if ( teamInstance2.getTeamScore() > teamInstance1.getTeamScore() )
  {
    cout << "\n\n-------------------------------" << endl;
    cout << "---------Winner: " << teamInstance2.getTeamName() << "---------" << endl;
    cout << "-------------------------------" << endl;
  }
  else
  {
    cout << "\n\n-------------------------------" << endl;
    cout << "-------------Draw--------------" << endl;
    cout << "-------------------------------" << endl;
  }

  // // Function to call match stats - begin match, half-time, full-time.
  // cout << "-------------------------------" << endl;
  // cout << "----------Half Time------------" << endl;
  // cout << "-------------------------------" << endl;
}

// Setter Methods
// -------------------------------

// setTeamName()
void Game::setTeamName(string tn)
{
  this->teamName = tn;
}
// setTeamLevel()
void Game::setTeamLevel(int tl)
{
  this->teamLevel = tl;
}
// setTeamScore()
void Game::setTeamScore(int ts)
{
  this->teamScore = ts;
}
// setTeamAttack()
void Game::setTeamAttack(int ta)
{
  this->attack = ta;
}

// Getter Methods
// -------------------------------

// getTeamName()
string Game::getTeamName()
{
  return this->teamName;
}
// getTeamLevel()
int Game::getTeamLevel()
{
  return this->teamLevel;
}
// getTeamScore()
int Game::getTeamScore()
{
  return this->teamScore;
}
// getTeamAttack()
int Game::getTeamAttack()
{
  return this->attack;
}

// Main()
// -------------------------------
int main() {
	srand(time(NULL)); // For random number generation
  
  // Create an object called teamInstance1 of class Game and assign a name i.e Donegal
	Game teamInstance1 = Game( "Donegal");  
	Game teamInstance2 = Game( "Derry");

// Start game and determine attack precedence, end after 70 minutes
  teamInstance1.GameStart(teamInstance2); 
  teamInstance1.GameOver(teamInstance1, teamInstance2); 
// Display the winner of the game
}
