#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <windows.h>

using namespace std;

string suits[] = { "Черви", "Бубни", "Піки", "Трефи" };
string ranks[] = { "6", "7", "8", "9", "10", "В", "Д", "К", "Т" };

struct Card {
    int rank, suit;

    string toString() const {
        return ranks[rank] + " " + suits[suit];
    }

    bool beats(const Card& other, int trump) const {
        if (suit == other.suit && rank > other.rank) return true;
        if (suit == trump && other.suit != trump) return true;
        return false;
    }

    bool sameRank(const Card& other) const {
        return rank == other.rank;
    }
};

vector<Card> deck, player, computer, tableAttack, tableDefense;
int trump;
bool playerTurn = true;

void createDeck() {
    deck.clear();
    for (int s = 0; s < 4; ++s)
        for (int r = 0; r < 9; ++r)
            deck.push_back({ r, s });
    srand(time(0));
    random_shuffle(deck.begin(), deck.end());
    trump = deck.back().suit;
}

void deal(vector<Card>& hand) {
    while (hand.size() < 6 && !deck.empty()) {
        hand.push_back(deck.back());
        deck.pop_back();
    }
}

void showHand(const vector<Card>& hand) {
    for (size_t i = 0; i < hand.size(); ++i)
        cout << i + 1 << ") " << hand[i].toString() << endl;
}

void clearTable() {
    tableAttack.clear();
    tableDefense.clear();
}

bool canThrow(const Card& c) {
    for (const Card& a : tableAttack)
        if (c.sameRank(a)) return true;
    for (const Card& d : tableDefense)
        if (c.sameRank(d)) return true;
    return tableAttack.empty();
}

bool allDefended() {
    return tableAttack.size() == tableDefense.size();
}

void removeCard(vector<Card>& hand, size_t index) {
    hand.erase(hand.begin() + index);
}

bool playerAttack() {
    bool added = false;
    while (true) {
        cout << "Ваші карти:\n";
        showHand(player);
        cout << "Виберіть карту для атаки або підкидання (0 — закінчити хід): ";
        int ch;
        cin >> ch;
        if (ch == 0) break;
        if (ch < 1 || ch >(int)player.size()) continue;
        if (!canThrow(player[ch - 1])) continue;
        Card chosen = player[ch - 1];
        cout << "Ви атакуєте картою: " << chosen.toString() << endl;
        tableAttack.push_back(chosen);
        removeCard(player, ch - 1);
        added = true;
        if (tableAttack.size() >= 6) break;
    }
    return added;
}

bool computerDefend() {
    for (size_t i = 0; i < tableAttack.size(); ++i) {
        Card attackCard = tableAttack[i];
        bool defended = false;
        for (size_t j = 0; j < computer.size(); ++j) {
            if (computer[j].beats(attackCard, trump)) {
                cout << "Комп'ютер захищається картою: " << computer[j].toString() << " проти " << attackCard.toString() << endl;
                tableDefense.push_back(computer[j]);
                removeCard(computer, j);
                defended = true;
                break;
            }
        }
        if (!defended) return false;
    }
    return true;
}

bool computerAttack() {
    bool added = false;
    for (size_t i = 0; i < computer.size(); ++i) {
        if (canThrow(computer[i])) {
            cout << "Комп'ютер підкидає карту: " << computer[i].toString() << endl;
            tableAttack.push_back(computer[i]);
            removeCard(computer, i);
            added = true;
            if (tableAttack.size() >= 6) break;
        }
    }
    return added;
}

bool playerDefend() {
    for (size_t i = tableDefense.size(); i < tableAttack.size(); ++i) {
        Card attackCard = tableAttack[i];
        cout << "Вас атакують картою: " << attackCard.toString() << endl;
        cout << "Ваші карти:\n";
        showHand(player);
        cout << "Виберіть карту для захисту (0 — взяти всі): ";
        int ch;
        cin >> ch;
        if (ch == 0) return false;
        if (ch < 1 || ch >(int)player.size()) return false;
        if (!player[ch - 1].beats(attackCard, trump)) return false;
        cout << "Ви захищаєтеся картою: " << player[ch - 1].toString() << endl;
        tableDefense.push_back(player[ch - 1]);
        removeCard(player, ch - 1);
    }
    return true;
}

void giveCards(vector<Card>& hand, const vector<Card>& cards1, const vector<Card>& cards2) {
    hand.insert(hand.end(), cards1.begin(), cards1.end());
    hand.insert(hand.end(), cards2.begin(), cards2.end());
}

int main() {
    system("chcp 1251 > nul");
    createDeck();
    for (int i = 0; i < 6; ++i) {
        player.push_back(deck.back()); deck.pop_back();
        computer.push_back(deck.back()); deck.pop_back();
    }

    while (true) {
        if (player.empty() && deck.empty()) {
            cout << "Ви виграли!\n"; break;
        }
        if (computer.empty() && deck.empty()) {
            cout << "Комп'ютер виграв!\n"; break;
        }

        clearTable();
        cout << "\nКозир: " << suits[trump] << "\n";

        if (playerTurn) {
            bool attacked = playerAttack();
            if (!attacked) {
                playerTurn = false;
                continue;
            }
            if (computerDefend()) {
                while (computerAttack()) {
                    if (!playerDefend()) {
                        giveCards(player, tableAttack, tableDefense);
                        clearTable();
                        playerTurn = false;
                        break;
                    }
                }
                clearTable();
            }
            else {
                cout << "Комп'ютер не зміг відбитися і бере карти.\n";
                giveCards(computer, tableAttack, tableDefense);
                clearTable();
                playerTurn = true;
                continue;
            }
            playerTurn = false;
        }
        else {
            cout << "\nКомп'ютер атакує:\n";
            Card atk = computer[0];
            tableAttack.push_back(atk);
            cout << "Комп'ютер атакує картою: " << atk.toString() << endl;
            removeCard(computer, 0);
            if (!playerDefend()) {
                giveCards(player, tableAttack, tableDefense);
                clearTable();
                playerTurn = false;
                continue;
            }
            bool added = true;
            while (added && tableAttack.size() < 6) {
                added = computerAttack();
                if (added && !playerDefend()) {
                    giveCards(player, tableAttack, tableDefense);
                    clearTable();
                    playerTurn = false;
                    break;
                }
            }
            clearTable();
            playerTurn = true;
        }

        deal(player);
        deal(computer);
    }

    return 0;
}
