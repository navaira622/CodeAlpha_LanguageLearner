#ifndef WORDENTRY_H
#define WORDENTRY_H

#include <QString>

struct WordEntry {
    int id;
    QString word;
    QString translation;
    QString example;
    QString pronunciation;
    QString difficulty;  // "easy", "medium", "hard"
    QString category;
    bool learned = false;

    // Pre-computed lowercase fields for fast case-insensitive search.
    // Populated once in DataManager::loadVocabulary() and never changed.
    QString wordLower;
    QString translationLower;
    QString categoryLower;
};

#endif // WORDENTRY_H
