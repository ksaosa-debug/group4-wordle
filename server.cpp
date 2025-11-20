#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include "library.h"

using namespace std;

static const int DEFAULT_PORT = 5000;

string get_guess() {
    string guess;
    while (true) {
        cout << "Enter your 5-letter guess: ";
        cin >> guess;

        guess = trim_whitespace(guess);

        if (validate_word(guess)) {
            break;
        }
        cout << "Invalid word. Must be exactly 5 letters A-Z.\n";
    }
    return guess;
}

void display_feedback(const string &guess, const string &pattern) {
    for (size_t i = 0; i < guess.size(); i++) {
        cout << guess[i] << ":" << pattern[i] << " ";
    }
    cout << endl;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cout << "Usage: ./client <hostname> [port]\n";
        return 1;
    }

    string hostname = argv[1];
    int port = DEFAULT_PORT;
    if (argc >= 3) {
        port = atoi(argv[2]);
        if (port <= 0) port = DEFAULT_PORT;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        cout << "Socket creation failed\n";
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, hostname.c_str(), &server_addr.sin_addr) <= 0) {
        cout << "Invalid hostname\n";
        return 1;
    }

    if (connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        cout << "Failed to connect to server on port " << port << endl;
        return 1;
    }

    cout << "Connected to server on port " << port << endl;

    string msg;
    if (!receive_message(sock, msg) || msg != "HELLO") {
        cout << "Did not receive HELLO from server\n";
        close_connection(sock);
        return 1;
    }

    cout << "Server says: " << msg << endl;

    bool play_again = true;
    while (play_again) {
        send_message(sock, "READY");

        string target_word;
        if (!receive_message(sock, target_word)) {
            cout << "Failed to receive word from server\n";
            break;
        }

        int attempts = 0;
        bool won = false;

        while (attempts < 6) {
            string guess = get_guess();
            string feedback = compare_guess(guess, target_word);
            display_feedback(guess, feedback);
            attempts++;

            if (guess == target_word) {
                cout << "Correct! You guessed the word in " << attempts << "/6 tries.\n";
                won = true;
                break;
            }
        }

        if (!won) {
            cout << "You ran out of tries. The correct word was: " << target_word << endl;
        }

        char response;
        cout << "Play again? (y/n): ";
        cin >> response;
        response = tolower(response);

        if (response == 'y') {
            play_again = true;
        } else {
            play_again = false;
            send_message(sock, "BYE");
        }
    }

    close_connection(sock);
    cout << "Disconnected from server. Thanks for playing!\n";
    return 0;
}
