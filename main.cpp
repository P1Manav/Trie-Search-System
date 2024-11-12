#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <limits>
#include <cctype>

class TrieNode {
public:
    std::unordered_map<char, TrieNode*> children;
    bool isEndOfWord = false;
};

class Trie {
private:
    TrieNode* root;

    std::string toLowerCase(const std::string& str) {
        std::string result;
        for (char ch : str) {
            result += std::tolower(ch);
        }
        return result;
    }

public:
    Trie() {
        root = new TrieNode();
    }

    void insert(const std::string& sentence) {
        TrieNode* node = root;
        std::string lowerSentence = toLowerCase(sentence);
        for (char ch : lowerSentence) {
            if (node->children.find(ch) == node->children.end()) {
                node->children[ch] = new TrieNode();
            }
            node = node->children[ch];
        }
        node->isEndOfWord = true;
    }

    void searchPrefix(TrieNode* node, const std::string& prefix, std::vector<std::string>& results) {
        if (node->isEndOfWord && results.size() < 10) results.push_back(prefix);
        for (auto& pair : node->children) {
            if (results.size() >= 10) return;
            searchPrefix(pair.second, prefix + pair.first, results);
        }
    }

    std::vector<std::string> getSuggestions(const std::string& prefix) {
        TrieNode* node = root;
        std::string lowerPrefix = toLowerCase(prefix);
        for (char ch : lowerPrefix) {
            if (node->children.find(ch) == node->children.end()) {
                return {};
            }
            node = node->children[ch];
        }

        std::vector<std::string> results;
        searchPrefix(node, lowerPrefix, results);
        std::sort(results.begin(), results.end());
        return results;
    }

    int levenshteinDistance(const std::string& a, const std::string& b) {
        int m = a.size();
        int n = b.size();
        std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1, 0));

        for (int i = 0; i <= m; ++i) dp[i][0] = i;
        for (int j = 0; j <= n; ++j) dp[0][j] = j;

        for (int i = 1; i <= m; ++i) {
            for (int j = 1; j <= n; ++j) {
                if (a[i - 1] == b[j - 1]) {
                    dp[i][j] = dp[i - 1][j - 1];
                }
                else {
                    dp[i][j] = 1 + std::min({ dp[i - 1][j], dp[i][j - 1], dp[i - 1][j - 1] });
                }
            }
        }
        return dp[m][n];
    }

    void getAllWords(TrieNode* node, const std::string& current, std::vector<std::string>& words) {
        if (node->isEndOfWord) words.push_back(current);
        for (auto& pair : node->children) {
            getAllWords(pair.second, current + pair.first, words);
        }
    }

    std::vector<std::string> getClosestSuggestions(const std::string& word, int maxDistance = 2) {
        std::vector<std::string> allWords;
        getAllWords(root, "", allWords);

        std::vector<std::string> results;
        std::string lowerWord = toLowerCase(word);
        for (const auto& candidate : allWords) {
            if (levenshteinDistance(lowerWord, candidate) <= maxDistance) {
                results.push_back(candidate);
            }
        }
        std::sort(results.begin(), results.end(), [&](const std::string& a, const std::string& b) {
            return levenshteinDistance(lowerWord, a) < levenshteinDistance(lowerWord, b);
            });
        return results;
    }
};

void loadWordsAndSentencesFromFile(Trie& trie, const std::string& filename) {
    std::ifstream file(filename);
    if (file.is_open()) {
        std::string entry;
        while (std::getline(file, entry, ',')) {
            entry.erase(entry.begin(), std::find_if(entry.begin(), entry.end(), [](unsigned char ch) { return !std::isspace(ch); }));
            entry.erase(std::find_if(entry.rbegin(), entry.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), entry.end());

            if (!entry.empty()) {
                trie.insert(entry);
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
    loadWordsAndSentencesFromFile(trie, "text.txt");

    sf::RenderWindow window(sf::VideoMode(1200, 1300), "Trie Search System");

    sf::Font font;
    if (!font.loadFromFile("./Fonts/black.ttf")) {
        std::cout << "Error loading font\n";
        return -1;
    }

    sf::Texture backgroundTexture;
    if (!backgroundTexture.loadFromFile("background.jpg")) {
        std::cout << "Error loading background image\n";
        return -1;
    }
    sf::Sprite backgroundSprite(backgroundTexture);

    sf::Text inputText;
    inputText.setFont(font);
    inputText.setCharacterSize(42);
    inputText.setFillColor(sf::Color::White);

    sf::Text suggestionText;
    suggestionText.setFont(font);
    suggestionText.setCharacterSize(36);
    suggestionText.setFillColor(sf::Color::Cyan);

    std::string userInput;
    std::vector<std::string> suggestions;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode == '\b' && !userInput.empty()) {
                    userInput.pop_back();
                }
                else if (event.text.unicode < 128 && event.text.unicode != '\b') {
                    userInput += std::tolower(static_cast<char>(event.text.unicode));
                }

                suggestions = trie.getSuggestions(userInput);
                if (suggestions.empty()) {
                    suggestions = trie.getClosestSuggestions(userInput, 2);
                }
            }
            else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Tab && !suggestions.empty()) {
                    userInput = suggestions[0];
                }
            }
        }

        inputText.setString("Input: " + userInput);
        inputText.setPosition((window.getSize().x - inputText.getLocalBounds().width) / 2, 50);

        std::string suggestionStr = "Suggestions:\n";
        for (const auto& suggestion : suggestions) {
            suggestionStr += suggestion + "\n";
        }
        suggestionText.setString(suggestionStr);
        suggestionText.setPosition((window.getSize().x - suggestionText.getLocalBounds().width) / 2, 100);

        window.clear();
        window.draw(backgroundSprite);
        window.draw(inputText);
        window.draw(suggestionText);
        window.display();
    }

    return 0;
}
