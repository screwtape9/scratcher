#include <iostream>
#include <algorithm>
#include <cassert>
#include "game.h"

Game::Game()
  : ID(0)
  , name("")
  , tktPrice(0)
  , strTktPrice("")
  , topPrize(0)
  , strTopPrize("")
  , numClaimed(0)
  , totalGames(0)
  , status(false)
{
}

static void StringToInt(uint32_t& n,
                        std::string const& s,
                        std::string const& lbl)
{
  errno = 0;
  n = strtoul(s.c_str(), nullptr, 10);
  if (errno) {
    std::cerr << "strtoul failed on '" << lbl << "': "
              << std::strerror(errno)  << std::endl;
    assert(false);
  }
}

static void DollarsToInt(uint32_t& n,
                         std::string& s,
                         std::string const& lbl)
{
  if (s.empty()) {
    std::cerr << "DollarsToInt failed on '" << lbl << "': empty input."
              << std::endl;
    assert(false);
  }

  // if exists, remove leading '$'
  if (s[0] == '$')
    s.erase(s.begin());

  if (s.empty()) {
    std::cerr << "DollarsToInt failed on '" << lbl
              << "': unknown or unexpected input format." << std::endl;
    assert(false);
  }

  // remove any commas
  s.erase(std::remove(s.begin(), s.end(), ','), s.end());

  if (s.empty()) {
    std::cerr << "DollarsToInt failed on '" << lbl
              << "': unknown or unexpected input format." << std::endl;
    assert(false);
  }

  // if exists, lop off trailing decimal and pennies
  std::size_t i = s.find('.');
  if (i != std::string::npos)
    s.erase(i);

  // we should have a whole number at this point
  // NOTE: sometimes the top prize is something like:
  //       $500 a week for life
  //       and that will get converted to the money amt, i.e. 500
  StringToInt(n, s, lbl);
}

Game::Game(rapidjson::Value const& jsonVal)
{
  for (rapidjson::Value::ConstMemberIterator it = jsonVal.MemberBegin();
       it != jsonVal.MemberEnd();
       ++it) {
    std::string memberName(it->name.GetString());
    if (it->value.IsString()) {
      std::string memberValue(it->value.GetString());
      /*
      {
        "gameId"      : "1698",
        "gameName"    : "Show Me $300,000",
        "ticketPrice" : "$5",
        "topPrize"    : "$300,000",
        "claimed"     : "2",
        "total"       : "4",
        "status"      : ["true"]
      }
      */
      if (memberName == "gameId")
        StringToInt(ID, memberValue, memberName);
      else if (memberName == "gameName")
        name = memberValue;
      else if (memberName == "ticketPrice") {
        strTktPrice = memberValue;
        DollarsToInt(tktPrice, memberValue, memberName);
      }
      else if (memberName == "topPrize") {
        strTopPrize= memberValue;
        DollarsToInt(topPrize, memberValue, memberName);
      }
      else if (memberName == "claimed")
        StringToInt(numClaimed, memberValue, memberName);
      else if (memberName == "total")
        StringToInt(totalGames, memberValue, memberName);
      else {
        std::cerr << "Unexpected or unknown game json value." << std::endl;
        assert(false);
      }
    }
    else if (it->value.IsBool()) {
      if (memberName == "status")
        status = it->value.IsTrue();
      else {
        std::cerr << "Unexpected or unknown game json value." << std::endl;
        assert(false);
      }
    }
  }
}

Game& Game::operator=(Game const& g)
{
  ID          = g.ID;
  name        = g.name;
  tktPrice    = g.tktPrice;
  strTktPrice = g.strTktPrice;
  topPrize    = g.topPrize;
  strTopPrize = g.strTopPrize;
  numClaimed  = g.numClaimed;
  totalGames  = g.totalGames;
  status      = g.status;
  return (*this);
}
