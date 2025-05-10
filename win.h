#ifndef __SCRATCHERS_MAIN_WIN_H_
#define __SCRATCHERS_MAIN_WIN_H_

#include <gtkmm.h>
#include "game.h"

class MainWin : public Gtk::Window
{
public:
  MainWin(uint32_t _tktPrice = 0, uint32_t _maxClaimedPercent = 0);
  virtual ~MainWin() { }

protected:
  void on_show();
  void on_button_quit();
  void on_cbtn_tkt_price();
  void on_cbtn_max_claim_pct();
  void on_tkt_price_activated();
  void on_max_claim_pct_activated();
  bool on_tkt_price_focus_out(GdkEventFocus* gdk_event);
  bool on_max_claim_pct_focus_out(GdkEventFocus* gdk_event);
  void fetch_games();
  void create_model();
  void liststore_add_item(const Game& g);
  void add_columns();

  Gtk::Box mainBox;
  Gtk::Box filtersBox;
  Gtk::Box tktPriceBox;
  Gtk::CheckButton tktPriceCheckBtn;
  Gtk::Entry tktPriceEntry;
  Gtk::Box claimedPctBox;
  Gtk::CheckButton claimedPctCheckBtn;
  Gtk::Entry claimedPctEntry;
  Gtk::ScrolledWindow scrolledWin;
  Gtk::TreeView treeView;
  Glib::RefPtr<Gtk::ListStore> listStore;
  Gtk::ButtonBox quitBtnBox;
  Gtk::Button quitBtn;

  GameList games;
  bool cgetFailed;
  uint32_t tktPrice;
  uint32_t maxClaimedPercent;

  struct GameColumnRecord : public Gtk::TreeModelColumnRecord
  {
    Gtk::TreeModelColumn<Glib::ustring> tktPrice;
    Gtk::TreeModelColumn<Glib::ustring> topPrize;
    Gtk::TreeModelColumn<unsigned int>  totalGames;
    Gtk::TreeModelColumn<unsigned int>  percentClaimed;
    Gtk::TreeModelColumn<Glib::ustring> gameName;

    GameColumnRecord()
    {
      add(tktPrice);
      add(topPrize);
      add(totalGames);
      add(percentClaimed);
      add(gameName);
    }
  };

  const GameColumnRecord treeColumns;
};

#endif // __SCRATCHERS_MAIN_WIN_H_
