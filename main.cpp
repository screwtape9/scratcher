#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <array>
#include <algorithm>
#include <cassert>
#include <unistd.h>
#include <rapidjson/document.h>
#include "cget.h"
#include "game.h"

extern const char *__progname;

static bool GamePercentClaimedComp(Game a, Game b)
{
  return (a.GetPercentClaimed() < b.GetPercentClaimed());
}

static void CalcColWidths(std::vector<Game> const& games,
                          std::array<std::size_t, 5>& colWidths)
{
  for (auto& g : games) {
    if (colWidths[0] < g.getTktPriceStr().length())
      colWidths[0] = g.getTktPriceStr().length();

    if (colWidths[1] < g.getTopPrizeStr().length())
      colWidths[1] = g.getTopPrizeStr().length();

    std::stringstream ss;
    ss << g.getTotalGames();
    if (colWidths[2] < ss.str().length())
      colWidths[2] = ss.str().length();

    if (colWidths[4] < g.getName().length())
      colWidths[4] = g.getName().length();
  }
}

static void DisplayTable(std::vector<Game> const& games,
                         std::array<std::string, 5> const& colHeaders,
                         std::array<std::size_t, 5>& colWidths,
                         const uint32_t optTktPrice,
                         const uint32_t optMaxClaimedPercent)
{
  std::cout << std::right << std::setw(colWidths[0]) << colHeaders[0] << ' '
            << std::right << std::setw(colWidths[1]) << colHeaders[1] << ' '
            << std::right << std::setw(colWidths[2]) << colHeaders[2] << ' '
            << std::right << std::setw(colWidths[3]) << colHeaders[3] << ' '
            << colHeaders[4]  << std::endl;

  // because we're going to add a '%' to this number, decrement 1 col width
  colWidths[3]--;

  for (auto& g : games) {
    if (optTktPrice && (optTktPrice != g.getTktPrice()))
      continue;
    if (optMaxClaimedPercent && (g.GetPercentClaimed() > optMaxClaimedPercent))
      continue;
    std::cout << std::right << std::setw(colWidths[0]) << g.getTktPriceStr() << ' '
              << std::right << std::setw(colWidths[1]) << g.getTopPrizeStr() << ' '
              << std::right << std::setw(colWidths[2]) << g.getTotalGames() << ' '
              << std::right << std::setw(colWidths[3]) << g.GetPercentClaimed() << '%' << ' '
              << g.getName() << std::endl;
  }
}

static void usage()
{
  std::cout << "Usage: "  << __progname                                               << std::endl
            << "Options:"                                                             << std::endl
            << "  -h                Display this information."                        << std::endl
            << "  -p <tkt price>    Only list games of this ticket price (dollars)."  << std::endl
            << "  -c <claimed %>    Only list games with claimed % less than this."   << std::endl;
}

static void GetOpts(int argc,
                    char *argv[],
                    uint32_t& optTktPrice,
                    uint32_t& optMaxClaimedPercent)
{
  int opt = 0;
  while ((opt = getopt(argc, argv, "hc:p:")) != -1) {
    switch (opt) {
    case 'c':
      errno = 0;
      optMaxClaimedPercent = strtoul(optarg, nullptr, 10);
      if (errno) {
        std::cerr << "Invalid claimed %." << std::endl;
        _exit(1);
      }
      break;
    case 'h':
      usage();
      _exit(0);
      break;
    case 'p':
      errno = 0;
      optTktPrice = strtoul(optarg, nullptr, 10);
      if (errno) {
        std::cerr << "Invalid tkt price." << std::endl;
        _exit(1);
      }
      break;
    default:
      usage();
      _exit(1);
      break;
    }
  }
}

int main(int argc, char *argv[])
{
  uint32_t optTktPrice = 0, optMaxClaimedPercent = 0;
  GetOpts(argc, argv, optTktPrice, optMaxClaimedPercent);

  const std::string strURLTopPrizes("https://www.galottery.com/en-us/games/scratchers/scratchers-top-prizes-claimed.html");
  //const std::string strURLActiveGames("https://www.galottery.com/en-us/games/scratchers/active-games.html");

  CGET cget;
  if (cget.GetFile(strURLTopPrizes.c_str(), "prizes.html", "cget.log")) {
    std::ifstream fin("prizes.html");
    for (std::string line; std::getline(fin, line);) {
      if (line.find("topPrizListArray") != std::string::npos) {
        std::size_t i = line.find('[');
        if (i != std::string::npos) {
          line.erase(0, i);
          line.pop_back();

          line.insert(0, "{ \"GAMES\" : ");
          line += " } ";
          
          rapidjson::Document doc;
          doc.Parse(line.c_str());
          
          const rapidjson::Value& a = doc["GAMES"];
          assert(a.IsArray());
          std::vector<Game> games;
          for (auto& v : a.GetArray())
            games.push_back(Game(v));

          const std::array<std::string, 5> colHeaders = {
            "Tkt Price", "Top Prize", "Total Games", "% claimed", "Title"
          };
          std::array<std::size_t, 5> colWidths = {
            colHeaders[0].length(),
            colHeaders[1].length(),
            colHeaders[2].length(),
            colHeaders[3].length(),
            colHeaders[4].length()
          };
          CalcColWidths(games, colWidths);

          std::sort(games.begin(), games.end(), GamePercentClaimedComp);
          DisplayTable(games, colHeaders, colWidths, optTktPrice, optMaxClaimedPercent);
        }
        break;
      }
    }
  }
  else
    std::cout << cget.GetLastError() << std::endl;
  return EXIT_SUCCESS;
}
