#include "mainwindow.h"
#include <QLabel>
#include <QGridLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QTimer>
MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("2048");

    // The whole game UI lives inside a fixed-size "content" widget, which is
    // then centered inside the top-level window, instead of calling
    // setFixedSize() directly on the top-level window.
    //
    // Reason: on Qt for WebAssembly, the top-level window is sometimes
    // resized to the full browser viewport before the real container size
    // has been reported back to Qt, which used to stretch every layout
    // (rows/columns/buttons) apart. Keeping the actual board in its own
    // fixed 420x650 child widget makes the layout look correct no matter
    // what size the browser assigns to the top-level widget - any extra
    // space just appears as empty margin around the centered board instead
    // of stretching the game itself. This has no effect on the desktop
    // build, where the window still ends up exactly 420x650.
    auto *content = new QWidget(this);
    content->setFixedSize(420, 650);
    content->setStyleSheet("background-color: #faf8ef;");

    setStyleSheet("background-color: #3c3a32;");

    auto *outerLayout = new QGridLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->addWidget(content, 0, 0, Qt::AlignCenter);

    auto *mainLayout = new QVBoxLayout(content);
    auto *headerLayout = new QHBoxLayout();
    auto *titleLayout = new QVBoxLayout();

    titleLabel = new QLabel("2048", content);
    titleLabel->setStyleSheet("color: #776e65; font-size: 48px; font-weight: bold; background: transparent;");

    subtitleLabel = new QLabel("Join the tiles, get to 2048!", content);
    subtitleLabel->setStyleSheet("color: #776e65; font-size: 14px; background: transparent;");

    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(subtitleLabel);

    headerLayout->addLayout(titleLayout);
    headerLayout->addStretch();

    auto *scoreLayout = new QHBoxLayout();

    scoreLabel = new QLabel("Score\n0", content);
    scoreLabel->setAlignment(Qt::AlignCenter);
    setCellStyle(scoreLabel, 187, 173, 160, "white", 12, "bold", 6);
    scoreLabel->setFixedSize(70, 45);

    bestScoreLabel = new QLabel("Best\n0", content);
    bestScoreLabel->setAlignment(Qt::AlignCenter);
    setCellStyle(bestScoreLabel, 187, 173, 160, "white", 12, "bold", 6);
    bestScoreLabel->setFixedSize(70, 45);

    currentModeLabel = new QLabel("Current Mode\n0", content);
    currentModeLabel->setAlignment(Qt::AlignCenter);
    setCellStyle(currentModeLabel, 187, 173, 160, "white", 12, "bold", 6);
    currentModeLabel->setFixedSize(70, 45);

    scoreLayout->addWidget(scoreLabel);
    scoreLayout->addWidget(bestScoreLabel);
    scoreLayout->addWidget(currentModeLabel);
    headerLayout->addLayout(scoreLayout);
    

    mainLayout->addLayout(headerLayout);
    auto *buttonLayout = new QHBoxLayout();
    QPushButton *undoButton = new QPushButton("Undo", content);
    QPushButton *restartButton = new QPushButton("Restart", content);
    connect(undoButton, &QPushButton::clicked, this, &MainWindow::undoMove);
    connect(restartButton, &QPushButton::clicked, this, &MainWindow::restartGame);

    buttonLayout->addWidget(undoButton);
    buttonLayout->addWidget(restartButton); 
    undoButton->setFocusPolicy(Qt::NoFocus); // restricting arrow keys 
    restartButton->setFocusPolicy(Qt::NoFocus);
    QPushButton *modeButton = new QPushButton("Mode", content);
    modeButton->setFocusPolicy(Qt::NoFocus);
    buttonLayout->addWidget(modeButton);
    connect(modeButton, &QPushButton::clicked, this, [this]() {
        bool wasGameOver = gameOverFlag;  // save previous state
        gameOverFlag = true;              // block input while dialog open
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Select Mode");
        msgBox.setText("Choose a game mode:");
        QPushButton *normalButton = msgBox.addButton("Normal", QMessageBox::AcceptRole); // to put first in selection order
        QPushButton *hardButton = msgBox.addButton("Hard", QMessageBox::ActionRole);
        QPushButton *unlimitedButton = msgBox.addButton("Unlimited", QMessageBox::ActionRole);
        QPushButton *unlimitedHardButton = msgBox.addButton("Unlimited Hard", QMessageBox::ActionRole);
        msgBox.exec();
        if (msgBox.clickedButton() == normalButton) changeToNormalMode();
        else if (msgBox.clickedButton() == hardButton) changeToHardMode();
        else if (msgBox.clickedButton() == unlimitedButton) { changeToUnlimitedMode(); restartGame(); }
        else if (msgBox.clickedButton() == unlimitedHardButton) { changeToUnlimitedHardMode(); restartGame(); }
        else gameOverFlag = wasGameOver;  // if dialog closed without selection, restore state
    });
    mainLayout->addLayout(buttonLayout); 

    auto *grid = new QGridLayout();
    grid->setSpacing(10);
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < M; j++)
        {
            QLabel *cell = new QLabel();
            cells[i][j] = cell;
            cell->setFixedSize(80, 80);
            cell->setAlignment(Qt::AlignCenter);
            setCellStyle(cell, 95, 88, 81, "white", 24, "bold", 6);
            grid->addWidget(cell, i, j);
        }
    }

    mainLayout->addLayout(grid); // push grid layout

    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < M; j++)
        {
            board[i][j] = 0;
        }
    }
    srand(static_cast<unsigned>(time(NULL))); // random seed to creates random numbers

    int placedTiles = 0;
    while (placedTiles < 2) // spawn random 2 tiles in grid with 2
    {
        int r = rand() % N;
        int c = rand() % M;

        if (board[r][c] == 0)
        {
            board[r][c] = 2;
            placedTiles++; 
        }
    }

    updateBoard();
    hardTimer = new QTimer(this); // creates hard mode timer

    connect(hardTimer, &QTimer::timeout,this,&MainWindow::hardDifficulty);
}
void MainWindow::updateCell(int row, int col, int value) // assigning colors
{
    cells[row][col]->setText(QString::number(value));
    if (value == 2)
        setCellStyle(cells[row][col], 255, 245, 220, "#776e65", 24, "bold", 6);
    else if (value == 4)
        setCellStyle(cells[row][col], 255, 238, 210, "#776e65", 24, "bold", 6);
    else if (value == 8)
        setCellStyle(cells[row][col], 233, 180, 130, "white", 24, "bold", 6);
    else if (value == 16)
        setCellStyle(cells[row][col], 227, 153, 108, "white", 24, "bold", 6);
    else if (value == 32)
        setCellStyle(cells[row][col], 230, 130, 103, "white", 24, "bold", 6);
    else if (value == 64)
        setCellStyle(cells[row][col], 218, 101, 68, "white", 24, "bold", 6);
    else if (value == 128)
        setCellStyle(cells[row][col], 235, 214, 140, "white", 24, "bold", 6);
    else if (value == 256)
        setCellStyle(cells[row][col], 230, 205, 112, "white", 24, "bold", 6);
    else if (value == 512)
        setCellStyle(cells[row][col], 235, 212, 100, "white", 24, "bold", 6);
    else if (value == 1024)
        setCellStyle(cells[row][col], 233, 204, 75, "white", 24, "bold", 6);
    else if (value == 2048)
        setCellStyle(cells[row][col], 230, 190, 40, "white", 24, "bold", 6);
    else if (value > 2048)
        setCellStyle(cells[row][col], 72, 60, 37, "white", 24, "bold", 6);
};
void MainWindow::updateBoard() // update board function
{
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < M; j++)
        {
            int val = board[i][j];
            if (val == 0)
            {
                cells[i][j]->setText("");
                setCellStyle(cells[i][j], 95, 88, 81, "white", 24, "bold", 6);
            }
            else
            {
                updateCell(i, j, val);
            }
        }
    }

    if (score > bestScore) // updates best score
    {
        bestScore = score;
    }

    scoreLabel->setText(QString("Score\n%1").arg(score)); // updates label
    bestScoreLabel->setText(QString("Best\n%1").arg(bestScore)); // updates label
    if (!hardMode && !unlimitedMode) // this section updates Mode label
    {
        currentModeLabel->setText("Mode\nNormal");
    }
    else if (hardMode && !unlimitedMode)
    {
        currentModeLabel->setText("Mode\nHard");
    }
    else if (!hardMode && unlimitedMode)
    {
        currentModeLabel->setText("Mode\nUnlimited");
    }
    else if (hardMode && unlimitedMode)
    {
        currentModeLabel->setText("Mode\nU-Hard");
    }
};
void MainWindow::keyPressEvent(QKeyEvent *event) // input
{
    if (gameOverFlag == true) // is game continue?
    {
        return;
    }

    if (event->key() == Qt::Key_U)
    {
        undoMove();
        return;
    }

    GameState currentState;
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < M; j++)
        {
            currentState.board[i][j] = board[i][j];
        }
    }

    currentState.score = score;
    bool moved = false; // chose for, hard difficulty if it is movable returns true, and also for if (moved) condition
    if (event->key() == Qt::Key_Left || event->key() == Qt::Key_A)
    {
        moved = moveLeft();
    }
    else if (event->key() == Qt::Key_Right || event->key() == Qt::Key_D)
    {
        moved = moveRight();
    }
    else if (event->key() == Qt::Key_Up || event->key() == Qt::Key_W)
    {
        moved = moveUp();
    }
    else if (event->key() == Qt::Key_Down || event->key() == Qt::Key_S)
    {
        moved = moveDown();
    }
    // else if (event->key() == Qt::Key_T) //-- Completely Testing
    // {
        // winning condition test
    //     board[0][0] = 1024;
    //     board[0][1] = 1024;
    //     updateBoard();
        // winning condition test
        // for color test
        // updateCell(0, 0, 2);
        // updateCell(0, 1, 4);
        // updateCell(0, 2, 8);
        // updateCell(0, 3, 16);
        // updateCell(1, 0, 32);
        // updateCell(1, 1, 64);
        // updateCell(1, 2, 128);
        // updateCell(1, 3, 256);
        // updateCell(2, 0, 512);
        // updateCell(2, 1, 1024);
        // updateCell(2, 2, 2048);
        // updateCell(2, 3, 4096);
        // for color test
        // move test
        // updateCell(0, 0, 2);
        // updateCell(0, 1, 2);
        // updateCell(0, 2, 4);
        // updateCell(0, 3, 8);
        // updateCell(1, 0, 2);
        // updateCell(1, 1, 2);
        // move test
    // }
    else if (event->key() == Qt::Key_R) {
        restartGame();
    }
    if (moved)
    {
        undoStack.push(currentState);
        if (moved && hardMode) {  hardTimer->start(5000); }
        spawnRandomTile();
        updateBoard();
        winningCondition();
        loseCondition();
    }
};
bool MainWindow::slideArray(int arr[], int size)
{
    bool changed = false;
    int insertPos = 0;

    for (int i = 0; i < size; i++)
    {
        if (arr[i] != 0)
        {
            if (i != insertPos)
            {
                arr[insertPos] = arr[i];
                arr[i] = 0;
                changed = true;
            }
            insertPos++;
        }
    }

    for (int i = 0; i < size - 1; i++)
    {
        if (arr[i] != 0 && arr[i] == arr[i + 1])
        {
            arr[i] *= 2;
            arr[i + 1] = 0;
            score += arr[i];
            changed = true;
        }
    }

    insertPos = 0;
    for (int i = 0; i < size; i++)
    {
        if (arr[i] != 0)
        {
            if (i != insertPos)
            {
                arr[insertPos] = arr[i];
                arr[i] = 0;
            }
            insertPos++;
        }
    }

    return changed;
};
bool MainWindow::moveLeft()
{
    bool boardChanged = false;
    for (int i = 0; i < N; i++)
    {
        int row[M];
        for (int j = 0; j < M; j++)
            row[j] = board[i][j];

        if (slideArray(row, M))
            boardChanged = true;

        for (int j = 0; j < M; j++)
            board[i][j] = row[j];
    }

    return boardChanged;
};
bool MainWindow::moveRight()
{
    bool boardChanged = false;
    for (int i = 0; i < N; i++)
    {
        int row[M];
        for (int j = 0; j < M; j++)
            row[j] = board[i][M - 1 - j];

        if (slideArray(row, M))
            boardChanged = true;

        for (int j = 0; j < M; j++)
            board[i][M - 1 - j] = row[j];
    }
    return boardChanged;
};
bool MainWindow::moveUp()
{
    bool boardChanged = false;
    for (int i = 0; i < M; i++)
    {
        int col[N];
        for (int j = 0; j < N; j++)
            col[j] = board[j][i];

        if (slideArray(col, N))
            boardChanged = true;

        for (int j = 0; j < N; j++)
            board[j][i] = col[j];
    }
    return boardChanged;
};
bool MainWindow::moveDown()
{
    bool boardChanged = false;

    for (int j = 0; j < M; j++)
    {
        int col[N];
        for (int i = 0; i < N; i++)
            col[i] = board[N - 1 - i][j];

        if (slideArray(col, N))
            boardChanged = true;

        for (int i = 0; i < N; i++)
            board[N - 1 - i][j] = col[i];
    }
    return boardChanged;
};
void MainWindow::spawnRandomTile()
{
    QList<QPair<int, int>> emptyCells;

    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < M; j++)
        {
            if (board[i][j] == 0)
            {
                emptyCells.append(qMakePair(i, j));
            }
        }
    }

    if (emptyCells.isEmpty())
        return;

    int randIndex = rand() % emptyCells.size();
    int r = emptyCells[randIndex].first;
    int c = emptyCells[randIndex].second;

    board[r][c] = (rand() % 100 < P) ? 2 : 4;
}
void MainWindow::winningCondition() // if it is not unlimited mode, checks for all tiles if there is 2048, returns a messagebox with mode selections, checks for current mode if it is same with the mode it does not restart the game, if not it restarts.
{
    if (!unlimitedMode)
    {
        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < M; j++)
            {
                if (board[i][j] == K)
                {
                    QMessageBox msgBox(this);
                    msgBox.setWindowTitle("You Win!");
                    msgBox.setText(QString("You Reached %1!").arg(K));
                    QPushButton *restartButton = msgBox.addButton("Restart", QMessageBox::AcceptRole);
                    QPushButton *normalButton = msgBox.addButton("Normal Mode", QMessageBox::ActionRole);
                    QPushButton *hardButton = msgBox.addButton("Hard Mode", QMessageBox::ActionRole);
                    QPushButton *unlimitedButton = msgBox.addButton("Unlimited Mode", QMessageBox::ActionRole);
                    QPushButton *unlimitedHardButton = msgBox.addButton("Unlimited Hard Mode", QMessageBox::ActionRole);
                    gameOverFlag = true;
                    msgBox.exec();
                    if (msgBox.clickedButton() == restartButton)
                    {
                        restartGame();
                    }
                    else if (msgBox.clickedButton() == normalButton)
                    {
                        changeToNormalMode();
                    }
                    else if (msgBox.clickedButton() == hardButton)
                    {
                        changeToHardMode();
                    }
                        else if (msgBox.clickedButton() == unlimitedButton)
                    {
                        if (hardMode) {
                            restartGame();
                        }
                        changeToUnlimitedMode();
                    }
                        else if (msgBox.clickedButton() == unlimitedHardButton)
                    {
                        if (!hardMode) {
                            restartGame();
                        }
                        changeToUnlimitedHardMode();
                    }
                    return;
                }
            }
        }
    }
}
void MainWindow::loseCondition() // lose checker
{
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < M; j++)
        {
            if (board[i][j] == 0)
            {
                return;
            }
        }
    }

    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < M; j++)
        {
            if (j < M - 1 && board[i][j] == board[i][j + 1])
            {
                return;
            }
            if (i < N - 1 && board[i][j] == board[i + 1][j])
            {
                return;
            }
        }
    }

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Game Over!");
    msgBox.setText("No more valid moves!");
    QPushButton *restartButton = msgBox.addButton("Restart", QMessageBox::AcceptRole);
    QPushButton *normalButton = msgBox.addButton("Normal Mode", QMessageBox::ActionRole);
    QPushButton *hardButton = msgBox.addButton("Hard Mode", QMessageBox::ActionRole);
    QPushButton *unlimitedButton = msgBox.addButton("Unlimited Mode", QMessageBox::ActionRole);
    QPushButton *unlimitedHardButton = msgBox.addButton("Unlimited Hard Mode", QMessageBox::ActionRole);
    gameOverFlag = true;
    msgBox.exec();

    if (msgBox.clickedButton() == restartButton)
    {
        restartGame();
    }
    else if (msgBox.clickedButton() == normalButton)
    {
        changeToNormalMode();
    }
    else if (msgBox.clickedButton() == hardButton)
    {
        changeToHardMode();
    }
    else if (msgBox.clickedButton() == unlimitedButton)
    {
        changeToUnlimitedMode();
        restartGame(); 
    }
    else if (msgBox.clickedButton() == unlimitedHardButton)
    {
        changeToUnlimitedHardMode();
        restartGame();
    }
}
void MainWindow::restartGame()
{
    undoStack.clear();
    gameOverFlag = false; // game continues
    score = 0;
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < M; j++)
        {
            board[i][j] = 0;
        }
    }
    spawnRandomTile();
    spawnRandomTile();
    if (hardMode) hardTimer->start(5000);
    updateBoard();
}
void MainWindow::changeToHardMode()
{
    gameOverFlag = false;
    hardMode = true;
    unlimitedMode = false;
    restartGame();
}
void MainWindow::changeToNormalMode()
{
    gameOverFlag = false;
    hardMode = false;
    unlimitedMode = false;
    hardTimer->stop();
    restartGame();
}
void MainWindow::changeToUnlimitedMode() // the reason why the gamestate do not reset is in winning condition the game must continue it makes much more sense
{
    gameOverFlag = false;
    hardMode = false;
    hardTimer->stop();
    unlimitedMode = true;
}
void MainWindow::changeToUnlimitedHardMode() // the reason why the gamestate do not reset is in winning condition the game must continue it makes much more sense
{
    gameOverFlag = false;
    hardMode = true;
    hardTimer->start(5000);
    unlimitedMode = true;
}
void MainWindow::undoMove()
{
    if (undoStack.isEmpty())
    {
        return;
    }

    GameState prevState = undoStack.pop();

    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < M; j++)
        {
            this->board[i][j] = prevState.board[i][j];
        }
    }

    this->score = prevState.score;
    updateBoard();
    if (hardMode && !gameOverFlag) hardTimer->start(5000);
}
void MainWindow::hardDifficulty() {
    if (hardMode && !gameOverFlag) {
        GameState currentState;
        for (int i = 0; i < N; i++)
            for (int j = 0; j < M; j++)
                currentState.board[i][j] = board[i][j];
        currentState.score = score;

        QList<int> validMoves;
        int backup[N][M];
        for (int i = 0; i < N; i++){
            for (int j = 0; j < M; j++)
                backup[i][j] = board[i][j];
        }
        int backupScore = score;
        if (moveLeft())
            validMoves.append(0);
        memcpy(board, backup, sizeof(board));
        score = backupScore;
        if (moveRight())
            validMoves.append(1);
        memcpy(board, backup, sizeof(board));
        score = backupScore;
        if (moveUp())
            validMoves.append(2);
        memcpy(board, backup, sizeof(board));
        score = backupScore;
        if (moveDown())
            validMoves.append(3);
        memcpy(board, backup, sizeof(board));
        score = backupScore;
        if (validMoves.isEmpty())
            return;

        int move = validMoves[rand() % validMoves.size()];
        switch (move)
        {
        case 0: moveLeft(); break;
        case 1: moveRight(); break;
        case 2: moveUp(); break;
        case 3: moveDown(); break;
        }
    undoStack.push(currentState);
    spawnRandomTile();
    updateBoard();
    winningCondition();
    loseCondition();
    }
}
