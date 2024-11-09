#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>   
#include <sstream>   
#include <algorithm> 

class TrieNode {
public:
    std::unordered_map<char, TrieNode*> children;
    bool isEndOfWord = false;
};

class Trie {
private:
    TrieNode* root;

public:
    Trie() {
        root = new TrieNode();
    }

    void insert(const std::string& word) {
        TrieNode* node = root;
        for (char ch : word) {
            if (node->children.find(ch) == node->children.end()) {
                node->children[ch] = new TrieNode();
            }
            node = node->children[ch];
        }
        node->isEndOfWord = true;
    }

    void searchPrefix(TrieNode* node, const std::string& prefix, std::vector<std::string>& results) {
        if (node->isEndOfWord) results.push_back(prefix);
        for (auto& pair : node->children) {
            searchPrefix(pair.second, prefix + pair.first, results);
        }
    }

    std::vector<std::string> getSuggestions(const std::string& prefix, int limit = 5) {
        TrieNode* node = root;
        for (char ch : prefix) {
            if (node->children.find(ch) == node->children.end()) {
                return {}; // No suggestions
            }
            node = node->children[ch];
        }
        std::vector<std::string> results;
        searchPrefix(node, prefix, results);
        return results;
    }
};

// Modified function to load words from a comma-separated text file
void loadWordsFromFile(Trie& trie, const std::string& filename) {
    std::ifstream file(filename);
    if (file.is_open()) {
        std::string line;
        std::getline(file, line); // Read the whole line (all words)

        std::stringstream ss(line);
        std::string word;

        while (std::getline(ss, word, ',')) {
            // Remove any leading/trailing whitespace from each word
            word.erase(word.begin(), std::find_if(word.begin(), word.end(), [](unsigned char ch) { return !std::isspace(ch); }));
            word.erase(std::find_if(word.rbegin(), word.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), word.end());

            if (!word.empty()) {
                trie.insert(word);
            }
        }
        file.close();
    }
    else {
        std::cerr << "Error opening file " << filename << std::endl;
    }
}

int main() {
    Trie trie;
    loadWordsFromFile(trie, "D:/Third Year Sem-5/Design and Analysis of Algorithm/Assignment/text.txt");

    sf::RenderWindow window(sf::VideoMode(1000,1000), "Trie Search System");

    // Load font
    sf::Font font;
    if (!font.loadFromFile("D:/Third Year Sem-5/Design and Analysis of Algorithm/Assignment/Debug/Fonts/black.ttf")) {
        std::cout << "Error loading font\n";
        return -1;
    }

    // Load background
    sf::Texture backgroundTexture;
    if (!backgroundTexture.loadFromFile("D:/Third Year Sem-5/Design and Analysis of Algorithm/Assignment/background.jpg")) {
        std::cout << "Error loading background image\n";
        return -1;
    }
    sf::Sprite backgroundSprite(backgroundTexture);

    // Input text setup
    sf::Text inputText;
    inputText.setFont(font);
    inputText.setCharacterSize(24);
    inputText.setFillColor(sf::Color::White);

    // Suggestion text setup
    sf::Text suggestionText;
    suggestionText.setFont(font);
    suggestionText.setCharacterSize(20);
    suggestionText.setFillColor(sf::Color::Cyan);

    std::string userInput;
    std::vector<std::string> suggestions;

    // Main loop
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode == '\b' && !userInput.empty()) {
                    userInput.pop_back(); // Handle backspace
                }
                else if (event.text.unicode < 128 && event.text.unicode != '\b') {
                    userInput += static_cast<char>(event.text.unicode);
                }
                suggestions = trie.getSuggestions(userInput);
            }
        }

        // Update displayed text
        inputText.setString("Input: " + userInput);
        inputText.setPosition((window.getSize().x - inputText.getLocalBounds().width) / 2, 50);

        // Prepare suggestion display
        std::string suggestionStr = "Suggestions:\n";
        int count = 0;
        for (const auto& suggestion : suggestions) {
            if (++count > 5) break;
            suggestionStr += suggestion + "\n";
        }
        suggestionText.setString(suggestionStr);
        suggestionText.setPosition((window.getSize().x - suggestionText.getLocalBounds().width) / 2, 100);

        // Rendering
        window.clear();
        window.draw(backgroundSprite);
        window.draw(inputText);
        window.draw(suggestionText);
        window.display();
    }

    return 0;
}
