#define USE_GET_INFO

#include "BlackJack.h"

/*
    BLACKJACK USER INTERFACE 

    THIS IS SOME SIMPLE UI FOR THE BLACKJACK ENGINE I CREATED SO THAT YOU CAN ACTUALY PLAY THE GAME 
    YOU CAN MAKE THE UI BETTER IF YOU WANT.

    TO PLAY YOU PLACE A BET AT THE START FROM A BALANCE OF $100 
    THEN THE GAME STARTS YOU CAN GIVE AVAILABLE ACTIONS HIT, STAND, ETC.
    THEN AFTER YOUR TURN THE DEALER STARTS, FINISHES 
    THEN THE RESULTS ARE SHOWN FOR ALL THE HANDS YOU PLAYED THAT ROUND
    AFTER ALL THIS A LOOP STARTS 
*/

#include <iostream>
#include <thread>
#include <chrono>

BlackJackGame blackjack_game;

std::string action_strings[5] = {"HIT", "STAND", "DOUBLE", "SPLIT"};
std::string result_strings[6] = {"WIN", "LOST", "BUST", "PUSH", "BLACKJACK"};

void ShowCards(const std::vector<Card> &cards)
{
    for(auto card : cards)
    {
        if(card.faceUp == 0) 
        {
            std::cout << "FaceDown "; 
            continue; 
        
        }
        if(card.name != "Ace") std::cout << card.power << ' ' ;

        std::cout << card.name << '-' ;        
        std::cout << card.type << ' '; 
    }
}

void BetUI(const GameInfo &game_info)
{
    std::system("cls");

    std::cout << "Balance: $" << game_info.balance << '\n';

    float bet_amount = 0;
    while (bet_amount <= 0)
    {
        std::cout << "Bet: ";
        std::cin >> bet_amount;

        if(bet_amount > game_info.balance)
            bet_amount = 0; 
    }
    
    blackjack_game.PlayerBet(bet_amount);
}

void PlayerTrunUI(const GameInfo &game_info)
{
    std::system("cls");

    int player_action = -1;

    std::cout << "Hand #" << game_info.current_hand_index << '\n'; 
    std::cout << "Player Cards: ";
    ShowCards(game_info.current_hand.cards);
    std::cout << " | (" << GetHandPower(game_info.current_hand.cards) << ")" << '\n';
    std::cout << "Dealer Cards: ";
    ShowCards(game_info.dealer_cards);
    std::cout << " | (" << GetHandPower(game_info.dealer_cards) << ")" << '\n';
    std::cout << "=====================================================" << '\n';
    std::cout << "Actions: ";
    
    for(int i = 0; i < PLAYER_ACTION_COUNT; i++)
    {
        if(blackjack_game.IsActionValid(i))
            std::cout << i + 1 << "-" << action_strings[i] << " ";
    }

    std::cout << '\n';
    std::cout << "Option: ";
    while (!blackjack_game.ApplyPlayerAction(player_action - 1))
    {
        std::cin >> player_action;
    }
}

void DealerTurnUI(const GameInfo &game_info)
{
    std::system("cls");

    int player_action = -1;

    std::cout << "Dealer Cards: ";
    ShowCards(game_info.dealer_cards);
    std::cout << " | (" << GetHandPower(game_info.dealer_cards) << ")" << '\n';
    std::cout << "=====================================================" << '\n';
    std::cout << "Action: " << action_strings[game_info.dealer_action] << '\n';

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
}

void ResultsUI(const ResultGameInfo &result_info)
{
    std::system("cls");

    std::cout << "Balance: $" << result_info.balance << '\n';
    
    std::cout << "=====================================================" << '\n';

    std::cout << "Dealer Cards: ";
    ShowCards(result_info.dealer_cards);
    std::cout << " | (" << GetHandPower(result_info.dealer_cards) << ")" << '\n';

    std::cout << "=====================================================" << '\n';

    for(int i = 0; i < result_info.game_results.size(); i++)
    {
        std::cout << "Hand #" << i + 1 << " | " << result_info.win_amount << "$" << '\n';
        std::cout << "Player Cards: ";
        ShowCards(result_info.player_hands[i].cards);
        std::cout << " | (" << GetHandPower(result_info.player_hands[i].cards) << ")" << '\n';
        std::cout << result_strings[result_info.game_results[i]] << '\n';
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

void DisplayGame()
{
    const GameInfo &game_info = blackjack_game.GetGameInfo();

    switch (game_info.state)
    {
    case BETTING:
        BetUI(game_info);
        break;
    case PLAYER_TURN:
        PlayerTrunUI(game_info);
        break;
    case DEALER_TURN:
        DealerTurnUI(game_info);
        break;
    case WAITING:
        ResultsUI(blackjack_game.GetResultsInfo());
        break;
    default:
        break;
    }
}

std::vector<Card> original_deck;
bool game_runnning = true;

int main()
{
    srand(time(NULL));

    original_deck = InitOriginalDeck();

    Player player;
    player.balance = 100;

    blackjack_game.Init(player, ShuffleDeck(original_deck));
    blackjack_game.Start();

    while (game_runnning)
    {
        blackjack_game.Run();

        if(blackjack_game.game_done)
        {
            blackjack_game.ResetGame();
            blackjack_game.SetDeck(ShuffleDeck(original_deck));
        }

        DisplayGame();
    }

    return 0;
}