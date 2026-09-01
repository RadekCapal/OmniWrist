#ifndef UI_CARD_H
#define UI_CARD_H

#include <TFT_eSPI.h>

// abstract base class for all smartwatch screens (cards)
class UI_Card {
public:
  // virtual destructor is required for proper memory cleanup in c++
  virtual ~UI_Card() = default;

  // called exactly once when the card slides into view.
  // draw static backgrounds, labels and static lines here to prevent
  // flickering.
  virtual void onShow(TFT_eSPI *tft) = 0;

  // called repeatedly in the main loop.
  // update only the dynamic parts here (numbers, moving graphs).
  virtual void onUpdate(TFT_eSPI *tft) = 0;

  // called when the user swipes away to another card.
  // perfect place to put sensors to sleep and save battery!
  virtual void onHide() {
    // default empty implementation. cards can override this if they need to.
  }

  // returns true if the card is doing something important and should not be put
  // to sleep
  virtual bool blocksSleep() { return false; }
};

#endif // UI_CARD_H
