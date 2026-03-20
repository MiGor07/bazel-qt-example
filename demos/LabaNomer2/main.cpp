#include <QtWidgets>
#include <QtMultimedia>

class PlayerWindow : public QWidget {
    

public:
    PlayerWindow() {
        player = new QMediaPlayer(this);
        audio  = new QAudioOutput(this);
        player->setAudioOutput(audio);
        audio->setVolume(0.5);

        setupUi();
        setupConnections();
        applyDarkTheme();
    }

private:
    QMediaPlayer *player;
    QAudioOutput *audio;

    QPushButton *playBtn;
    QPushButton *fileBtn;
    QLabel      *statusLabel;
    QSlider     *volumeSlider;
    QDial       *volumeDial;
    QComboBox   *presetBox;

    
    void setupUi() {
        setWindowTitle("Sound Player");
        resize(420, 260);

        auto *mainLayout = new QVBoxLayout(this);

        
        fileBtn = new QPushButton("Выбрать файл");
        mainLayout->addWidget(fileBtn);

        
        statusLabel = new QLabel("Файл не выбран");
        mainLayout->addWidget(statusLabel);

        
        playBtn = new QPushButton("▶ Play");
        playBtn->setEnabled(false);
        mainLayout->addWidget(playBtn);

        
        auto *volLabel = new QLabel("Громкость");
        mainLayout->addWidget(volLabel);

        auto *volLayout = new QHBoxLayout;
        volumeSlider = new QSlider(Qt::Horizontal);
        volumeSlider->setRange(0, 100);
        volumeSlider->setValue(50);

        volumeDial = new QDial;
        volumeDial->setRange(0, 100);
        volumeDial->setValue(50);
        volumeDial->setNotchesVisible(true);
        volumeDial->setFixedSize(70,70);

        volLayout->addWidget(volumeSlider);
        volLayout->addWidget(volumeDial);
        mainLayout->addLayout(volLayout);

       
        auto *presetLabel = new QLabel("Пресет громкости");
        mainLayout->addWidget(presetLabel);

        presetBox = new QComboBox;
        presetBox->addItems({
            "Тихо (20%)",
            "Средне (50%)",
            "Громко (80%)",
            "Максимум (100%)"
        });
        mainLayout->addWidget(presetBox);

        mainLayout->addStretch();
    }

   
    void setupConnections() {
        connect(fileBtn, &QPushButton::clicked,
                this, &PlayerWindow::chooseFile);

        connect(playBtn, &QPushButton::clicked,
                this, &PlayerWindow::togglePlayback);

        connect(volumeSlider, &QSlider::valueChanged,
                this, &PlayerWindow::volumeChanged);

        connect(volumeDial, &QDial::valueChanged,
                this, &PlayerWindow::volumeChanged);

        connect(presetBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &PlayerWindow::presetChanged);

        connect(player, &QMediaPlayer::playbackStateChanged,
                this, &PlayerWindow::updateStatus);
    }

    
    void applyDarkTheme() {
        qApp->setStyle("Fusion");

        QPalette p;
        p.setColor(QPalette::Window, QColor(30,30,30));
        p.setColor(QPalette::WindowText, Qt::white);
        p.setColor(QPalette::Base, QColor(22,22,22));
        p.setColor(QPalette::AlternateBase, QColor(45,45,45));
        p.setColor(QPalette::ToolTipBase, Qt::white);
        p.setColor(QPalette::ToolTipText, Qt::white);
        p.setColor(QPalette::Text, Qt::white);
        p.setColor(QPalette::Button, QColor(45,45,45));
        p.setColor(QPalette::ButtonText, Qt::white);
        p.setColor(QPalette::Highlight, QColor(0,120,215));
        p.setColor(QPalette::HighlightedText, Qt::black);

        qApp->setPalette(p);
    }
    
    


private slots:

    void loadDefaultFile() {
    QString path = QCoreApplication::applicationDirPath() + "/01.mp3";

    if (QFile::exists(path)) {
        player->setSource(QUrl::fromLocalFile(path));
        statusLabel->setText("Загружен файл: 01.mp3");
        playBtn->setEnabled(true);
    } else {
        statusLabel->setText("Файл 01.mp3 не найден");
    }
}
    
    void chooseFile() {
        QString file = QFileDialog::getOpenFileName(
            this,
            "Выберите аудиофайл",
            "",
            "Audio (*.mp3 *.wav *.ogg)"
        );

        if(file.isEmpty()) return;

        player->setSource(QUrl::fromLocalFile(file));
        statusLabel->setText("Файл загружен");
        playBtn->setEnabled(true);
    }

    
    void togglePlayback() {
        if(player->playbackState() == QMediaPlayer::PlayingState)
            player->pause();
        else
            player->play();
    }

   
    void volumeChanged(int v) {
        audio->setVolume(v / 100.0);
        volumeSlider->blockSignals(true);
        volumeDial->blockSignals(true);
        volumeSlider->setValue(v);
        volumeDial->setValue(v);
        volumeSlider->blockSignals(false);
        volumeDial->blockSignals(false);
    }

   
    void presetChanged(int index) {
        static int values[] = {20, 50, 80, 100};
        int v = values[index];
        volumeChanged(v);
    }

   
    void updateStatus() {
        if(player->playbackState() == QMediaPlayer::PlayingState)
            playBtn->setText("⏸ Pause");
        else
            playBtn->setText("▶ Play");
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    PlayerWindow w;
    w.show();
    return app.exec();
}

