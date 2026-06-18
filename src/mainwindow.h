#pragma once
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QStack>

const int M = 4;
const int N = 4;
const int K = 2048;
const int P = 90;
const int Q = 10;
struct GameState
{
    int board[N][M];
    int score;
};


class MainWindow : public QWidget
{
public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    QLabel *cells[N][M];
    QLabel *scoreLabel;
    QLabel *bestScoreLabel;
    QLabel *currentModeLabel;
    QLabel *titleLabel;
    QLabel *subtitleLabel;

    QPushButton *normalBtn;
    QPushButton *unlimitedBtn;
    QPushButton *hardBtn;
    QPushButton *undoBtn;
    QPushButton *restartBtn;
    QTimer *hardTimer;

    int bestScore = 0;
    int board[N][M];

    void updateCell(int row, int col, int value);
    void updateBoard();
    void spawnRandomTile();
    void winningCondition();
    void loseCondition();
    bool moveLeft();
    bool moveRight();
    bool moveUp();
    bool moveDown();
    bool gameOverFlag = false;
    void restartGame();
    void changeToNormalMode();
    void changeToHardMode();
    void changeToUnlimitedMode();
    void changeToUnlimitedHardMode();
    QStack<GameState> undoStack;
    void hardDifficulty();
    void undoMove();
    bool slideArray(int arr[], int size);
    bool hardMode = false;
    bool unlimitedMode = false;

    int score = 0;

    void setCellStyle(QLabel *cell, int r, int g, int b, QString textColor, int fontSize, QString fontWeight, int radius)
    {

        cell->setStyleSheet(
            QString(
                "background-color: rgb(%1,%2,%3);"
                "color: %4;"
                "font-size: %5px;"
                "font-weight: %6;"
                "border-radius: %7px;")
                .arg(r)
                .arg(g)
                .arg(b)
                .arg(textColor)
                .arg(fontSize)
                .arg(fontWeight)
                .arg(radius));
    }
};
