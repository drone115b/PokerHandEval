#ifndef EVALUATORHEADER
#define EVALUATORHEADER

#include <cstdint>
#include "card.h"

typedef uint32_t HandValue;

//
// To outside entities, the HandValue can appear to be
// an arbitrary integer, used to compare the relative
// strength of hands.
//
// Internally, the lower 20b bits represent the
// "kicker" part.  This can correspond to 5 cards,
// each occupying 4b. (13 ranks fit in ~ 4b).
// The bits above the kicker
// represent the "class" of the hand
// (e.g. straight, flush, 1-pair)
//
// A "hand" can consist of any number of cards,
// the aggregation of the hole cards and any board
// cards.  The evaluator should be able to return the
// greatest hand.
//


class HandEvaluator {
public:

  virtual HandValue
  value( CardSet hand, char *cpDesc ) = 0;

};


class StdHandEvaluator : public HandEvaluator {
public:

  HandValue
  value( CardSet hand, char *cpDesc );

};


bool is_a_straight( HandValue& h, char *cpDesc, const CardSet& );
bool is_a_flush( HandValue& h, char *cpDesc, const CardSet& );
bool is_a_pairing( HandValue& h, char *cpDesc, const CardSet& );
bool is_a_straight_flush( HandValue& h, char *cpDesc, const CardSet& );

#endif
