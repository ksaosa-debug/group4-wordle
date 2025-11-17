/************************************************************/
/* Author: Matthew Ventura */
/* Major: Computer Science */
/* Creation Date: 11/15/25 */
/* Due Date: TBD */
/* Course:  CPSC 328*/
/* Professor Name: Professor Walther */
/* Assignment: Network Design */
/* Filename: library.cpp */
/* Purpose: library of functions that are used in the client server for the wrodle game. */
/************************************************************/

#include "library.h"
#include <cctype>   // for isalpha, tolower
#include <iostream>
#include <ctime> 
#include <cstdlib>
#include <fstream>  
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h> 
#include <errno.h>
#include <cstring>

using namespace std;

// ------------------------------
// Removes leading and trailing whitespace
// ------------------------------
string trim_whitespace(const string &str) {
    if (str.empty()) {  
        return "";      //check to see if the string is empty, if it is returns empty str literal
    }

    int start = 0;          // start represents index of first char in string 
    int end = str.length() - 1;  //end is assigned to last char of the string  

    // Move start forward past spaces
    while (start <= end && isspace(str[start])) {
        start++;
    }

    // Move end backward past spaces
    while (end >= start && isspace(str[end])) {
        end--;
    }

    // If entire string was whitespace
    if (start > end) {
        return "";
    }

    return str.substr(start, end - start + 1);      //returns the trimmed string
}

// Validates that a word is 5 alphabetic letters (A–Z)
bool validate_word(const string &word) { //loop itrerates 5 tiumes checking each char one by one 
    if (word.length() != 5) {
        return false;
    }

    for (int i = 0; i < 5; i++) {
        if (!isalpha(word[i])) {        //isalpha used to evaluate valid letter A-Z used this example: https://www.programiz.com/cpp-programming/library-function/cctype/isalpha
            return false; // Contains non-letter characters
        }
    }

    return true; //returns true meaning word has passed validation for length and proper character
}


// Compares guess with target and returns pattern
string compare_guess(const string &guess, const string &target) {
    string result = "_____"; // each _ represents a letter in the guess that is not present in target word

    // Copy of target to mark matched letters
    string tempTarget = target;

    // loop that checks for letters that match both char and index
    for (int i = 0; i < 5; i++) { //iterates through all 4 postions
        if (tolower(guess[i]) == tolower(tempTarget[i])) {       //Allows upper and lowercase
            result[i] = '+';        //uses + if exact letter and position match 
            tempTarget[i] = '*'; // temp is marked 
        }
    }

    // belw loop checks for correct letter in wrong positions 
    for (int i = 0; i < 5; i++) {
        if (result[i] == '+') {     //this check skips ant pos in guess already marked as a match '+'
            continue; 
        }
                //inner loop iterated through eacyh char of the tempTarget to see if guess[i] char exists
        for (int j = 0; j < 5; j++) {
            if (tolower(guess[i]) == tolower(tempTarget[j]) && tempTarget[j] != '*') {
                result[i] = '?';
                tempTarget[j] = '*'; //prevents the guess letter from matching a target letter that was already used
                break;
            }
        }
    }

    return result; //final result is string ex (_?+?_)
}

//function to read words from a text file and process them into vector 
bool load_words(const string &path, vector<string> &out_words) {
    out_words.clear();

    ifstream fin(path.c_str());
    if (!fin.is_open()) {
         return false; // error checking to check if file system was open
    }

    string line;
    while (getline(fin, line)) {
        string w = trim_whitespace(line); //calls the trim_whitespace() function removing any whitespace 

        // loops through the arr of chars to convert to lowercase for consistency
        for (size_t i = 0; i < w.size(); i++) {
            w[i] = (char)tolower(w[i]);
        }

        if (validate_word(w)) { //once again calls validate_word function to check validity of word
            out_words.push_back(w); //if validation is passed, it is appended to out_words vector
        }
        // else ignore invalid lines (could log if you want)
    }

    fin.close();

    if (out_words.size() == 0) {
        return false; //error checking to assure words were loaded in after reading file
    }

    // https://en.cppreference.com/w/c/numeric/random/srand used this link to help w/ randome number generator (used to pick a random target word)
    static bool seeded = false;
    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = true;
    }

    return true;
}


// Returns a single rand word from the list, or "" if empty
//https://www.cplusplus.com/reference/cstdlib/rand/ Used lofgic from this code snippet and edited it for project usage
string get_random_word(const vector<string> &words) {
    if (words.size() == 0) {
        return "";
    }
    int idx = rand() % (int)words.size();   //uses  modulus to constraint the indeces of the random number 
    return words[(size_t)idx];
}

//function reliably ensures a string of data can be sent over a network socket
bool send_message(int sockfd, const string &data) {
    string line = data; //variable line contains the entire message
    line.push_back('\n'); //adds null term to signify end of message 

    const char *buf = line.c_str();
    int total = 0; 
    int to_send = (int)line.size();     //total num of bytes in package 

    //https://beej.us/guide/bgnet/html/split/client-server-background.html#sending-a-complete-buffer used beej's guide for the while loop portion for data to be sent incrementally
    while (total < to_send) { //loop iterates while the total num bytes sent (initally starts at 0) equal total size
        int n = (int)send(sockfd, buf + total, to_send - total, 0);
        if (n == -1) {
            // send error
            return false;
        }
        if (n == 0) {
            // peer closed unexpectedly
            return false;
        }
        total += n;
    }
    return true;
}


//reliably read data from a socket (sockfd) until it finds a newline character (\n), which is used as a message terminator
bool receive_message(int sockfd, string &out) {
    out.clear(); //clears content on output string prior to recieving message
    string accum;       //variable will store the bytes being read from socket
    const int BUFF_SIZE = 1024;
    char buf[BUFF_SIZE]; //holds data directly from socket 

    // keep reading until we find a newline
    while (true) {
        // loop iterates through all the data stored in accum
        for (size_t i = 0; i < accum.size(); i++) {
            if (accum[i] == '\n') {
                out = accum.substr(0, (int)i); //if the \n is found, extracts all chars prior to \n and stores in out 
                
                // not in 'accum'; we only read from the socket each loop)
                return true;
            }
        }

        // used similar code in assignemnt 5 to recieve the bytes 
        int n = (int)recv(sockfd, buf, BUFF_SIZE, 0); // n stores the actual num bytes read
        if (n == -1) {
            // indicates an error in socket
            return false;
        }
        if (n == 0) {
            // closed connection
            return false; //due to no \n false is returned to function
        }

        // append to accum
        for (int i = 0; i < n; i++) {
            accum.push_back(buf[i]);
        }
        // loop continues; we'll check for '\n' at top of loop
    }
}

//designed to write a timestamped message to a file named log.txt. It ensures that new messages are appended to the end of the file
void log_event(const string &msg) {
    // Open log file in append mode
    ofstream fout("log.txt", ios::app); //ios::app assures data will not overwrite older data
    if (!fout.is_open()) {
        // error checking to see if stream failed
        return;
    }

    // get current time
    time_t now = time(NULL);
    char *timeStr = ctime(&now);

    // ctime() adds a newline; we remove it manually
    string cleanTime = "";
    for (int i = 0; timeStr[i] != '\0'; i++) {
        if (timeStr[i] != '\n') {
            cleanTime.push_back(timeStr[i]);
        }
    }

    fout << "[" << cleanTime << "] " << msg << endl;
    fout.close();
}



// Print error message & exit program cleanly

void error_exit(const string &msg) {
    cout << "ERROR: " << msg << endl;
    exit(1);
}



// Close a socket safely
void close_connection(int sockfd) {
    if (sockfd >= 0) {
        close(sockfd);
    }
}

