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
        auto *leftBox = new QGroupBox("Меню");
        auto *leftLay = new QVBoxLayout(leftBox);

        translationBtn = new QPushButton("📖 Translation");
        grammarBtn     = new QPushButton("📝 Grammar");
        translationBtn->setMinimumHeight(42);
        grammarBtn->setMinimumHeight(42);

        leftLay->addWidget(translationBtn);
        leftLay->addWidget(grammarBtn);
        leftLay->addSpacing(8);

        auto *settingsLabel = new QLabel();
        settingsLabel->setWordWrap(true);
        difficultyLabel = settingsLabel;
        leftLay->addWidget(settingsLabel);

        leftLay->addStretch();

        auto *scoreBox = new QGroupBox("Ваш прогресс");
        auto *scoreLay = new QVBoxLayout(scoreBox);
        scoreLabel = new QLabel("Баллы: 0");
        QFont f = scoreLabel->font(); f.setBold(true); f.setPointSize(f.pointSize() + 2);
        scoreLabel->setFont(f);
        scoreLay->addWidget(scoreLabel);
        leftLay->addWidget(scoreBox);

        root->addWidget(leftBox, 0);
        leftBox->setMinimumWidth(240);

        // --- Правая часть: верхняя инфо-строка + stacked ---
        auto *rightWrap = new QVBoxLayout;
        root->addLayout(rightWrap, 1);

        auto *topLay = new QHBoxLayout;
        progressBar = new QProgressBar;
        progressBar->setFormat("Задание %v / %m");
        progressBar->setRange(0, 1);
        progressBar->setValue(0);
        timerLabel = new QLabel("⏱ --:--");
        timerLabel->setAlignment(Qt::AlignCenter);
        timerLabel->setMinimumWidth(80);
        QFont tf = timerLabel->font(); tf.setBold(true);
        timerLabel->setFont(tf);
        livesLabel = new QLabel("❤❤❤");
        livesLabel->setAlignment(Qt::AlignCenter);
        livesLabel->setMinimumWidth(70);

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
        auto *lay = new QVBoxLayout(w);
        lay->addStretch();
        auto *title = new QLabel("Добро пожаловать в LinguaDuo!");
        QFont tf = title->font(); tf.setPointSize(tf.pointSize() + 6); tf.setBold(true);
        title->setFont(tf);
        title->setAlignment(Qt::AlignCenter);
        lay->addWidget(title);

        auto *desc = new QLabel(
            "Выберите упражнение слева.\n\n"
            "• Translation — перевод фраз с английского\n"
            "• Grammar — выбор правильной формы\n\n"
            "Подсказка: клавиша H.\n"
            "Уровень сложности — в меню «Настройки».");
        desc->setAlignment(Qt::AlignCenter);
        lay->addWidget(desc);
        lay->addStretch();
        return w;
    }

    QWidget *buildTranslationPage() {
        auto *w = new QWidget;
        auto *lay = new QVBoxLayout(w);

        auto *hdr = new QLabel("Переведите на русский:");
        lay->addWidget(hdr);

        trSource = new QLabel;
        QFont f = trSource->font(); f.setPointSize(f.pointSize() + 4); f.setBold(true);
        trSource->setFont(f);
        trSource->setWordWrap(true);
        trSource->setStyleSheet("padding: 12px; background: #2a2a2a; border-radius: 6px;");
        lay->addWidget(trSource);

        trEdit = new QTextEdit;
        trEdit->setPlaceholderText("Введите перевод здесь…");
        lay->addWidget(trEdit, 1);

        trFeedback = new QLabel(" ");
        trFeedback->setWordWrap(true);
        lay->addWidget(trFeedback);

        auto *btnRow = new QHBoxLayout;
        btnRow->addStretch();
        trSubmit = new QPushButton("Submit");
        trSubmit->setMinimumHeight(36);
        btnRow->addWidget(trSubmit);
        lay->addLayout(btnRow);

        connect(trSubmit, &QPushButton::clicked, this, &MainWindow::submitTranslation);
        return w;
    }

    QWidget *buildGrammarPage() {
        auto *w = new QWidget;
        auto *lay = new QVBoxLayout(w);

        auto *hdr = new QLabel("Выберите правильный вариант:");
        lay->addWidget(hdr);

        grQuestion = new QLabel;
        QFont f = grQuestion->font(); f.setPointSize(f.pointSize() + 4); f.setBold(true);
        grQuestion->setFont(f);
        grQuestion->setWordWrap(true);
        grQuestion->setStyleSheet("padding: 12px; background: #2a2a2a; border-radius: 6px;");
        lay->addWidget(grQuestion);

        grGroup = new QButtonGroup(this);
        auto *optsBox = new QGroupBox("Варианты");
        grOptsLayout = new QVBoxLayout(optsBox);
        for (int i = 0; i < 4; ++i) {
            auto *rb = new QRadioButton;
            grGroup->addButton(rb, i);
            grOptsLayout->addWidget(rb);
            grOptions.push_back(rb);
        }
        lay->addWidget(optsBox);

        grFeedback = new QLabel(" ");
        grFeedback->setWordWrap(true);
        lay->addWidget(grFeedback);

        auto *btnRow = new QHBoxLayout;
        btnRow->addStretch();
        grSubmit = new QPushButton("Submit");
        grSubmit->setMinimumHeight(36);
        btnRow->addWidget(grSubmit);
        lay->addLayout(btnRow);

        connect(grSubmit, &QPushButton::clicked, this, &MainWindow::submitGrammar);
        return w;
    }

    void buildStatus() {
        statusBar()->showMessage("Готов к работе.");
    }

    void applyDarkTheme() {
        qApp->setStyle("Fusion");
        QPalette p;
        p.setColor(QPalette::Window, QColor(30, 30, 32));
        p.setColor(QPalette::WindowText, Qt::white);
        p.setColor(QPalette::Base, QColor(22, 22, 24));
        p.setColor(QPalette::AlternateBase, QColor(45, 45, 48));
        p.setColor(QPalette::ToolTipBase, Qt::white);
        p.setColor(QPalette::ToolTipText, Qt::black);
        p.setColor(QPalette::Text, Qt::white);
        p.setColor(QPalette::Button, QColor(45, 45, 48));
        p.setColor(QPalette::ButtonText, Qt::white);
        p.setColor(QPalette::Highlight, QColor(0, 120, 215));
        p.setColor(QPalette::HighlightedText, Qt::white);
        qApp->setPalette(p);

        qApp->setStyleSheet(
            "QPushButton { padding: 6px 14px; border-radius: 6px; background: #3a3a3f; }"
            "QPushButton:hover { background: #4a4a50; }"
            "QPushButton:disabled { color: #888; background: #2a2a2e; }"
            "QProgressBar { border: 1px solid #555; border-radius: 6px; text-align: center; background: #222; }"
            "QProgressBar::chunk { background-color: #2e8b57; border-radius: 5px; }"
            "QGroupBox { border: 1px solid #444; border-radius: 6px; margin-top: 14px; padding-top: 6px; }"
            "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 4px; }"
            "QRadioButton { padding: 4px; }"
            "QTextEdit { border: 1px solid #444; border-radius: 6px; padding: 6px; }"
        );
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
            trFeedback->setStyleSheet("color: #ffb347;");
            return;
        }

        auto r = StringMatcher::compare(user, t.acceptedAnswers);
        if (r.exact) {
            playCorrect();
            trFeedback->setText("✅ Верно!");
            trFeedback->setStyleSheet("color: #7cff7c;");
            advanceTask();
        } else if (r.close) {
            playCorrect();
            trFeedback->setText(QString("✅ Принято (мелкие расхождения, расстояние %1). "
                                        "Эталон: «%2»")
                                    .arg(r.distance).arg(t.acceptedAnswers.first()));
            trFeedback->setStyleSheet("color: #c6ff9e;");
            advanceTask();
        } else {
            registerWrong();
            trFeedback->setText(QString("❌ Неверно. Попробуйте ещё раз. (жизней осталось: %1)")
                                    .arg(maxWrong - wrongAttempts));
            trFeedback->setStyleSheet("color: #ff7c7c;");
            playWrong();
        }
    }

    void submitGrammar() {
        if (currentKind != ExerciseKind::Grammar) return;
        int id = grGroup->checkedId();
        if (id < 0) {
            grFeedback->setText("Выберите вариант перед отправкой.");
            grFeedback->setStyleSheet("color: #ffb347;");
            return;
        }
        const auto &t = grammarTasks[taskIndex];
        if (id == t.correctIndex) {
            playCorrect();
            grFeedback->setText("✅ Верно!");
            grFeedback->setStyleSheet("color: #7cff7c;");
            advanceTask();
        } else {
            registerWrong();
            grFeedback->setText(QString("❌ Неверно. Правильно: «%1». (жизней осталось: %2)")
                                    .arg(t.options[t.correctIndex])
                                    .arg(maxWrong - wrongAttempts));
            grFeedback->setStyleSheet("color: #ff7c7c;");
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
        QString hearts;
        for (int i = 0; i < left; ++i) hearts += "❤";
        for (int i = left; i < maxWrong; ++i) hearts += "♡";
        livesLabel->setText(hearts);
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
        QString d;
        switch (difficulty) {
            case Difficulty::Easy:   d = "Easy";   break;
            case Difficulty::Medium: d = "Medium"; break;
            case Difficulty::Hard:   d = "Hard";   break;
        }
        difficultyLabel->setText(QString(
            "Уровень: <b>%1</b><br>"
            "Время: %2 сек<br>"
            "Жизни: 3<br>"
            "Баллы за задание: %3")
            .arg(d).arg(totalTimeForCurrent()).arg(pointsForDifficulty()));
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
