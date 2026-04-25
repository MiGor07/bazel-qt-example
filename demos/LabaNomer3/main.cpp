#include <QtWidgets>
#if __has_include(<QtMultimedia>)
#  include <QtMultimedia>
#  define LL_HAS_MULTIMEDIA 1
#else
#  define LL_HAS_MULTIMEDIA 0
#endif
#include <algorithm>
#include <random>
#include <vector>

enum class Difficulty { Easy, Medium, Hard };

struct TranslationTask {
    QString sourceText;
    QStringList acceptedAnswers;
    QString hint;
};

struct GrammarTask {
    QString question;
    QStringList options;
    int correctIndex;
    QString hint;
};

namespace Banks {

// Переводы: английский -> русский. Принимаем несколько вариантов.
static QList<TranslationTask> translationsEasy() {
    return {
        {"I love you",         {"Я тебя люблю", "я люблю тебя"},
         "Порядок слов в русском свободный: оба варианта верны."},
        {"Good morning",       {"Доброе утро"},
         "Приветствие, используется до полудня."},
        {"How are you?",       {"Как дела", "как ты", "как ты?", "как дела?"},
         "Дословно: «как ты?»."},
        {"The cat is black",   {"Кот чёрный", "кошка чёрная", "кот черный", "кошка черная"},
         "Род существительного определяет окончание прилагательного."},
        {"I am a student",     {"Я студент", "я студентка"},
         "В русском глагол-связка «есть» обычно опускается."},
    };
}
static QList<TranslationTask> translationsMedium() {
    return {
        {"I have been waiting for an hour", {"Я жду уже час", "я ждал уже час"},
         "Present Perfect Continuous: действие началось в прошлом и длится до сих пор."},
        {"She would like a cup of tea",     {"Она хотела бы чашку чая", "она хотела бы чашечку чая"},
         "Would like — вежливая форма «хотеть»."},
        {"We are going to the cinema",      {"Мы идём в кино", "мы идем в кино"},
         "Present Continuous с глаголами движения выражает ближайшие планы."},
        {"He can speak three languages",    {"Он говорит на трёх языках", "он говорит на трех языках",
                                             "он может говорить на трёх языках", "он может говорить на трех языках"},
         "Модальный глагол can."},
        {"What time is it?",                {"Который час", "который час?", "сколько времени", "сколько времени?"},
         "Два устоявшихся варианта в русском."},
    };
}
static QList<TranslationTask> translationsHard() {
    return {
        {"If I had known, I would have come earlier",
         {"Если бы я знал, я бы пришёл раньше", "если бы я знала, я бы пришла раньше",
          "если бы я знал я бы пришел раньше", "если бы я знала я бы пришла раньше"},
         "Conditional III: нереальное условие в прошлом."},
        {"The book had been read before the lecture started",
         {"Книга была прочитана до того, как началась лекция",
          "книгу прочитали до того, как началась лекция",
          "книга была прочитана до начала лекции"},
         "Past Perfect Passive."},
        {"She suggested going to the beach",
         {"Она предложила пойти на пляж", "она предложила сходить на пляж"},
         "После suggest — герундий."},
        {"I wish I could fly",
         {"Жаль, что я не умею летать", "хотел бы я уметь летать", "хотела бы я уметь летать"},
         "Конструкция I wish выражает сожаление."},
        {"Hardly had he arrived when it started raining",
         {"Едва он прибыл, как пошёл дождь", "едва он приехал, как начался дождь",
          "едва он прибыл, как начался дождь", "едва он приехал, как пошёл дождь"},
         "Инверсия после hardly."},
    };
}

static QList<GrammarTask> grammarEasy() {
    return {
        {"She ___ to school every day.",
         {"go", "goes", "going", "gone"}, 1,
         "3-е лицо ед. числа в Present Simple получает -s/-es."},
        {"They ___ playing football now.",
         {"is", "am", "are", "be"}, 2,
         "Present Continuous: to be + Ving. They → are."},
        {"I ___ a book yesterday.",
         {"read", "reads", "reading", "have read"}, 0,
         "Past Simple глагола read читается как [red]."},
        {"There ___ a cat on the sofa.",
         {"are", "is", "be", "am"}, 1,
         "There is/are: ед. число → is."},
        {"Can you ___ me, please?",
         {"helps", "helping", "help", "helped"}, 2,
         "После модального can — инфинитив без to."},
    };
}
static QList<GrammarTask> grammarMedium() {
    return {
        {"By the time we arrived, the film ___ .",
         {"started", "has started", "had started", "was starting"}, 2,
         "Past Perfect для действия, предшествующего другому в прошлом."},
        {"If it ___ tomorrow, we will stay home.",
         {"will rain", "rains", "rained", "would rain"}, 1,
         "Conditional I: if + Present Simple, will + инфинитив."},
        {"I'm not used to ___ up early.",
         {"get", "got", "getting", "be get"}, 2,
         "Be used to + герундий."},
        {"The letter ___ yesterday.",
         {"was sent", "was send", "sent", "is sent"}, 0,
         "Past Simple Passive: was/were + V3."},
        {"She has lived here ___ 2010.",
         {"for", "since", "from", "during"}, 1,
         "Since — с указанием момента, for — с промежутком."},
    };
}
static QList<GrammarTask> grammarHard() {
    return {
        {"I wish I ___ more time yesterday.",
         {"have", "had", "had had", "would have"}, 2,
         "I wish + Past Perfect — сожаление о прошлом."},
        {"Hardly ___ the door when the phone rang.",
         {"I had opened", "had I opened", "I opened", "did I open"}, 1,
         "Инверсия после hardly."},
        {"The project ___ by next Monday.",
         {"will finish", "will be finished", "will have been finished", "finishes"}, 2,
         "Future Perfect Passive: by + будущее время."},
        {"He denied ___ the window.",
         {"to break", "breaking", "broken", "break"}, 1,
         "После deny — герундий."},
        {"Not only ___ the exam, but she also got the top grade.",
         {"she passed", "did she pass", "she did pass", "passed she"}, 1,
         "Инверсия после not only."},
    };
}

} // namespace Banks


// Advanced string comparison: нормализация + расстояние Левенштейна.
class StringMatcher {
public:
    static QString normalize(const QString &s) {
        QString out;
        out.reserve(s.size());
        for (QChar c : s) {
            if (c.isSpace()) {
                if (!out.isEmpty() && !out.endsWith(' '))
                    out.append(' ');
                continue;
            }
            if (c.isPunct()) continue;
            // заменим «ё» на «е», чтобы не штрафовать
            if (c == QChar(0x0451)) { out.append(QChar(0x0435)); continue; } // ё → е
            if (c == QChar(0x0401)) { out.append(QChar(0x0435)); continue; } // Ё → е
            out.append(c.toLower());
        }
        return out.trimmed();
    }

    static int levenshtein(const QString &a, const QString &b) {
        const int n = a.size(), m = b.size();
        if (n == 0) return m;
        if (m == 0) return n;
        std::vector<int> prev(m + 1), cur(m + 1);
        for (int j = 0; j <= m; ++j) prev[j] = j;
        for (int i = 1; i <= n; ++i) {
            cur[0] = i;
            for (int j = 1; j <= m; ++j) {
                int cost = (a[i - 1] == b[j - 1]) ? 0 : 1;
                cur[j] = std::min({ prev[j] + 1, cur[j - 1] + 1, prev[j - 1] + cost });
            }
            std::swap(prev, cur);
        }
        return prev[m];
    }

    // Возвращает пару: (точное ли совпадение после нормализации, достаточно ли близкое)
    struct Result {
        bool exact;
        bool close;
        int distance;
        int threshold;
    };

    static Result compare(const QString &user, const QStringList &accepted) {
        QString u = normalize(user);
        Result best{false, false, INT_MAX, 0};
        for (const QString &a : accepted) {
            QString na = normalize(a);
            if (u == na) return {true, true, 0, 0};
            // также пробуем перестановку слов: сортировка токенов
            QStringList tu = u.split(' ', Qt::SkipEmptyParts);
            QStringList ta = na.split(' ', Qt::SkipEmptyParts);
            std::sort(tu.begin(), tu.end());
            std::sort(ta.begin(), ta.end());
            if (tu == ta) return {true, true, 0, 0};

            int d = levenshtein(u, na);
            int thr = std::max(1, static_cast<int>(na.size() * 0.15));
            if (d < best.distance) {
                best.distance = d;
                best.threshold = thr;
                best.close = (d <= thr);
            }
        }
        return best;
    }
};


// ---- Диалог выбора уровня сложности ----
class DifficultyDialog : public QDialog {
public:
    DifficultyDialog(Difficulty current, QWidget *parent = nullptr)
        : QDialog(parent) {
        setWindowTitle("Выбор уровня сложности");
        auto *lay = new QVBoxLayout(this);
        lay->addWidget(new QLabel("Выберите уровень:"));

        group = new QButtonGroup(this);
        auto *easy   = new QRadioButton("Easy — короткие фразы, базовая грамматика");
        auto *med    = new QRadioButton("Medium — времена, пассив, условные I/II");
        auto *hard   = new QRadioButton("Hard — инверсии, wish, perfect passive");
        group->addButton(easy, 0);
        group->addButton(med,  1);
        group->addButton(hard, 2);
        lay->addWidget(easy);
        lay->addWidget(med);
        lay->addWidget(hard);

        switch (current) {
            case Difficulty::Easy:   easy->setChecked(true); break;
            case Difficulty::Medium: med->setChecked(true);  break;
            case Difficulty::Hard:   hard->setChecked(true); break;
        }

        auto *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        connect(box, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);
        lay->addWidget(box);
    }

    Difficulty chosen() const {
        switch (group->checkedId()) {
            case 0: return Difficulty::Easy;
            case 2: return Difficulty::Hard;
            default: return Difficulty::Medium;
        }
    }
private:
    QButtonGroup *group;
};


// ---- Основное окно ----
class MainWindow : public QMainWindow {
public:
    MainWindow() {
        setWindowTitle("LinguaDuo — Language Learning");
        resize(980, 620);

        buildMenu();
        buildCentral();
        buildStatus();
        applyDarkTheme();

        // Горячая клавиша Help
        auto *helpShort = new QShortcut(QKeySequence(Qt::Key_H), this);
        connect(helpShort, &QShortcut::activated, this, &MainWindow::showHelp);

        // Таймер обратного отсчёта
        uiTimer = new QTimer(this);
        uiTimer->setInterval(1000);
        connect(uiTimer, &QTimer::timeout, this, &MainWindow::onTick);

        // Аудио для обратной связи (bonus)
#if LL_HAS_MULTIMEDIA
        correctPlayer = new QMediaPlayer(this);
        wrongPlayer   = new QMediaPlayer(this);
        auto *ao1 = new QAudioOutput(this);
        auto *ao2 = new QAudioOutput(this);
        ao1->setVolume(0.6);
        ao2->setVolume(0.6);
        correctPlayer->setAudioOutput(ao1);
        wrongPlayer->setAudioOutput(ao2);
        QString beep = QCoreApplication::applicationDirPath() + "/01.mp3";
        if (QFile::exists(beep)) {
            correctPlayer->setSource(QUrl::fromLocalFile(beep));
            wrongPlayer->setSource(QUrl::fromLocalFile(beep));
        }
#endif

        showWelcome();
        updateStatusBar();
    }

private:
    // ---- UI ----
    void buildMenu() {
        auto *mb = menuBar();

        auto *fileMenu = mb->addMenu("&Файл");
        auto *quitAct = fileMenu->addAction("Выход");
        quitAct->setShortcut(QKeySequence::Quit);
        connect(quitAct, &QAction::triggered, this, &QWidget::close);

        auto *settings = mb->addMenu("&Настройки");
        auto *diffAct = settings->addAction("Уровень сложности…");
        connect(diffAct, &QAction::triggered, this, &MainWindow::pickDifficulty);

        auto *helpMenu = mb->addMenu("&Помощь");
        auto *helpAct = helpMenu->addAction("Подсказка к заданию  (H)");
        connect(helpAct, &QAction::triggered, this, &MainWindow::showHelp);
        helpMenu->addSeparator();
        auto *aboutAct = helpMenu->addAction("О программе");
        connect(aboutAct, &QAction::triggered, this, [this]{
            QMessageBox::about(this, "О программе",
                "LinguaDuo\n\n"
                "Приложение для изучения английского языка.\n"
                "Лабораторная работа №3 по Qt.\n"
                "Упражнения: Translation, Grammar.\n"
                "Горячая клавиша подсказки: H.");
        });
    }

    void buildCentral() {
        auto *central = new QWidget;
        setCentralWidget(central);

        auto *root = new QHBoxLayout(central);
        root->setContentsMargins(12, 12, 12, 12);
        root->setSpacing(12);

        // --- Левая панель меню ---
        auto *leftBox = new QGroupBox("📚 Меню");
        leftBox->setObjectName("leftBox");
        auto *leftLay = new QVBoxLayout(leftBox);
        leftLay->setSpacing(10);

        translationBtn = new QPushButton("📖 Translation");
        translationBtn->setObjectName("translationBtn");
        grammarBtn     = new QPushButton("📝 Grammar");
        grammarBtn->setObjectName("grammarBtn");
        translationBtn->setMinimumHeight(48);
        grammarBtn->setMinimumHeight(48);
        translationBtn->setCursor(Qt::PointingHandCursor);
        grammarBtn->setCursor(Qt::PointingHandCursor);

        leftLay->addWidget(translationBtn);
        leftLay->addWidget(grammarBtn);
        leftLay->addSpacing(8);

        auto *settingsLabel = new QLabel();
        settingsLabel->setObjectName("difficultyLabel");
        settingsLabel->setWordWrap(true);
        settingsLabel->setTextFormat(Qt::RichText);
        difficultyLabel = settingsLabel;
        leftLay->addWidget(settingsLabel);

        leftLay->addStretch();

        auto *scoreBox = new QGroupBox("⭐ Ваш прогресс");
        scoreBox->setObjectName("scoreBox");
        auto *scoreLay = new QVBoxLayout(scoreBox);
        scoreLabel = new QLabel("Баллы: 0");
        scoreLabel->setObjectName("scoreLabel");
        QFont f = scoreLabel->font(); f.setBold(true); f.setPointSize(f.pointSize() + 4);
        scoreLabel->setFont(f);
        scoreLabel->setAlignment(Qt::AlignCenter);
        scoreLay->addWidget(scoreLabel);
        leftLay->addWidget(scoreBox);

        root->addWidget(leftBox, 0);
        leftBox->setMinimumWidth(260);

        // --- Правая часть: верхняя инфо-строка + stacked ---
        auto *rightWrap = new QVBoxLayout;
        root->addLayout(rightWrap, 1);

        auto *topLay = new QHBoxLayout;
        topLay->setSpacing(10);
        progressBar = new QProgressBar;
        progressBar->setObjectName("progressBar");
        progressBar->setFormat("Задание %v / %m");
        progressBar->setRange(0, 1);
        progressBar->setValue(0);
        progressBar->setMinimumHeight(28);
        timerLabel = new QLabel("⏱ --:--");
        timerLabel->setObjectName("timerLabel");
        timerLabel->setAlignment(Qt::AlignCenter);
        timerLabel->setMinimumWidth(110);
        QFont tf = timerLabel->font(); tf.setBold(true); tf.setPointSize(tf.pointSize() + 1);
        timerLabel->setFont(tf);
        livesLabel = new QLabel("❤ ❤ ❤");
        livesLabel->setObjectName("livesLabel");
        livesLabel->setAlignment(Qt::AlignCenter);
        livesLabel->setMinimumWidth(110);
        QFont lf = livesLabel->font(); lf.setPointSize(lf.pointSize() + 2);
        livesLabel->setFont(lf);

        topLay->addWidget(progressBar, 1);
        topLay->addWidget(livesLabel);
        topLay->addWidget(timerLabel);
        rightWrap->addLayout(topLay);

        stack = new QStackedWidget;
        rightWrap->addWidget(stack, 1);

        // welcome page
        welcomePage = buildWelcomePage();
        stack->addWidget(welcomePage);

        // translation page
        translationPage = buildTranslationPage();
        stack->addWidget(translationPage);

        // grammar page
        grammarPage = buildGrammarPage();
        stack->addWidget(grammarPage);

        connect(translationBtn, &QPushButton::clicked, this, [this]{ startExercise(ExerciseKind::Translation); });
        connect(grammarBtn,     &QPushButton::clicked, this, [this]{ startExercise(ExerciseKind::Grammar); });

        refreshDifficultyLabel();
    }

    QWidget *buildWelcomePage() {
        auto *w = new QWidget;
        w->setObjectName("welcomePage");
        auto *lay = new QVBoxLayout(w);
        lay->addStretch();

        auto *emoji = new QLabel("🦜");
        emoji->setAlignment(Qt::AlignCenter);
        QFont ef = emoji->font(); ef.setPointSize(64);
        emoji->setFont(ef);
        lay->addWidget(emoji);

        auto *title = new QLabel("Добро пожаловать в LinguaDuo!");
        title->setObjectName("welcomeTitle");
        QFont tf = title->font(); tf.setPointSize(tf.pointSize() + 10); tf.setBold(true);
        title->setFont(tf);
        title->setAlignment(Qt::AlignCenter);
        lay->addWidget(title);

        auto *desc = new QLabel(
            "<p style='line-height:160%;'>"
            "Выберите упражнение в меню слева.<br><br>"
            "<span style='color:#a6e3a1;'>📖 Translation</span> — перевод фраз с английского<br>"
            "<span style='color:#cba6f7;'>📝 Grammar</span> — выбор правильной формы<br><br>"
            "<span style='color:#f9e2af;'>Клавиша H</span> — подсказка к текущему заданию<br>"
            "Уровень сложности — в меню <b>«Настройки»</b>."
            "</p>");
        desc->setObjectName("welcomeDesc");
        desc->setTextFormat(Qt::RichText);
        desc->setAlignment(Qt::AlignCenter);
        QFont df = desc->font(); df.setPointSize(df.pointSize() + 1);
        desc->setFont(df);
        lay->addWidget(desc);
        lay->addStretch();
        return w;
    }

    QWidget *buildTranslationPage() {
        auto *w = new QWidget;
        auto *lay = new QVBoxLayout(w);
        lay->setSpacing(10);

        auto *hdr = new QLabel("📖  Переведите на русский:");
        hdr->setObjectName("subHeader");
        lay->addWidget(hdr);

        trSource = new QLabel;
        trSource->setObjectName("translationSource");
        QFont f = trSource->font(); f.setPointSize(f.pointSize() + 6); f.setBold(true);
        trSource->setFont(f);
        trSource->setWordWrap(true);
        trSource->setAlignment(Qt::AlignCenter);
        trSource->setMinimumHeight(80);
        lay->addWidget(trSource);

        trEdit = new QTextEdit;
        trEdit->setObjectName("translationEdit");
        trEdit->setPlaceholderText("Введите перевод здесь…");
        lay->addWidget(trEdit, 1);

        trFeedback = new QLabel(" ");
        trFeedback->setObjectName("feedback");
        trFeedback->setWordWrap(true);
        QFont ff = trFeedback->font(); ff.setPointSize(ff.pointSize() + 1); ff.setBold(true);
        trFeedback->setFont(ff);
        lay->addWidget(trFeedback);

        auto *btnRow = new QHBoxLayout;
        btnRow->addStretch();
        trSubmit = new QPushButton("✓ Submit");
        trSubmit->setObjectName("submitBtn");
        trSubmit->setCursor(Qt::PointingHandCursor);
        trSubmit->setMinimumHeight(40);
        trSubmit->setMinimumWidth(140);
        btnRow->addWidget(trSubmit);
        lay->addLayout(btnRow);

        connect(trSubmit, &QPushButton::clicked, this, &MainWindow::submitTranslation);
        return w;
    }

    QWidget *buildGrammarPage() {
        auto *w = new QWidget;
        auto *lay = new QVBoxLayout(w);
        lay->setSpacing(10);

        auto *hdr = new QLabel("📝  Выберите правильный вариант:");
        hdr->setObjectName("subHeader");
        lay->addWidget(hdr);

        grQuestion = new QLabel;
        grQuestion->setObjectName("grammarQuestion");
        QFont f = grQuestion->font(); f.setPointSize(f.pointSize() + 6); f.setBold(true);
        grQuestion->setFont(f);
        grQuestion->setWordWrap(true);
        grQuestion->setAlignment(Qt::AlignCenter);
        grQuestion->setMinimumHeight(80);
        lay->addWidget(grQuestion);

        grGroup = new QButtonGroup(this);
        auto *optsBox = new QGroupBox("Варианты ответа");
        optsBox->setObjectName("grammarOptsBox");
        grOptsLayout = new QVBoxLayout(optsBox);
        grOptsLayout->setSpacing(4);
        for (int i = 0; i < 4; ++i) {
            auto *rb = new QRadioButton;
            rb->setObjectName("grammarOption");
            rb->setCursor(Qt::PointingHandCursor);
            QFont rf = rb->font(); rf.setPointSize(rf.pointSize() + 1);
            rb->setFont(rf);
            grGroup->addButton(rb, i);
            grOptsLayout->addWidget(rb);
            grOptions.push_back(rb);
        }
        lay->addWidget(optsBox);

        grFeedback = new QLabel(" ");
        grFeedback->setObjectName("feedback");
        grFeedback->setWordWrap(true);
        QFont ff = grFeedback->font(); ff.setPointSize(ff.pointSize() + 1); ff.setBold(true);
        grFeedback->setFont(ff);
        lay->addWidget(grFeedback);

        auto *btnRow = new QHBoxLayout;
        btnRow->addStretch();
        grSubmit = new QPushButton("✓ Submit");
        grSubmit->setObjectName("submitBtn");
        grSubmit->setCursor(Qt::PointingHandCursor);
        grSubmit->setMinimumHeight(40);
        grSubmit->setMinimumWidth(140);
        btnRow->addWidget(grSubmit);
        lay->addLayout(btnRow);

        connect(grSubmit, &QPushButton::clicked, this, &MainWindow::submitGrammar);
        return w;
    }

    void buildStatus() {
        statusBar()->showMessage("Готов к работе.");
    }

    void applyDarkTheme() {
        // Палитра Catppuccin Mocha + ярко-цветные акценты на виджетах.
        qApp->setStyle("Fusion");
        QPalette p;
        const QColor mantle    (0x18, 0x18, 0x25);  // окно
        const QColor base      (0x1e, 0x1e, 0x2e);  // поля ввода
        const QColor surface0  (0x31, 0x32, 0x44);  // подложка кнопок
        const QColor surface1  (0x45, 0x47, 0x5a);
        const QColor text      (0xcd, 0xd6, 0xf4);
        const QColor subtext   (0xa6, 0xad, 0xc8);
        const QColor lavender  (0xb4, 0xbe, 0xfe);
        const QColor mauve     (0xcb, 0xa6, 0xf7);

        p.setColor(QPalette::Window,           mantle);
        p.setColor(QPalette::WindowText,       text);
        p.setColor(QPalette::Base,             base);
        p.setColor(QPalette::AlternateBase,    surface0);
        p.setColor(QPalette::ToolTipBase,      base);
        p.setColor(QPalette::ToolTipText,      text);
        p.setColor(QPalette::Text,             text);
        p.setColor(QPalette::Button,           surface0);
        p.setColor(QPalette::ButtonText,       text);
        p.setColor(QPalette::BrightText,       lavender);
        p.setColor(QPalette::Link,             lavender);
        p.setColor(QPalette::Highlight,        mauve);
        p.setColor(QPalette::HighlightedText,  mantle);
        p.setColor(QPalette::PlaceholderText,  subtext);
        p.setColor(QPalette::Disabled, QPalette::Text,        surface1);
        p.setColor(QPalette::Disabled, QPalette::ButtonText,  surface1);
        p.setColor(QPalette::Disabled, QPalette::WindowText,  surface1);
        qApp->setPalette(p);

        qApp->setStyleSheet(R"CSS(
        /* ───────── Базовые цвета ───────── */
        QWidget                 { color: #cdd6f4; }
        QMainWindow             { background: #181825; }
        QToolTip                { color: #cdd6f4; background: #1e1e2e;
                                  border: 1px solid #cba6f7; padding: 4px; border-radius: 4px; }

        /* ───────── Меню сверху ───────── */
        QMenuBar                { background: #11111b; color: #cdd6f4; padding: 2px; }
        QMenuBar::item          { padding: 6px 12px; background: transparent; border-radius: 4px; }
        QMenuBar::item:selected { background: #313244; color: #b4befe; }
        QMenu                   { background: #1e1e2e; color: #cdd6f4;
                                  border: 1px solid #45475a; padding: 4px; }
        QMenu::item             { padding: 6px 22px; border-radius: 4px; }
        QMenu::item:selected    { background: #313244; color: #b4befe; }
        QMenu::separator        { height: 1px; background: #45475a; margin: 4px 8px; }

        /* ───────── GroupBox ───────── */
        QGroupBox {
            border: 1px solid #45475a;
            border-radius: 10px;
            margin-top: 18px;
            padding: 14px 10px 10px 10px;
            background: #1e1e2e;
            font-weight: bold;
            color: #b4befe;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            left: 12px;
            padding: 0 8px;
            background: #181825;
            color: #b4befe;
        }

        /* Левая панель: чуть синее, цветной заголовок */
        QGroupBox#leftBox {
            background: qlineargradient(x1:0,y1:0,x2:0,y2:1,
                stop:0 #1e1e2e, stop:1 #181825);
            border: 1px solid #585b70;
        }

        /* Score box — золотая рамка */
        QGroupBox#scoreBox {
            border: 2px solid #f9e2af;
            background: qlineargradient(x1:0,y1:0,x2:0,y2:1,
                stop:0 #2a2533, stop:1 #1e1e2e);
        }
        QGroupBox#scoreBox::title { color: #f9e2af; }
        QLabel#scoreLabel { color: #f9e2af; padding: 4px; }

        /* Difficulty info — лавандовая полоса слева */
        QLabel#difficultyLabel {
            color: #cdd6f4;
            background: #313244;
            border-left: 4px solid #b4befe;
            border-radius: 6px;
            padding: 8px 10px;
        }

        /* ───────── Кнопки общие ───────── */
        QPushButton {
            border: none;
            border-radius: 8px;
            padding: 8px 16px;
            background: #313244;
            color: #cdd6f4;
            font-weight: bold;
        }
        QPushButton:hover    { background: #45475a; }
        QPushButton:pressed  { background: #585b70; }
        QPushButton:disabled { background: #1e1e2e; color: #585b70; }

        /* Translation — изумрудно-бирюзовый градиент */
        QPushButton#translationBtn {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1,
                stop:0 #94e2d5, stop:1 #a6e3a1);
            color: #11111b;
            font-size: 16px;
        }
        QPushButton#translationBtn:hover {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1,
                stop:0 #b3ecdf, stop:1 #c4ebbf);
        }
        QPushButton#translationBtn:pressed {
            background: #74c7ec; color: #11111b;
        }

        /* Grammar — лилово-розовый градиент */
        QPushButton#grammarBtn {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1,
                stop:0 #cba6f7, stop:1 #f5c2e7);
            color: #11111b;
            font-size: 16px;
        }
        QPushButton#grammarBtn:hover {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1,
                stop:0 #d8baf9, stop:1 #facfee);
        }
        QPushButton#grammarBtn:pressed {
            background: #f38ba8; color: #11111b;
        }

        /* Submit — небесно-синий градиент */
        QPushButton#submitBtn {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1,
                stop:0 #89b4fa, stop:1 #74c7ec);
            color: #11111b;
            font-size: 15px;
        }
        QPushButton#submitBtn:hover {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1,
                stop:0 #a3c5fb, stop:1 #92d3f0);
        }
        QPushButton#submitBtn:pressed { background: #b4befe; }

        /* ───────── Поля ввода ───────── */
        QTextEdit, QLineEdit {
            background: #11111b;
            color: #cdd6f4;
            selection-background-color: #cba6f7;
            selection-color: #11111b;
            border: 2px solid #45475a;
            border-radius: 8px;
            padding: 8px;
        }
        QTextEdit:focus, QLineEdit:focus { border-color: #89b4fa; }

        /* ───────── Progress bar ───────── */
        QProgressBar {
            border: 1px solid #45475a;
            border-radius: 8px;
            background: #1e1e2e;
            text-align: center;
            color: #cdd6f4;
            font-weight: bold;
        }
        QProgressBar::chunk {
            border-radius: 7px;
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #a6e3a1, stop:0.5 #94e2d5, stop:1 #89dceb);
        }

        /* ───────── Жизни и таймер ───────── */
        QLabel#livesLabel {
            color: #f38ba8;
            background: #2b1d24;
            border: 1px solid #f38ba8;
            border-radius: 8px;
            padding: 4px 10px;
            font-weight: bold;
        }
        QLabel#timerLabel {
            color: #fab387;
            background: #2c2419;
            border: 1px solid #fab387;
            border-radius: 8px;
            padding: 4px 10px;
        }

        /* ───────── Заголовок задачи ───────── */
        QLabel#translationSource {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1,
                stop:0 #1e2a2a, stop:1 #163338);
            border: 2px solid #94e2d5;
            border-radius: 12px;
            padding: 18px;
            color: #cdd6f4;
        }
        QLabel#grammarQuestion {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1,
                stop:0 #2a1f3a, stop:1 #2e2440);
            border: 2px solid #cba6f7;
            border-radius: 12px;
            padding: 18px;
            color: #cdd6f4;
        }
        QLabel#subHeader { color: #b4befe; font-weight: bold; padding: 2px 0; }

        /* ───────── Welcome page ───────── */
        QLabel#welcomeTitle {
            color: #b4befe;
            padding: 8px;
        }
        QLabel#welcomeDesc { color: #cdd6f4; }

        /* ───────── Радио-кнопки ───────── */
        QRadioButton#grammarOption {
            padding: 10px 12px;
            background: #313244;
            border: 2px solid transparent;
            border-radius: 8px;
            color: #cdd6f4;
            spacing: 10px;
        }
        QRadioButton#grammarOption:hover {
            background: #45475a;
            border-color: #cba6f7;
        }
        QRadioButton#grammarOption:checked {
            background: #2e2440;
            border-color: #cba6f7;
            color: #f5c2e7;
        }
        QRadioButton::indicator {
            width: 16px; height: 16px; border-radius: 8px;
            border: 2px solid #6c7086; background: #1e1e2e;
        }
        QRadioButton::indicator:hover    { border-color: #cba6f7; }
        QRadioButton::indicator:checked {
            border: 2px solid #cba6f7;
            background: qradialgradient(cx:0.5, cy:0.5, radius:0.5,
                fx:0.5, fy:0.5, stop:0 #f5c2e7, stop:0.5 #cba6f7, stop:1 #1e1e2e);
        }

        /* ───────── Status bar ───────── */
        QStatusBar { background: #11111b; color: #a6adc8; border-top: 1px solid #313244; }

        /* ───────── Scrollbars (тонкие, в тон) ───────── */
        QScrollBar:vertical   { background: #1e1e2e; width: 10px; }
        QScrollBar:horizontal { background: #1e1e2e; height: 10px; }
        QScrollBar::handle    { background: #45475a; border-radius: 5px; min-height: 20px; min-width: 20px; }
        QScrollBar::handle:hover { background: #585b70; }
        QScrollBar::add-line, QScrollBar::sub-line { background: none; border: none; height: 0; width: 0; }

        /* ───────── Диалоги ───────── */
        QDialog { background: #181825; }
        QMessageBox { background: #1e1e2e; }
        QDialogButtonBox QPushButton { min-width: 80px; }
        )CSS");
    }

    // ---- Логика упражнений ----
    enum class ExerciseKind { None, Translation, Grammar };

    void showWelcome() {
        stack->setCurrentWidget(welcomePage);
        progressBar->setRange(0, 1);
        progressBar->setValue(0);
        livesLabel->setText("");
        timerLabel->setText("⏱ --:--");
        uiTimer->stop();
    }

    void startExercise(ExerciseKind kind) {
        currentKind = kind;
        taskIndex = 0;
        wrongAttempts = 0;
        maxWrong = 3;

        if (kind == ExerciseKind::Translation) {
            auto pool = banksTranslation();
            std::shuffle(pool.begin(), pool.end(), rng());
            translationTasks = pool.mid(0, std::min<int>(5, pool.size()));
            stack->setCurrentWidget(translationPage);
        } else {
            auto pool = banksGrammar();
            std::shuffle(pool.begin(), pool.end(), rng());
            grammarTasks = pool.mid(0, std::min<int>(5, pool.size()));
            stack->setCurrentWidget(grammarPage);
        }

        int total = (kind == ExerciseKind::Translation)
                        ? translationTasks.size()
                        : grammarTasks.size();
        progressBar->setRange(0, total);
        progressBar->setValue(0);

        secondsLeft = totalTimeForCurrent();
        uiTimer->start();
        updateTimerLabel();
        updateLives();

        loadCurrentTask();
        statusBar()->showMessage(QString("Упражнение началось. Заданий: %1.").arg(total));
    }

    void finishExercise(bool success, const QString &reason) {
        uiTimer->stop();
        int awarded = 0;
        if (success) {
            awarded = pointsForDifficulty() * currentExerciseTotal();
            score += awarded;
        }
        updateStatusBar();

        QString msg;
        if (success) {
            msg = QString("Отлично! Все задания выполнены.\nНачислено: %1 баллов.").arg(awarded);
        } else {
            msg = reason;
            if (msg.isEmpty()) msg = "Упражнение завершено.";
            msg += "\nБаллы начисляются только за полное корректное выполнение.";
        }
        QMessageBox::information(this, success ? "Победа!" : "Упражнение завершено", msg);
        showWelcome();
    }

    int currentExerciseTotal() const {
        return (currentKind == ExerciseKind::Translation)
                   ? translationTasks.size()
                   : grammarTasks.size();
    }

    void loadCurrentTask() {
        if (currentKind == ExerciseKind::Translation) {
            const auto &t = translationTasks[taskIndex];
            trSource->setText(t.sourceText);
            trEdit->clear();
            trEdit->setFocus();
            trFeedback->setText(" ");
            trFeedback->setStyleSheet("");
        } else {
            const auto &t = grammarTasks[taskIndex];
            grQuestion->setText(t.question);
            for (int i = 0; i < grOptions.size(); ++i) {
                if (i < t.options.size()) {
                    grOptions[i]->setText(t.options[i]);
                    grOptions[i]->setVisible(true);
                    grOptions[i]->setChecked(false);
                } else {
                    grOptions[i]->setVisible(false);
                }
            }
            grGroup->setExclusive(false);
            for (auto *rb : grOptions) rb->setChecked(false);
            grGroup->setExclusive(true);
            grFeedback->setText(" ");
            grFeedback->setStyleSheet("");
        }
    }

    void submitTranslation() {
        if (currentKind != ExerciseKind::Translation) return;
        const auto &t = translationTasks[taskIndex];
        QString user = trEdit->toPlainText();
        if (user.trimmed().isEmpty()) {
            trFeedback->setText("Введите перевод перед отправкой.");
            trFeedback->setStyleSheet("color: #f9e2af; background: #2c2419; "
                                       "border: 1px solid #f9e2af; border-radius: 6px; padding: 8px;");
            return;
        }

        auto r = StringMatcher::compare(user, t.acceptedAnswers);
        if (r.exact) {
            playCorrect();
            trFeedback->setText("✅ Верно!");
            trFeedback->setStyleSheet("color: #a6e3a1; background: #1d2a22; "
                                       "border: 1px solid #a6e3a1; border-radius: 6px; padding: 8px;");
            advanceTask();
        } else if (r.close) {
            playCorrect();
            trFeedback->setText(QString("✅ Принято (мелкие расхождения, расстояние %1). "
                                        "Эталон: «%2»")
                                    .arg(r.distance).arg(t.acceptedAnswers.first()));
            trFeedback->setStyleSheet("color: #94e2d5; background: #1c2929; "
                                       "border: 1px solid #94e2d5; border-radius: 6px; padding: 8px;");
            advanceTask();
        } else {
            registerWrong();
            trFeedback->setText(QString("❌ Неверно. Попробуйте ещё раз. (жизней осталось: %1)")
                                    .arg(maxWrong - wrongAttempts));
            trFeedback->setStyleSheet("color: #f38ba8; background: #2b1d24; "
                                       "border: 1px solid #f38ba8; border-radius: 6px; padding: 8px;");
            playWrong();
        }
    }

    void submitGrammar() {
        if (currentKind != ExerciseKind::Grammar) return;
        int id = grGroup->checkedId();
        if (id < 0) {
            grFeedback->setText("Выберите вариант перед отправкой.");
            grFeedback->setStyleSheet("color: #f9e2af; background: #2c2419; "
                                       "border: 1px solid #f9e2af; border-radius: 6px; padding: 8px;");
            return;
        }
        const auto &t = grammarTasks[taskIndex];
        if (id == t.correctIndex) {
            playCorrect();
            grFeedback->setText("✅ Верно!");
            grFeedback->setStyleSheet("color: #a6e3a1; background: #1d2a22; "
                                       "border: 1px solid #a6e3a1; border-radius: 6px; padding: 8px;");
            advanceTask();
        } else {
            registerWrong();
            grFeedback->setText(QString("❌ Неверно. Правильно: «%1». (жизней осталось: %2)")
                                    .arg(t.options[t.correctIndex])
                                    .arg(maxWrong - wrongAttempts));
            grFeedback->setStyleSheet("color: #f38ba8; background: #2b1d24; "
                                       "border: 1px solid #f38ba8; border-radius: 6px; padding: 8px;");
            playWrong();
        }
    }

    void advanceTask() {
        taskIndex++;
        progressBar->setValue(taskIndex);
        if (taskIndex >= currentExerciseTotal()) {
            finishExercise(true, "");
            return;
        }
        loadCurrentTask();
    }

    void registerWrong() {
        wrongAttempts++;
        updateLives();
        if (wrongAttempts >= maxWrong) {
            finishExercise(false,
                QString("Исчерпан лимит неверных попыток (%1). Упражнение завершено.")
                    .arg(maxWrong));
        }
    }

    void updateLives() {
        int left = std::max(0, maxWrong - wrongAttempts);
        QStringList parts;
        for (int i = 0; i < left; ++i) parts << "❤";
        for (int i = left; i < maxWrong; ++i) parts << "♡";
        livesLabel->setText(parts.join("  "));
    }

    void onTick() {
        secondsLeft--;
        if (secondsLeft <= 0) {
            uiTimer->stop();
            timerLabel->setText("⏱ 00:00");
            finishExercise(false, "⏰ Время, отведённое на выполнение задания, истекло!");
            return;
        }
        updateTimerLabel();
    }

    void updateTimerLabel() {
        int m = secondsLeft / 60;
        int s = secondsLeft % 60;
        timerLabel->setText(QString("⏱ %1:%2")
                                .arg(m, 2, 10, QChar('0'))
                                .arg(s, 2, 10, QChar('0')));
    }

    // ---- Меню/настройки ----
    void pickDifficulty() {
        DifficultyDialog dlg(difficulty, this);
        if (dlg.exec() == QDialog::Accepted) {
            difficulty = dlg.chosen();
            refreshDifficultyLabel();
            updateStatusBar();
            if (currentKind != ExerciseKind::None && uiTimer->isActive()) {
                QMessageBox::information(this, "Уровень изменён",
                    "Новый уровень применится при следующем запуске упражнения.");
            }
        }
    }

    void refreshDifficultyLabel() {
        QString d, color;
        switch (difficulty) {
            case Difficulty::Easy:   d = "Easy";   color = "#a6e3a1"; break;
            case Difficulty::Medium: d = "Medium"; color = "#f9e2af"; break;
            case Difficulty::Hard:   d = "Hard";   color = "#f38ba8"; break;
        }
        difficultyLabel->setText(QString(
            "<b style='color:#b4befe;'>Уровень:</b> "
            "<b style='color:%1;'>%2</b><br>"
            "<span style='color:#fab387;'>⏱ Время:</span> %3 сек<br>"
            "<span style='color:#f38ba8;'>❤ Жизни:</span> 3<br>"
            "<span style='color:#f9e2af;'>★ Баллы / задание:</span> %4")
            .arg(color).arg(d)
            .arg(totalTimeForCurrent())
            .arg(pointsForDifficulty()));
    }

    void showHelp() {
        QString hint = "Подсказки сейчас нет. Запустите упражнение.";
        if (currentKind == ExerciseKind::Translation && taskIndex < translationTasks.size()) {
            hint = translationTasks[taskIndex].hint;
        } else if (currentKind == ExerciseKind::Grammar && taskIndex < grammarTasks.size()) {
            hint = grammarTasks[taskIndex].hint;
        }
        QMessageBox box(this);
        box.setWindowTitle("Подсказка");
        box.setIcon(QMessageBox::Information);
        box.setText(hint);
        box.exec();
    }

    void updateStatusBar() {
        scoreLabel->setText(QString("Баллы: %1").arg(score));
        statusBar()->showMessage(QString("Всего баллов: %1").arg(score));
    }

    int totalTimeForCurrent() const {
        switch (difficulty) {
            case Difficulty::Easy:   return 120;
            case Difficulty::Medium: return 90;
            case Difficulty::Hard:   return 60;
        }
        return 90;
    }
    int pointsForDifficulty() const {
        switch (difficulty) {
            case Difficulty::Easy:   return 1;
            case Difficulty::Medium: return 2;
            case Difficulty::Hard:   return 3;
        }
        return 1;
    }

    QList<TranslationTask> banksTranslation() const {
        switch (difficulty) {
            case Difficulty::Easy:   return Banks::translationsEasy();
            case Difficulty::Medium: return Banks::translationsMedium();
            case Difficulty::Hard:   return Banks::translationsHard();
        }
        return {};
    }
    QList<GrammarTask> banksGrammar() const {
        switch (difficulty) {
            case Difficulty::Easy:   return Banks::grammarEasy();
            case Difficulty::Medium: return Banks::grammarMedium();
            case Difficulty::Hard:   return Banks::grammarHard();
        }
        return {};
    }

    void playCorrect() {
#if LL_HAS_MULTIMEDIA
        if (correctPlayer && !correctPlayer->source().isEmpty()) {
            correctPlayer->setPosition(0);
            correctPlayer->play();
        }
#endif
    }
    void playWrong() {
#if LL_HAS_MULTIMEDIA
        if (wrongPlayer && !wrongPlayer->source().isEmpty()) {
            wrongPlayer->setPosition(0);
            wrongPlayer->play();
        }
#endif
    }

    static std::mt19937 &rng() {
        static std::mt19937 g{std::random_device{}()};
        return g;
    }

    // ---- Состояние ----
    QStackedWidget *stack = nullptr;
    QWidget *welcomePage = nullptr;
    QWidget *translationPage = nullptr;
    QWidget *grammarPage = nullptr;

    QPushButton *translationBtn = nullptr;
    QPushButton *grammarBtn = nullptr;
    QLabel *difficultyLabel = nullptr;
    QLabel *scoreLabel = nullptr;
    QLabel *timerLabel = nullptr;
    QLabel *livesLabel = nullptr;
    QProgressBar *progressBar = nullptr;

    QLabel *trSource = nullptr;
    QTextEdit *trEdit = nullptr;
    QLabel *trFeedback = nullptr;
    QPushButton *trSubmit = nullptr;

    QLabel *grQuestion = nullptr;
    QVBoxLayout *grOptsLayout = nullptr;
    QButtonGroup *grGroup = nullptr;
    QList<QRadioButton*> grOptions;
    QLabel *grFeedback = nullptr;
    QPushButton *grSubmit = nullptr;

    QTimer *uiTimer = nullptr;
    int secondsLeft = 0;

#if LL_HAS_MULTIMEDIA
    QMediaPlayer *correctPlayer = nullptr;
    QMediaPlayer *wrongPlayer = nullptr;
#endif

    Difficulty difficulty = Difficulty::Easy;
    ExerciseKind currentKind = ExerciseKind::None;
    int taskIndex = 0;
    int wrongAttempts = 0;
    int maxWrong = 3;
    int score = 0;

    QList<TranslationTask> translationTasks;
    QList<GrammarTask> grammarTasks;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return app.exec();
}
