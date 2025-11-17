#ifndef LIBRARY_H
#define LIBRARY_H

#include <string>
#include <vector>
using namespace std;

// Removes extra whitespace (spaces, tabs, newlines) from start and end of a string
string trim_whitespace(const string &str);

// Checks if a word is valid: exactly 5 letters (A-Z only)
bool validate_word(const string &word);

// Compares a player's guess to the target word and returns feedback pattern
// '+' = correct letter, correct position
// '?' = correct letter, wrong position
// '_' = letter not in word
string compare_guess(const string &guess, const string &target);

bool   load_words(const string &path, vector<string> &out_words);
string get_random_word(const vector<string> &words);

bool send_message(int sockfd, const string &data);
bool receive_message(int sockfd, string &out);

// Logs a message with a timestamp to a log file (shared by client/server)
void log_event(const string &msg);

// Prints an error message and exits the program safely
void error_exit(const string &msg);

// Closes a socket connection cleanly
void close_connection(int sockfd);

#endif
