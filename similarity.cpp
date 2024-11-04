#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include <array>
#include <filesystem>

const std::array<std::string, 6> COMMON_WORDS = { "a", "and", "an", "of", "in", "the" };
const int TOP_N = 100;

std::string normalizeWord(const std::string& word) {
    std::string normalized;
    for (char c : word) {
        if (std::isalnum(c)) {
            normalized += std::toupper(c);
        }
    }
    return normalized;
}

void countWords(const std::string& filename, std::unordered_map<std::string, int>& wordCount, int& totalWords) {
    std::ifstream file(filename);
    std::string word;

    while (file >> word) {
        std::string normalizedWord = normalizeWord(word);
        if (std::find(COMMON_WORDS.begin(), COMMON_WORDS.end(), normalizedWord) == COMMON_WORDS.end()) {
            wordCount[normalizedWord]++;
            totalWords++;
        }
    }
}

void calculateNormalizedFrequency(const std::unordered_map<std::string, int>& wordCount, double wordFreq[TOP_N]) {
    int totalWords = 0;
    for (const auto& pair : wordCount) {
        totalWords += pair.second;
    }

    int index = 0;
    for (const auto& pair : wordCount) {
        if (index < TOP_N) {
            wordFreq[index++] = static_cast<double>(pair.second) / totalWords;
        } else {
            break;
        }
    }
}

double calculateSimilarity(const double freqA[TOP_N], const double freqB[TOP_N]) {
    double similarity = 0.0;
    for (int i = 0; i < TOP_N; ++i) {
        similarity += freqA[i] * freqB[i];
    }
    return similarity;
}

int main() {
    std::vector<std::string> bookNames;
    std::vector<std::unordered_map<std::string, int>> wordCounts;
    std::vector<double[TOP_N]> normalizedFrequencies;
    double similarityMatrix[64][64] = { 0 };
    
    // Read all text files from the current directory
    for (const auto& entry : std::filesystem::directory_iterator("Book-Txt")) {
        if (entry.path().extension() == ".txt") {
            bookNames.push_back(entry.path().filename().string());
            wordCounts.emplace_back();
            normalizedFrequencies.emplace_back();
            
            int totalWords = 0;
            countWords(entry.path().string(), wordCounts.back(), totalWords);
            calculateNormalizedFrequency(wordCounts.back(), normalizedFrequencies.back());
        }
    }

    size_t numBooks = bookNames.size();
    // Calculate the similarity matrix
    for (size_t i = 0; i < numBooks; ++i) {
        for (size_t j = i + 1; j < numBooks; ++j) {
            similarityMatrix[i][j] = calculateSimilarity(normalizedFrequencies[i], normalizedFrequencies[j]);
            similarityMatrix[j][i] = similarityMatrix[i][j]; // Symmetric matrix
        }
    }

    // Find top 10 similar pairs
    using SimilarityPair = std::pair<double, std::pair<int, int>>;
    std::vector<SimilarityPair> similarities;
    
    for (size_t i = 0; i < numBooks; ++i) {
        for (size_t j = i + 1; j < numBooks; ++j) {
            similarities.emplace_back(similarityMatrix[i][j], std::make_pair(i, j));
        }
    }

    std::sort(similarities.rbegin(), similarities.rend());

    std::cout << "Top 10 Similar Pairs of Textbooks:\n";
    for (int i = 0; i < 10 && i < similarities.size(); ++i) {
        int bookA = similarities[i].second.first;
        int bookB = similarities[i].second.second;
        std::cout << "Similarity: " << similarities[i].first
                  << " -> (" << bookNames[bookA] << ", " << bookNames[bookB] << ")\n";
    }

    return 0;
}
