#ifndef __SCRATCHER_GAME_H_
#define __SCRATCHER_GAME_H_

#include <cstdint>
#include <string>
#include <rapidjson/document.h>

class Game
{
public:
  Game();
  Game(Game const& g) { (*this) = g; }
  Game(rapidjson::Value const& jsonVal);
  virtual ~Game() { }
  Game& operator=(Game const& g);

  uint32_t getID() const              { return ID;          }
  std::string getName() const         { return name;        }
  uint32_t getTktPrice() const        { return tktPrice;    }
  std::string getTktPriceStr() const  { return strTktPrice; }
  uint32_t getTopPrize() const        { return topPrize;    }
  std::string getTopPrizeStr() const  { return strTopPrize; }
  uint32_t getNumClaimed() const      { return numClaimed;  }
  uint32_t getTotalGames() const      { return totalGames;  }

  uint32_t GetPercentClaimed() const
  {
    return uint32_t(((double)numClaimed / (double)totalGames) * 100.00);
  }
protected:
  uint32_t    ID;
  std::string name;
  uint32_t    tktPrice;
  std::string strTktPrice;
  uint32_t    topPrize;
  std::string strTopPrize;
  uint32_t    numClaimed;
  uint32_t    totalGames;
  bool        status;
};

#endif // __SCRATCHER_GAME_H_
