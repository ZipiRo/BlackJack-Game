#pragma once

/*
    BLACK JACK

    A CARD GAME WHERE YOU GET CARDS UNTIL YOU HIT 21 TO WIN

    HOW THE GAME WORKS:
    THERE IS A DEALER AND A PLAYER
    YOU PLACE A BET ON ONE HAND
    THEN THE DEALER IS GOING TO GIVE CARDS FACING UP, 2 FOR YOU AND 2 FOR HIM, HIS SECOND CARD IS FACING DOWN
    THEN YOU CHOSE FROM FOR ACTIONS IF VALID
        HIT: GET ONE CARD
        STAND: YOU STAY WITH YOUR CARDS
        DOUBLE: YOU DOUBLE DOWN YOUR BET AND STAND
        SPLIT: YOU ARE GOING TO SPLIT THE CARDS IN TWO HANDS THAT ARE THE SAME BET AMOUNT, THEN THE DEALER ADDS A CARD TO EACH HAND
            .YOU CAN SPLIT JUST ONE TIME
    AFTER YOU STAND THE DEALER TURN STARTS
    THE DEALER IS GOING TO HIT UNTIL HIS HAND POWER IS >= 16 AFTER HE IS GOING TO STAND
    WHEN THE DEALER FINNISHES THERE ARE 4 OUTCOMES:
        WIN: YOU HAD A HAND POWER <= 21 AND THE DEALER HAND POWER > 21 OR SMALLER THEN YOUR HAND
            .IF THIS HAPPENDS YOU GET BET AMOUNT
        LOST: YOU HAD A HAND POWER THAT IS < 21 AND THE DEALER WINNED
        BUST: YOU HAD A HAND POWER > 21
        BLACK-JACK: IS WHEN FROM THE START YOU GET A ACE AND A 10 POWER CARD
            .IF THIS HAPPENDS YOU GET BET AMOUNT * 1.5
        PUSH: WHEN THE DEALER AND PLAYER HAVE THE SAME RESULTS
            .THE PLAYER GETS HIS BET BACK

    -"HAND POWER" IS THE ADDED POWER OF THE CARDS IN A HAND 
    -"HAND" IS THE PLACE WHERE YOU HOLD YOUR CARDS AND THE BET AMOUNT,
        THERE ARE JUST TWO HANDS POSIBLE THE START HAND AND THE SPLITTED ONE 
    
    -------------------------------------------------------------------
        IF YOU WANT TO USE FETCH GAME INFORMATION USE:
            #define USE_GET_INFO

        YOU CAN GET THE GAME STATE INFO, PLAYER INFO, ETC.
        YOU CAN USE THE RESULT INFO THAT IS CREATED AT THE RESULT STATE 
        
        USING THAT YOU CAN MAKE UI FOR THE GAME, OR USE THE DATA IN ANY WAY YOU WANT
    -------------------------------------------------------------------

    BY ZIPIRO
*/

#include <vector>
#include <string>

#define ERR_HAND_NOT_FOUND 100
#define ERR_NO_CARDS_AT_IDX 101
#define ERR_NO_CARDS_IN_DECK 102

#define HIT 0
#define STAND 1
#define DOUBLE 2
#define SPLIT 3

#define WIN 0
#define LOST 1
#define BUST 2
#define PUSH 3
#define BLACJACK 4

static std::string card_type[4] = {"Hearts", "Tiles", "Clovers", "Pikes"};
static std::string card_name[13] = {"Ace", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine", "Ten", "Jack", "Queen", "King"};

struct Card {
    std::string type;
    std::string name ;
    int power;
    int faceUp = false;
};

std::vector<Card> InitOriginalDeck()
{    
    std::vector<Card> original_deck;;

    for(int i = 0; i < 4; i++)
    {
        for(int j = 0; j < 13; j++)
        {
            int power = j + 1;
            if(power > 10) power = 10;

            Card card = {card_type[i], card_name[j], power};
            original_deck.push_back(card);
        }
    }

    return original_deck;
}

std::vector<Card> ShuffleDeck(std::vector<Card> deck) 
{
    for (int i = deck.size() - 1; i > 0; --i)
    {
        int j = rand() % (i + 1);
        std::swap(deck[i], deck[j]);
    }

    return deck;
}

int GetHandPower(const std::vector<Card> &cards, bool avoid_face_down = true)
{
    int hand_power = 0;

    int aces_count = 0;
    for(auto card : cards)
    {
        if(!card.faceUp && avoid_face_down) continue;
        
        if(card.name == "Ace")
        {
            hand_power += 11;
            aces_count++;
        }
        else hand_power += card.power;
    }

    while(hand_power > 21 && aces_count > 0)
    {
        hand_power -= 10;
        aces_count--;
    }

    return hand_power;
}

bool BlackJack(const std::vector<Card> &cards)
{
    if(cards.size() != 2) return false;

    return 
    (
        (cards[0].name == "Ace" || cards[1].name == "Ace") &&
        (cards[0].power == 10 || cards[1].power == 10)
    );
}

struct BlackJackGame;

const int PLAYER_ACTION_COUNT = 4;

struct Hand
{
    bool open = true;
    bool busted = false;

    float bet_amount = 0.0f;

    std::vector<Card> cards;

    // OPERATOR GIVES THE CARD AT INDEX
    Card &operator[] (int index)
    {
        if(index >= 0 && index < cards.size())
            return cards[index];
        
        throw ERR_NO_CARDS_AT_IDX;
    }
};

struct Player
{
private:
    int current_hand_index = 0;
    
    std::vector<Hand> open_hands;

    void OpenHand() 
    {
        open_hands.push_back(Hand());
    }

    void AddCard(const Card &card)
    {
        open_hands[current_hand_index].cards.push_back(card);
    }
    
    int OpenHandsCount()
    {
        return open_hands.size();
    }

    int HandsLeftCount()
    {
        int count = 0;
        for(const auto &hand : open_hands)
            if(hand.open) count++;

        return count;
    }
    
    void NextHand()
    {
        if(HandsLeftCount() == 0) return;

        current_hand_index += 1;
        if(current_hand_index > open_hands.size() - 1)
        {
            throw ERR_HAND_NOT_FOUND;
        }
    }

    Hand &GetCurrentHand()
    {
        return open_hands[current_hand_index];
    }

    // OPERATOR GIVES THE HAND AT THE INDEX
    Hand &operator()(int index)
    {
        if(index >= 0 && index < open_hands.size())
            return open_hands[index];
        
        throw ERR_HAND_NOT_FOUND;
    }

    const std::vector<Hand> &GetOpenHands()
    {
        return open_hands;
    }

    void SplitHand(const Card &card_one, const Card &card_two)
    {
        if(open_hands[current_hand_index].cards.size() != 2) return;

        OpenHand();
        
        int last_opened_hand =  open_hands.size() - 1;
        open_hands[last_opened_hand].cards.push_back(open_hands[current_hand_index][1]);
        
        open_hands[current_hand_index].cards.pop_back();

        open_hands[current_hand_index].cards.push_back(card_one);
        open_hands[last_opened_hand].cards.push_back(card_two);

        open_hands[last_opened_hand].bet_amount = open_hands[current_hand_index].bet_amount;
    }

    void ResetHands()
    {
        open_hands.clear();
        current_hand_index = 0;
    }

    friend BlackJackGame;

public:
    float balance = 0.0f;

};

struct Dealer
{
private:
    std::vector<Card> cards;

    void AddCard(const Card &card)
    {
        cards.push_back(card);
    }

    friend BlackJackGame;
};

enum GameState
{
    WAITING,
    BETTING,
    PLAYER_TURN,
    DEALER_TURN,
    RESULTS
};

// THIS WILL GET INFO ABOUT THE GAME FOR USE IN A USER INTERFACE
struct GameInfo
{
    int dealer_action;
    int current_hand_index;

    float bet_amount;
    float balance;

    GameState state;

    std::vector<Card> dealer_cards;
    Hand current_hand;
};

// THIS WILL GET INFO FOR THE RESULTS OF THE GAME FOR A UI
struct ResultGameInfo
{
    float balance;
    float win_amount;

    std::vector<int> game_results;
    std::vector<Card> dealer_cards;
    std::vector<Hand> player_hands;
};

struct BlackJackGame
{
private:
    bool dealer_done = false;

    bool splitted = false;

    float player_bet_amount = 0.0f;

    GameState state;

    Dealer dealer;
    Player player;
    std::vector<Card> deck;

    Card GiveCard(bool faceUp = true)
    {
        if(deck.size() <= 0)
            throw ERR_NO_CARDS_IN_DECK;

        Card card = deck.back();
        card.faceUp = faceUp;
        deck.pop_back();

        return card;
    }

    void OpenPlayerHand(float bet_amount, const std::vector<Card> &cards)
    {
        player.OpenHand();
        
        int last_opened_hand = player.OpenHandsCount() - 1;

        player(last_opened_hand).bet_amount = bet_amount;
        player(last_opened_hand).cards = cards;
    }

    void Betting()
    {
        if(player_bet_amount < 1.0f) return; 
        OpenPlayerHand(player_bet_amount, {});

        player.AddCard(GiveCard());
        dealer.AddCard(GiveCard());
        player.AddCard(GiveCard());
        dealer.AddCard(GiveCard(false));

        state = PLAYER_TURN;

        #ifdef USE_GET_INFO
        game_info.bet_amount = player_bet_amount;
        #endif
    }

    void PlayerHit()
    {
        player.AddCard(GiveCard());
    }

    void PlayerDouble()
    {
        player.AddCard(GiveCard());
        player.GetCurrentHand().bet_amount *= 2;
        player.GetCurrentHand().open = false;
    }

    void PlayerSplit()
    {
        player.SplitHand(GiveCard(), GiveCard());
        splitted = true;
    }

    void PlayerStand()
    {
        player.GetCurrentHand().open = false;
    }

    void DealerHit()
    {   
        dealer.AddCard(GiveCard());
    }   

    void DealerStand()
    {
        dealer_done = true;
    }

    void DealerTurn()
    {
        int action = -1;
        if(GetHandPower(dealer.cards) <= 16) action = HIT;
            else action = STAND;

        if(action == HIT) 
        {
            DealerHit();
        }
        else if(action == STAND) 
        {
            DealerStand();
        }

        if(dealer_done)
        {
            state = RESULTS;
        }

        #ifdef USE_GET_INFO
        game_info.dealer_action = action;
        #endif
    }

    void AdvanceTurnLogic()
    {
        if(player.GetCurrentHand().open)
            return;

        if(player.HandsLeftCount() > 0)
        {
            player.NextHand();
        }
        else 
        {
            state = DEALER_TURN;

            dealer.cards[1].faceUp = true;
        }
    }

    void Results()
    {
        std::vector<int> game_results(player.OpenHandsCount());

        int dealer_hand_power = GetHandPower(dealer.cards);
        for(int hand = 0; hand < player.OpenHandsCount(); hand++)
        {
            float win_amount = 0;
            int game_result = -1;
            
            int player_hand_power = GetHandPower(player(hand).cards);

            bool player_blackjack = BlackJack(player(hand).cards);
            bool dealer_blackjack = BlackJack(dealer.cards);  

            if(player(hand).busted)
                game_result = BUST;
            else if(player_blackjack && dealer_blackjack)
                game_result = PUSH;
            else if(dealer_blackjack && !player_blackjack)
                game_result = LOST;
            else if(player_blackjack && !dealer_blackjack)
                game_result = BLACJACK;
            else if(player_hand_power == dealer_hand_power)
                game_result = PUSH;
            else if (
                player_hand_power <= 21 && (
                    dealer_hand_power > 21 || 
                    dealer_hand_power < player_hand_power
                )
            ) 
            game_result = WIN;

            else if (
                dealer_hand_power <= 21 && (
                    player_hand_power > 21 ||
                    player_hand_power < dealer_hand_power
                )
            )
            game_result = LOST;

            if(game_result == BLACJACK) win_amount = player(hand).bet_amount * 1.5;   
            else if(game_result == WIN) win_amount = player(hand).bet_amount;
            else if(game_result == LOST || game_result == BUST) win_amount = -player(hand).bet_amount;

            player.balance += win_amount;
            game_results[hand] = game_result;

            #ifdef USE_GET_INFO
            results_info.win_amount = win_amount;
            #endif
        }

        state = WAITING;

        #ifdef USE_GET_INFO
        game_info.balance = player.balance;
        results_info.dealer_cards = dealer.cards;
        results_info.game_results = game_results;
        results_info.player_hands = player.GetOpenHands();
        results_info.balance = player.balance;
        #endif
    }

    #ifdef USE_GET_INFO
    GameInfo game_info;
    ResultGameInfo results_info;
    #endif

public:
    bool game_done = false;
    void Init(const Player &player, const std::vector<Card> &deck)
    {
        this->player = player;
        this->deck = deck;

        state = WAITING;
    }

    void Start()
    {
        state = BETTING;
        
        #ifdef USE_GET_INFO
        game_info.balance = player.balance;
        #endif
    }

    void ResetGame()
    {
        player.ResetHands();
        dealer.cards.clear();
        dealer_done = false;
        splitted = false;
        player_bet_amount = 0;
        game_done = false;

        state = BETTING;
    }

    void SetDeck(const std::vector<Card> deck)
    {
        this->deck = deck;
    }

    bool IsActionValid(int action)
    {
        Hand &current_hand = player.GetCurrentHand();

        if(action == HIT)
        {
            return
            (
                deck.size() > 0 &&
                !player.GetCurrentHand().busted
            );
        }  

        if(action == STAND)
        {
            return 
            (
                true
            );
        }

        if(action == DOUBLE)
        {
            return
            (
                current_hand.bet_amount <= player.balance - current_hand.bet_amount &&
                current_hand.cards.size() == 2 
            );
        }

        if(action == SPLIT)
        {
            return
            (
                current_hand.bet_amount <= player.balance - current_hand.bet_amount &&
                current_hand[0].power == current_hand[1].power && 
                current_hand.cards.size() == 2 && 
                !splitted
            );
        }

        return false;
    }

    bool ApplyPlayerAction(int action)
    {
        if(state != PLAYER_TURN)
            return false;

        if(!IsActionValid(action)) 
            return false;

        if(action == HIT)
        {
            PlayerHit();
        }
        else if(action == STAND)
        {
            PlayerStand();
        }
        else if(action == DOUBLE)
        {
            PlayerDouble();
        }
        else if(action == SPLIT)
        {
            PlayerSplit();
        }

        if(GetHandPower(player.GetCurrentHand().cards) > 21)
            player.GetCurrentHand().busted = true;

        AdvanceTurnLogic();

        return true;
    }

    void PlayerBet(float player_bet_amount)
    {
        this->player_bet_amount = player_bet_amount;
    }

    GameState GetGameState()
    {
        return state;
    }

    void Run()
    {
        switch (state)
        {
        case BETTING:
            Betting();
            break;
        case DEALER_TURN:
            DealerTurn();
            break;
        case RESULTS:
            Results();
            break;
        case WAITING:
            game_done = true;
            break;
        default:
            break;
        }

        #ifdef USE_GET_INFO
        game_info.state = state;
        game_info.dealer_cards = dealer.cards;
        if(player.OpenHandsCount() > 0)
        {
            game_info.current_hand = player.GetCurrentHand();
            game_info.current_hand_index = player.current_hand_index;
        }
        #endif
    }

    #ifdef USE_GET_INFO
    const GameInfo &GetGameInfo() { return game_info; }
    ResultGameInfo GetResultsInfo() { return results_info; }
    #endif

};