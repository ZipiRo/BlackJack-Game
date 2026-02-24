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

/*
    I WANTED TO ADD THIS CHEET SHEET TO SEE HOW EFECTIVE IT IS
    AI HELPED ME WITH THE PARSING OF THE TABLE (I AM TO LAZY FOR THAT)
    AND HELPED ME ON THE FUNCTION Action GetBestAction(player_hand, dealer_card);

    WHEN IT IS THE PLAYER'S TURN THERE WILL BE A BEST ACTION VALUE
*/

/*
    I MADE A SIMULTION USING THIS CHEET SHEET SO KNOW I CAN REALY SEE HOW EFECTIVE IT IS
    TO RUN SUMULATIONS YOU NEED TO RUN THE PROGRAM IN THE COMAND PROMPT LIKE THIS
    --------------------------------------------------------------
    COMMAND: ./program --sim [simulations] [balance] [min_bet] [max_bet]
    --------------------------------------------------------------
    AFTER THE SIMULATION THE PROGRAM WILL OUTPUT A FILE NAMED 'bj_simulation.log',
    WHERE YOU CAN FIND INFORMATION ABOUT THE SIMULATION. 
    IN THE LOG FILE YOU FIND THE ROUND INFORMATION, MIN_BALANCE, MAX_BALANCE, ROUNDS_PLAYED.
*/

/*
    ADDED THE OPTION TO OUTPUT A CSV FILE FOR VISUAL STATISTICS
    COMMAND: ./program --sim [simulations] [balance] [min_bet] [max_bet] (--csv [file_name] ) 
*/

#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>
#include <random>
#include <cstdlib>
#include <cstring>

BlackJackGame blackjack_game;

std::string action_strings[5] = {"HIT", "STAND", "DOUBLE", "SPLIT"};
std::string result_strings[6] = {"WIN", "LOST", "BUST", "PUSH", "BLACKJACK"};
 
enum Action
{
    H, S, P,
    DH, DS, PH, RH
}; 

std::string recomanded_action_string[8] = 
{
    "HIT",
    "STAND",
    "SPLIT",
    "DOUBLE/HIT",
    "DOUBLE/STAND",
    "SPLIT/HIT",
    "SURRENDER/HIT"
};

const int HARD_START = 0;
const int HARD_END = 9;
const int SOFT_START = 10;
const int SOFT_END = 16;
const int PAIR_START = 17;
const int PAIR_END = 26;

Action cheet_table[27][10] = {
    /* 8  */ {H,  H,  H,  H,  H,  H,  H,  H,  H,  H}, // HARD
    /* 9  */ {H,  DH, DH, DH, DH, H,  H,  H,  H,  H},
    /* 10 */ {DH, DH, DH, DH, DH, DH, DH, DH, H,  H},
    /* 11 */ {DH, DH, DH, DH, DH, DH, DH, DH, DH, DH},
    /* 12 */ {H,  H,  S,  S,  S,  H,  H,  H,  H,  H},
    /* 13 */ {S,  S,  S,  S,  S,  H,  H,  H,  H,  H},
    /* 14 */ {S,  S,  S,  S,  S,  H,  H,  H,  H,  H},
    /* 15 */ {S,  S,  S,  S,  S,  H,  H,  H,  RH, H},
    /* 16 */ {S,  S,  S,  S,  S,  H,  H,  RH, RH, RH},
    /* 17 */ {S,  S,  S,  S,  S,  S,  S,  S,  S,  S}, // END

    /* A,2*/ {H,  H,  H,  DH, DH, H,  H,  H,  H,  H}, // SOFT
    /* A,3*/ {H,  H,  H,  DH, DH, H,  H,  H,  H,  H},
    /* A,4*/ {H,  H,  DH, DH, DH, H,  H,  H,  H,  H},
    /* A,5*/ {H,  H,  DH, DH, DH, H,  H,  H,  H,  H},
    /* A,6*/ {H,  DH, DH, DH, DH, H,  H,  H,  H,  H},
    /* A,7*/ {S,  DS, DS, DS, DS, S,  S,  H,  H,  H},
    /* A,8*/ {S,  S,  S,  S,  S,  S,  S,  S,  S,  S}, // END

    /* 2,2*/ {PH, PH, P,  P,  P,  P,  H,  H,  H,  H}, // PAIR
    /* 3,3*/ {PH, PH, P,  P,  P,  P,  H,  H,  H,  H},
    /* 4,4*/ {H,  H,  H,  PH, PH, H,  H,  H,  H,  H},
    /* 5,5*/ {DH, DH, DH, DH, DH, DH, DH, DH, H,  H},
    /* 6,6*/ {PH, P,  P,  P,  P,  H,  H,  H,  H,  H},
    /* 7,7*/ {P,  P,  P,  P,  P,  P,  H,  H,  H,  H},
    /* 8,8*/ {P,  P,  P,  P,  P,  P,  P,  P,  P,  P},
    /* 9,9*/ {P,  P,  P,  P,  P,  S,  P,  P,  S,  S},
    /*10,10*/{S,  S,  S,  S,  S,  S,  S,  S,  S,  S},
    /* A,A*/ {P,  P,  P,  P,  P,  P,  P,  P,  P,  P} // END
};

Action GetBestAction(const std::vector<Card> &player_hand, const Card &dealer_card)
{
    int hand_power = GetHandPower(player_hand);
    int column = (dealer_card.name == "Ace") ? 9 : (dealer_card.power - 2);

    // CHECKING FOR PAIRS
    if (player_hand.size() == 2 && player_hand[0].power == player_hand[1].power) 
    {
        int pair_value = player_hand[0].power;
        int row;
        
        if (player_hand[0].name == "Ace") row = PAIR_END;
        else row = PAIR_START + (pair_value - 2);          
        
        return cheet_table[row][column];
    }

    // CHECKING FOR SOFT TOTAL
    bool is_soft = false;
    int other_card_value = 0;

    if (player_hand.size() == 2) 
    {
        if (player_hand[0].name == "Ace") { is_soft = true; other_card_value = player_hand[1].power; }
        else if (player_hand[1].name == "Ace") { is_soft = true; other_card_value = player_hand[0].power; }
    }

    if (is_soft) 
    {
        if (other_card_value >= 8) return Action::S;
        int row = SOFT_START + (other_card_value - 2); 
        return cheet_table[row][column];
    }

    // CHECKING THE REST OF THE TOTALS
    int row;
    if (hand_power <= 8) row = HARD_START;
    else if (hand_power >= 17) row = HARD_END;
    else row = HARD_START + hand_power - 8;

    return cheet_table[row][column];
}

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

void BetUI()
{
    const GameInfo &game_info = blackjack_game.GetGameInfo();

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

void PlayerTurnUI()
{
    const GameInfo &game_info = blackjack_game.GetGameInfo();

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
    std::cout << "BestAction: " << recomanded_action_string[GetBestAction(game_info.current_hand.cards, game_info.dealer_cards[0])] <<'\n';
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

void DealerTurnUI()
{
    const GameInfo &game_info = blackjack_game.GetGameInfo();

    std::system("cls");

    int player_action = -1;

    std::cout << "Dealer Cards: ";
    ShowCards(game_info.dealer_cards);
    std::cout << " | (" << GetHandPower(game_info.dealer_cards) << ")" << '\n';
    std::cout << "=====================================================" << '\n';
    std::cout << "Action: " << action_strings[game_info.dealer_action] << '\n';

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
}

void ResultsUI()
{
    const ResultGameInfo &result_info = blackjack_game.GetResultsInfo();

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
    switch (blackjack_game.GetGameState())
    {
    case BETTING:
        BetUI();
        break;
    case PLAYER_TURN:
        PlayerTurnUI();
        break;
    case DEALER_TURN:
        DealerTurnUI();
        break;
    case WAITING:
        ResultsUI();
        break;
    default:
        break;
    }
}

std::vector<Card> original_deck;
bool game_runnning = true;

void PlayerPlay()
{
    Player player;
    player.balance = 100;

    blackjack_game.Init(player, ShuffleDeck(original_deck));
    blackjack_game.Start();

    while (game_runnning)
    {
        if(blackjack_game.game_done)
        {
            blackjack_game.ResetGame();
            blackjack_game.SetDeck(ShuffleDeck(original_deck));
        }

        blackjack_game.Run();

        DisplayGame();
    }
}

bool use_CSV_file = false;
std::ofstream CSVFile;
void CreateCSVFile(std::string csv_file_name)
{
    CSVFile.open(csv_file_name + ".csv");
    CSVFile << "Balance,Min_Balance,Max_Balance\n";
}
void AddLineToCSVFile(float balance, float min_balance, float max_balance)
{
    if(!CSVFile.is_open()) return;

    CSVFile << balance << ',' << min_balance << ',' << max_balance << '\n';
}

void SimulatePlay(int rounds, float balance, int min_bet, int max_bet)
{
    std::ofstream out("bj_simulation.log");

    Player player;
    player.balance = balance;

    blackjack_game.Init(player, ShuffleDeck(original_deck));
    blackjack_game.Start();

    int rounds_played = 0;
    float max_balance = player.balance;
    float min_balance = player.balance;

    while (rounds_played != rounds)
    {
        if(blackjack_game.game_done)
        {
            blackjack_game.ResetGame();
            blackjack_game.SetDeck(ShuffleDeck(original_deck));
        }

        if(blackjack_game.GetGameInfo().balance <= 0)
        {
            out << "Player lost all the balance!\n";
            out << "=====================================================" << '\n';
            break;
        }

        if(blackjack_game.GetGameState() == GameState::BETTING)
        {
            float bet = min_bet + rand() % (max_bet- min_bet + 1);
            if(bet > blackjack_game.GetGameInfo().balance)
                bet = blackjack_game.GetGameInfo().balance;

            blackjack_game.PlayerBet(bet);
        }

        blackjack_game.Run();

        if(blackjack_game.GetGameState() == GameState::WAITING)
        {
            const ResultGameInfo &result_info = blackjack_game.GetResultsInfo();
            rounds_played++;

            out << "Round: #" << rounds_played << '\n'; 
            out << "Balance: $" << result_info.balance << '\n';
            for(int i = 0; i < result_info.game_results.size(); i++)
            {
                out << "Hand #" << i + 1 << " | " << result_info.win_amount << "$" << '\n';
                out << result_strings[result_info.game_results[i]] << '\n';
            }
            out << "=====================================================" << '\n';

            if(min_balance > result_info.balance)
                min_balance = result_info.balance;
            
            if(max_balance < result_info.balance)
                max_balance = result_info.balance;

            if(use_CSV_file)
                AddLineToCSVFile(result_info.balance, min_balance, max_balance);
        }

        const GameInfo &game_info = blackjack_game.GetGameInfo();

        int action = GetBestAction(game_info.current_hand.cards, game_info.dealer_cards[0]);

        if(action == Action::H) action = HIT;
        else if(action == Action::S) action = STAND;
        else if(action == Action::P) action = SPLIT;
        else if(action == Action::RH) action = HIT;
        if(action == Action::DS) 
            if(blackjack_game.IsActionValid(DOUBLE)) action = DOUBLE;
                else action = STAND;
        else if(action == Action::DH)
            if(blackjack_game.IsActionValid(DOUBLE)) action = DOUBLE;
                else action = HIT;
        else if(action == Action::PH)
            if(blackjack_game.IsActionValid(SPLIT)) action = SPLIT;
                else action = HIT;

        blackjack_game.ApplyPlayerAction(action);
    }

    out << "Balance: $" << blackjack_game.GetGameInfo().balance << '\n';   
    out << "Min balance: $" << min_balance << '\n';
    out << "Max balance: $" << max_balance << '\n';
    out << "Rounds Played: " << rounds_played << '\n'; 
    out.close();

    std::cout << "Balance: $" << blackjack_game.GetGameInfo().balance << '\n';   
    std::cout << "Min balance: $" << min_balance << '\n';
    std::cout << "Max balance: $" << max_balance << '\n';
    std::cout << "Rounds Played: " << rounds_played << '\n';  
}

void ProgramInfo()
{
    std::cout << "================================================================================================\n";
    std::cout << "To simulate BlackJack rounds you need to use this command\n";
    std::cout << '\n';
    std::cout << "Command: ./program --sim [simulations] [balance] [min_bet] [max_bet] ( --csv [file_name] )\n";
    std::cout << '\n';
    std::cout << "When the program finnishes it will output a log file 'bj_simulation.log';\n";
    std::cout << "The log file will contain: rounds_stats, min_balance, max_balance, rounds_played, balance;\n";
    std::cout << "In the console you'll find the min_balance, max_balance, rounds_played, balance;\n";
    std::cout << '\n';
    std::cout << "Optionaly you can tell the program to create a csv file with a said file_name\n";
    std::cout << '\n';
    std::cout << "The simulation is using the basic cheet sheet table of BlackJack\n";
    std::cout << '\n';
    std::cout << "By ZipiRo\n";
    std::cout << "================================================================================================\n";

}

int main(int argc, char* argv[])
{
    srand(time(0));

    original_deck = InitOriginalDeck();

    if(argc < 2)
    {
        PlayerPlay();
        return 0;
    }

    if(strstr(argv[1], "--sim") && argc >= 6)
    {
        if(argc >= 8 && strstr(argv[6], "--csv"))
        {    
            use_CSV_file = true;
            CreateCSVFile(argv[7]);      
        }
        else if(argc >= 7)
        {
            ProgramInfo();
            return 1;
        } 

        int simulations = std::stoi(argv[2]);
        int balance = std::stoi(argv[3]);
        int min_bet = std::stoi(argv[4]);
        int max_bet = std::stoi(argv[5]);

        SimulatePlay(simulations, balance, min_bet, max_bet);

        return 0;
    }
    else 
    {
        ProgramInfo();
        return 1;
    }
}