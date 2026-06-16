# LinguaLearn — English · Spanish
## C++ / Qt Language Learning Application (Improved Edition)

### Build Requirements
- Qt 5.15+ or Qt 6.x (Widgets module)
- C++17 compiler (GCC, Clang, MSVC)
- qmake

### How to Build
```bash
cd LanguageLearnerImproved
qmake LanguageLearner.pro
make        # or nmake on Windows
./LanguageLearner
```

Or open `LanguageLearner.pro` in Qt Creator and click **Run**.

---

### What Was Improved

#### 1. Vocabulary Database — 222 words total
| Difficulty | Count | Requirement |
|------------|-------|-------------|
| Easy       | 68    | 50+ ✅      |
| Medium     | 103   | 100+ ✅     |
| Hard       | 51    | 50+ ✅      |

Every word includes: English word, Spanish translation, phonetic pronunciation, and an example sentence in both languages.

New categories added: **school**, **travel**, **daily_life**

#### 2. Lessons Section — Fully Rebuilt
- **Daily Lessons tab**: 7 lessons (one per day of the week), each with 10 rotating words based on the day's date seed
- **Topic Lessons tab**: 14 topic-based lessons covering Food, Travel, School, Daily Life, Greetings, Home, People, Places, Adjectives, Verbs, Transport, Nature, Numbers, and Advanced vocabulary
- Each lesson shows word list + detail panel with pronunciation and example sentence
- Progress bar tracks learned words per lesson
- Mark Complete button saves progress

#### 3. Data Loading — Always Works
- Vocabulary bundled as Qt resource (`:/data/vocabulary.json`) — loads without any file system path
- 8 fallback paths checked so it works from Qt Creator shadow builds, terminal, or installed

#### 4. UI Layout Fixes
- No overlapping widgets — all pages use proper `QVBoxLayout`/`QSplitter` layouts
- Responsive splitters with fixed minimum widths
- Scroll areas wrap long content pages

#### 5. Modern Multi-Color Theme
- **Indigo/Violet** primary (`#4f46e5` → `#7c3aed`)
- **Pink** accent on flashcards (`#ec4899`)
- **Cyan** for quiz stats (`#06b6d4`)
- **Emerald** for learned/success states (`#10b981`)
- Full **Dark Mode** support (toggle 🌙/☀️)
- Smooth fade-in page transitions

### App Sections
| Section | Description |
|---------|-------------|
| 🏠 Home | Dashboard with stats, streak, quick-action cards |
| 📖 Vocabulary | Full word table with search, filter by difficulty/category, word detail panel |
| 🃏 Flashcards | Flip cards with shuffle, difficulty filter |
| ✏️ Quiz | 10-question multiple-choice quiz with score tracking |
| 📅 Lessons | Daily + topic lessons (see above) |
| 📊 Progress | Vocabulary bar, streak calendar, quiz history |
