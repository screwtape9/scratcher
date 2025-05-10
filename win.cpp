#include <fstream>
#include <sstream>
#include <rapidjson/document.h>
#include "win.h"
#include "cget.h"

MainWin::MainWin(uint32_t _tktPrice, uint32_t _maxClaimedPercent)
  : mainBox(Gtk::ORIENTATION_VERTICAL, 8)
  , tktPriceBox(Gtk::ORIENTATION_HORIZONTAL, 8)
  , tktPriceCheckBtn("Ticket price (dollars)")
  , claimedPctBox(Gtk::ORIENTATION_HORIZONTAL, 8)
  , claimedPctCheckBtn("Max claimed %")
  , quitBtn("Quit")
  , cgetFailed(false)
  , tktPrice(_tktPrice)
  , maxClaimedPercent(_maxClaimedPercent)
{
  set_title("Georgia Lotto Scratchers");
  set_border_width(8);
  set_default_size(280, 250);

  // create the tkt price filter box
  tktPriceBox.pack_start(tktPriceCheckBtn, Gtk::PACK_SHRINK);
  tktPriceBox.pack_start(tktPriceEntry, Gtk::PACK_SHRINK);

  // create the max claimed percentage filter box
  claimedPctBox.pack_start(claimedPctCheckBtn, Gtk::PACK_SHRINK);
  claimedPctBox.pack_start(claimedPctEntry, Gtk::PACK_SHRINK);

  // put those two boxes in the filters box
  filtersBox.set_border_width(5);
  filtersBox.set_spacing(60);
  filtersBox.pack_start(tktPriceBox, Gtk::PACK_SHRINK);
  filtersBox.pack_start(claimedPctBox, Gtk::PACK_SHRINK);

  // get the games and populate the treeview
  create_model();
  treeView.set_model(listStore);
  treeView.set_search_column(treeColumns.gameName.index());
  add_columns();

  // add the treeview to a scrolled window
  scrolledWin.set_shadow_type(Gtk::SHADOW_ETCHED_IN);
  scrolledWin.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
  scrolledWin.add(treeView);

  // create the quit button box
  quitBtnBox.pack_start(quitBtn, Gtk::PACK_SHRINK);
  quitBtnBox.set_border_width(5);
  quitBtnBox.set_layout(Gtk::BUTTONBOX_END);

  // add all those boxes to the main box
  mainBox.pack_start(filtersBox, Gtk::PACK_SHRINK);
  mainBox.pack_start(scrolledWin);
  mainBox.pack_start(quitBtnBox, Gtk::PACK_SHRINK);

  // and finally add it to the main window
  add(mainBox);

  // if we were passed in a tkt price or max claimed %, act accordingly
  if (tktPrice) {
    Glib::ustring txt = Glib::ustring::compose("%1", tktPrice);
    tktPriceEntry.get_buffer()->set_text(txt);
    tktPriceCheckBtn.set_active();
  }
  else
    tktPriceEntry.set_sensitive(false);

  if (maxClaimedPercent) {
    Glib::ustring txt = Glib::ustring::compose("%1", maxClaimedPercent);
    claimedPctEntry.get_buffer()->set_text(txt);
    claimedPctCheckBtn.set_active();
  }
  else
    claimedPctEntry.set_sensitive(false);

  // fancy tooltips
  tktPriceBox.set_tooltip_text("Display games with this price.");
  claimedPctBox.set_tooltip_text("Display games with this (or less) claimed percentage.");

  // don't draw focus rect around them when clicked
  tktPriceCheckBtn.set_can_focus(false);
  claimedPctCheckBtn.set_can_focus(false);

  // connect signals
  tktPriceCheckBtn.signal_toggled().connect(sigc::mem_fun(*this,
                                                          &MainWin::on_cbtn_tkt_price));
  claimedPctCheckBtn.signal_toggled().connect(sigc::mem_fun(*this,
                                                            &MainWin::on_cbtn_max_claim_pct));
  quitBtn.signal_clicked().connect(sigc::mem_fun(*this,
                                                 &MainWin::on_button_quit));
  tktPriceEntry.signal_activate().connect(sigc::mem_fun(*this,
                                                        &MainWin::on_tkt_price_activated));
  tktPriceEntry.signal_focus_out_event().connect(sigc::mem_fun(*this,
                                                               &MainWin::on_tkt_price_focus_out));
  claimedPctEntry.signal_activate().connect(sigc::mem_fun(*this,
                                                          &MainWin::on_max_claim_pct_activated));
  claimedPctEntry.signal_focus_out_event().connect(sigc::mem_fun(*this,
                                                                 &MainWin::on_max_claim_pct_focus_out));
  // voila!
  show_all();
}

void MainWin::on_show()
{
  Gtk::Widget::on_show();
  if (cgetFailed) {
    Glib::ustring msg = "Failed to download data.";
    Glib::ustring log = "cget.log:\n";
    std::ifstream fin("cget.log");
    if (fin)
      for (std::string line; std::getline(fin, line);) {
        log += line;
        log += '\n';
      }
    Gtk::MessageDialog dlg(*this, msg, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
    //dlg.set_transient_for(*this);
    //dlg.property_window_position() = Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT;
    dlg.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dlg.set_secondary_text(log);
    dlg.run();
  }
}

void MainWin::on_button_quit()
{
  hide();
}

void MainWin::on_cbtn_tkt_price()
{
  tktPriceEntry.set_sensitive(tktPriceCheckBtn.get_active());
  if (tktPriceCheckBtn.get_active()) {
    Glib::ustring txt = tktPriceEntry.get_buffer()->get_text();
    std::stringstream ss;
    ss << txt.raw();
    ss >> tktPrice;
  }
  else
    tktPrice = 0;

  listStore->clear();
  std::for_each(games.begin(),
                games.end(),
                sigc::mem_fun(*this, &MainWin::liststore_add_item));
}

void MainWin::on_cbtn_max_claim_pct()
{
  claimedPctEntry.set_sensitive(claimedPctCheckBtn.get_active());
  if (claimedPctCheckBtn.get_active()) {
    Glib::ustring txt = claimedPctEntry.get_buffer()->get_text();
    std::stringstream ss;
    ss << txt.raw();
    ss >> maxClaimedPercent;
  }
  else
    maxClaimedPercent = 0;

  listStore->clear();
  std::for_each(games.begin(),
                games.end(),
                sigc::mem_fun(*this, &MainWin::liststore_add_item));
}

void MainWin::on_tkt_price_activated()
{
  on_cbtn_tkt_price();
}

bool MainWin::on_tkt_price_focus_out(GdkEventFocus* gdk_event)
{
  on_cbtn_tkt_price();
  return false; // to progagate the event onward
}

void MainWin::on_max_claim_pct_activated()
{
  on_cbtn_max_claim_pct();
}

bool MainWin::on_max_claim_pct_focus_out(GdkEventFocus* gdk_event)
{
  on_cbtn_max_claim_pct();
  return false; // to progagate the event onward
}

static bool GamePercentClaimedComp(Game a, Game b)
{
  return (a.GetPercentClaimed() < b.GetPercentClaimed());
}

void MainWin::fetch_games()
{
  const std::string strURLTopPrizes("https://www.galottery.com/en-us/games/scratchers/scratchers-top-prizes-claimed.html");
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
          for (auto& v : a.GetArray())
            games.push_back(Game(v));
          std::sort(games.begin(), games.end(), GamePercentClaimedComp);
        }
      }
    }
  }
  else
    cgetFailed = true;
}

void MainWin::create_model()
{
  listStore = Gtk::ListStore::create(treeColumns);
  fetch_games();
  std::for_each(games.begin(),
                games.end(),
                sigc::mem_fun(*this, &MainWin::liststore_add_item));
}

void MainWin::liststore_add_item(const Game& g)
{
  if ((tktPrice && (tktPrice != g.getTktPrice())) ||
      (maxClaimedPercent && (g.GetPercentClaimed() > maxClaimedPercent)))
    return;

  Gtk::TreeRow row = *(listStore->append());

  row[treeColumns.tktPrice]       = g.getTktPriceStr();
  row[treeColumns.topPrize]       = g.getTopPrizeStr();
  row[treeColumns.totalGames]     = g.getTotalGames();
  row[treeColumns.percentClaimed] = g.GetPercentClaimed();
  row[treeColumns.gameName]       = g.getName();
}

void MainWin::add_columns()
{
  treeView.append_column("Tkt Price",   treeColumns.tktPrice);
  treeView.append_column("Top Prize",   treeColumns.topPrize);
  treeView.append_column("Total Games", treeColumns.totalGames);
  treeView.append_column("% Claimed",   treeColumns.percentClaimed);
  treeView.append_column("Title",       treeColumns.gameName);
}
