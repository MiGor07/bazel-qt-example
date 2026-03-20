#include <QtWidgets>
#include <vector>
#include <random>

enum class TicketStatus { Default, Review, Done };

class MainWindow : public QWidget {
public:
    MainWindow() {
        auto *mainLayout = new QVBoxLayout(this);

        
        spin = new QSpinBox;
        spin->setRange(1, 200);
        spin->setValue(10);
        mainLayout->addWidget(spin);

        
        auto *btnLayout = new QHBoxLayout;
        prevBtn = new QPushButton("Предыдущий");
        nextBtn = new QPushButton("Следующий (рандом)");
        btnLayout->addWidget(prevBtn);
        btnLayout->addWidget(nextBtn);
        btnLayout->addStretch();
        mainLayout->addLayout(btnLayout);

        
        auto *progLayout = new QHBoxLayout;
        totalBar = new QProgressBar;
        greenBar = new QProgressBar;
        totalBar->setFormat("Общий прогресс: %p%");
        greenBar->setFormat("Готово: %p%");
        progLayout->addWidget(totalBar);
        progLayout->addWidget(greenBar);
        mainLayout->addLayout(progLayout);

        
        auto *contentLayout = new QHBoxLayout;
        mainLayout->addLayout(contentLayout);

        table = new QTableWidget;
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        table->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        contentLayout->addWidget(table, 2);

        auto *group = new QGroupBox("Билет");
        auto *g = new QVBoxLayout(group);

        numberLabel = new QLabel("Номер: -");
        nameLabel   = new QLabel("Имя: -");

        nameEdit = new QLineEdit;
        nameEdit->setPlaceholderText("Новое имя");

        statusBox = new QComboBox;
        statusBox->addItems({"Не повторяли", "Повторить", "Готово"});

        g->addWidget(numberLabel);
        g->addWidget(nameLabel);
        g->addWidget(new QLabel("Изменить имя:"));
        g->addWidget(nameEdit);
        g->addWidget(new QLabel("Статус:"));
        g->addWidget(statusBox);
        g->addStretch();

        contentLayout->addWidget(group, 1);

        
        connect(spin, &QSpinBox::valueChanged, this, &MainWindow::rebuildTickets);
        connect(table, &QTableWidget::cellClicked, this, &MainWindow::onCellClicked);
        connect(table, &QTableWidget::cellDoubleClicked, this, &MainWindow::onCellDoubleClicked);
        connect(nameEdit, &QLineEdit::returnPressed, this, &MainWindow::applyNameEdit);
        connect(statusBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onStatusChanged);
        connect(nextBtn, &QPushButton::clicked, this, &MainWindow::goNextRandom);
        connect(prevBtn, &QPushButton::clicked, this, &MainWindow::goPrevious);

        rebuildTickets(spin->value());
    }

private:
    
    static int statusToIndex(TicketStatus s){
        if (s==TicketStatus::Default) return 0;
        if (s==TicketStatus::Review)  return 1;
        return 2;
    }
    static TicketStatus indexToStatus(int i){
        if (i==0) return TicketStatus::Default;
        if (i==1) return TicketStatus::Review;
        return TicketStatus::Done;
    }

     
    void rebuildTickets(int count){
        const int cols = 5;
        const int rows = (count + cols - 1) / cols;

        table->clear();
        table->setRowCount(rows);
        table->setColumnCount(cols);

        statuses.assign(count, TicketStatus::Default);
        names.resize(count);
        for(int i=0;i<count;++i) names[i]=QString("Билет %1").arg(i+1);

        currentIndex = -1;
        history.clear();

        totalBar->setRange(0, count);
        greenBar->setRange(0, count);
        updateProgress();

        for(int i=0;i<count;++i){
            int r=i/cols, c=i%cols;
            auto *item=new QTableWidgetItem(names[i]);
            item->setTextAlignment(Qt::AlignCenter);
            item->setData(Qt::UserRole,i);
            paintItem(item,statuses[i]);
            table->setItem(r,c,item);
        }
        resetQuestionView();
    }

    
    void paintItem(QTableWidgetItem* it, TicketStatus s){
    switch(s){
        case TicketStatus::Default: it->setBackground(Qt::lightGray); break;
        case TicketStatus::Review:  it->setBackground(Qt::yellow);    break;
        case TicketStatus::Done:    it->setBackground(Qt::green);     break;
    }
    it->setForeground(Qt::black);   
}

   
    void updateProgress(){
        int total=0, green=0;
        for(auto s:statuses){
            if(s!=TicketStatus::Default) total++;
            if(s==TicketStatus::Done) green++;
        }
        totalBar->setValue(total);
        greenBar->setValue(green);
    }

    
    void selectIndex(int idx, bool pushHistory=true){
        if(idx<0 || idx>= (int)statuses.size()) return;

        if(pushHistory && currentIndex!=-1)
            history.push_back(currentIndex);

        currentIndex = idx;

        int cols=table->columnCount();
        table->setCurrentCell(idx/cols, idx%cols);

        updateQuestionView();
    }

    void onCellClicked(int r,int c){
        auto *it=table->item(r,c);
        if(!it) return;
        selectIndex(it->data(Qt::UserRole).toInt());
    }

    
    void onCellDoubleClicked(int r,int c){
        auto *it=table->item(r,c);
        if(!it) return;
        int idx=it->data(Qt::UserRole).toInt();

        auto old=statuses[idx];
        auto &st=statuses[idx];

        if(st==TicketStatus::Done) st=TicketStatus::Review;
        else st=TicketStatus::Done;

        paintItem(it,st);
        if(idx==currentIndex)
            statusBox->setCurrentIndex(statusToIndex(st));

        if(old!=st) updateProgress();
    }

    
    void applyNameEdit(){
        if(currentIndex<0) return;
        if(!nameEdit->hasFocus()) return;
        auto t=nameEdit->text().trimmed();
        if(t.isEmpty()) return;

        names[currentIndex]=t;
        nameLabel->setText("Имя: "+t);
        updateTableItem(currentIndex);
    }

    
    void onStatusChanged(int i){
        if(currentIndex<0) return;
        auto old=statuses[currentIndex];
        statuses[currentIndex]=indexToStatus(i);
        updateTableItem(currentIndex);
        if(old!=statuses[currentIndex]) updateProgress();
    }

    void updateTableItem(int idx){
        int cols=table->columnCount();
        auto *it=table->item(idx/cols, idx%cols);
        if(!it) return;
        it->setText(names[idx]);
        paintItem(it,statuses[idx]);
    }

    
    void goNextRandom(){
        std::vector<int> pool;
        for(int i=0;i<(int)statuses.size();++i)
            if(statuses[i]!=TicketStatus::Done)
                pool.push_back(i);
        if(pool.empty()) return;

        std::uniform_int_distribution<> d(0,pool.size()-1);
        static std::mt19937 gen{std::random_device{}()};
        int idx=pool[d(gen)];
        selectIndex(idx,true);
    }

    void goPrevious(){
        if(history.empty()) return;
        int idx=history.back();
        history.pop_back();
        selectIndex(idx,false);
    }

    
    void updateQuestionView(){
        numberLabel->setText(QString("Номер: %1").arg(currentIndex+1));
        nameLabel->setText("Имя: "+names[currentIndex]);
        nameEdit->clear();
        statusBox->setCurrentIndex(statusToIndex(statuses[currentIndex]));
    }

    void resetQuestionView(){
        numberLabel->setText("Номер: -");
        nameLabel->setText("Имя: -");
        nameEdit->clear();
        statusBox->setCurrentIndex(0);
    }

    QSpinBox *spin;
    QTableWidget *table;
    QPushButton *nextBtn,*prevBtn;
    QProgressBar *totalBar,*greenBar;
    QLabel *numberLabel,*nameLabel;
    QLineEdit *nameEdit;
    QComboBox *statusBox;

    std::vector<TicketStatus> statuses;
    std::vector<QString> names;
    int currentIndex=-1;
    std::vector<int> history;
};

int main(int argc,char*argv[]){
    QApplication app(argc,argv);
    MainWindow w;
    w.resize(1100,650);
    w.show();
    return app.exec();
}