// g++ -std=c++17 *.cpp

#include "card.h"
#include "evaluator.h"

#include <random>
#include <cstdint>
#include <string>
#include <iostream>
#include <chrono>  // for high_resolution_clock

const unsigned NumShowHands = 4096;
const unsigned NumTimeHands = 1024*1024;


void time_hands(StdHandEvaluator& heval, unsigned howmany) {
  CardSet *cards = new CardSet[NumTimeHands];
  std::uniform_int_distribution<uint8_t> picker(0,51);
  std::default_random_engine generator (1492);

  CardSet c, picked;

  // precalculate hands
  for(int i=0; i<NumTimeHands; ++i){
    cards[i] = 0;
    do {
      picked = 1ULL << picker(generator);
      cards[i] |= picked;
    } while(count_cards(cards[i]) < howmany);
  }


  // Record start time
  auto start = std::chrono::high_resolution_clock::now();
  HandValue h;
  for(int i=0; i<NumTimeHands; ++i )
    h ^= heval.value(cards[i], 0);
  // Record end time
  auto finish = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = finish - start;
  std::cout << "Noise: " << h << "\r";
  std::cout << "Elapsed time for " << howmany << " card hands: " << elapsed.count() << " sec\n";
  std::cout << (double) NumTimeHands / (double) elapsed.count() << " hands/sec\n\n";

  delete[] cards;
  return;
}



void show_hands(StdHandEvaluator& heval) {
  char cpBuffer[32];
  std::string s;
  std::uniform_int_distribution<uint8_t> picker(0,51);
  std::default_random_engine generator (1776);

  CardSet c, picked;

  for(int i=0; i<NumShowHands; ++i){
    c = 0;

    uint8_t how_many = 9;
    while( how_many-- ){
        picked = 1ULL << picker(generator);
        c |= picked;
    }

    print_hand(s, c);
    HandValue h = heval.value(c, cpBuffer);

    std::cout << '(' << i << ")  " << s << "  [" << cpBuffer << ']' << '\t' << uint32_t(h) << std::endl;
  }

  return;
}


int main(int argc, char* argv[]) {
  StdHandEvaluator heval;

  time_hands(heval, 5);
  time_hands(heval, 6);
  time_hands(heval, 7);
  show_hands(heval);

  return 0;
}
