#include "device_driver.h"
#include "graphics.h"
#include <stdlib.h>

#define BASE (500) // msec

static void Buzzer_Beep(unsigned char tone, int duration)
{
    const static unsigned short tone_value[] = {261, 277, 293, 311, 329, 349, 369, 391, 415, 440, 466, 493, 523, 554, 587, 622, 659, 698, 739, 783, 830, 880, 932, 987};

    TIM3_Out_Freq_Generation(tone_value[tone]);
    TIM2_Delay(duration);
    TIM3_Out_Stop();
}

void ScoreSound(int index1, int index2)
{
    enum key
    {
        C1,
        C1_,
        D1,
        D1_,
        E1,
        F1,
        F1_,
        G1,
        G1_,
        A1,
        A1_,
        B1,
        C2,
        C2_,
        D2,
        D2_,
        E2,
        F2,
        F2_,
        G2,
        G2_,
        A2,
        A2_,
        B2
    };
    enum note
    {
        N16 = BASE / 4,
        N8 = BASE / 2,
        N4 = BASE,
        N2 = BASE * 2,
        N1 = BASE * 4
    };
    const int song1[][2] = {{G1, N4}, {G1, N4}, {E1, N8}, {F1, N8}, {G1, N4}, {A1, N4}, {A1, N4}, {G1, N2}, {G1, N4}, {C2, N4}, {E2, N4}, {D2, N8}, {C2, N8}, {D2, N2}};
    const char *note_name[] = {"C1", "C1#", "D1", "D1#", "E1", "F1", "F1#", "G1", "G1#", "A1", "A1#", "B1", "C2", "C2#", "D2", "D2#", "E2", "F2", "F2#", "G2", "G2#", "A2", "A2#", "B2"};

    int n = song1[index1][0]; // key
    int m = song1[index2][1]; // note
    TIM3_Out_Init();
    Buzzer_Beep(n, m);
}

/*==========================================================*
 *
 * STL related function override
 *
 *==========================================================*/

extern "C" void __cxa_pure_virtual()
{
    Uart1_Printf("Pure virtual function call\n");
    while (1)
        ;
}

extern "C" void __cxa_deleted_virtual()
{
    Uart1_Printf("Deleted virtual function call\n");
    while (1)
        ;
}

namespace __gnu_cxx
{
    void __verbose_terminate_handler()
    {
        Uart1_Printf("Unexpected error\n");
        while (1)
            ;
    }
}

/*==========================================================*
 *
 * STL related function override END
 *
 *==========================================================*/

/*==========================================================*
 *
 * DEFINE
 *
 *==========================================================*/

#define LCDW (320)
#define LCDH (240)
#define X_MIN (0)
#define X_MAX (LCDW - 1)
#define Y_MIN (0)
#define Y_MAX (LCDH - 1)

#define TIMER_PERIOD (10)

#define MAX_CAR (5)
#define MAX_CAR_STEP (10)
#define MIN_CAR_STEP (3)
#define CAR_SIZE_X (20)
#define CAR_SIZE_Y (20)

#define FROG_STEP (10)
#define FROG_SIZE_X (20)
#define FROG_SIZE_Y (20)

#define MAX_POOP (10)
#define POOP_SIZE_X (20)
#define POOP_SIZE_Y (20)
#define POOP_STEP (1)

#define BACK_COLOR (6)

#define SCORE_COLOR (4)
#define SCORE_SCALE (20)

#define GAME_OVER (1)

typedef void (*DrawFunc)(int, int, int, int, short);

/*==========================================================*
 *
 * DEFINE END
 *
 *==========================================================*/

/*==========================================================*
 *
 * ENUM
 *
 *==========================================================*/

enum SHAPE
{
    CIRCLE,
    RECTANGLE,
    INVALID,
    SHAPE_COUNT
};

enum DIR
{
    NONE = -1,
    UP = 0,
    DOWN = 1,
    LEFT = 2,
    RIGHT = 3,
    DIR_COUNT
};

enum FROG_GOAL
{
    SCHOOL,
    HOME
};

enum CURRENT_GAME
{
    FROGGER,
    POOP,
    GAME_COUNT
};

enum GAME_STATUS
{
    STANDBY,
    RUNNING,
    GAMEOVER
};

/*==========================================================*
 *
 * ENUM END
 *
 *==========================================================*/

/*==========================================================*
 *
 * FUNCTION PROTOTYPE
 *
 *==========================================================*/

void lcdDrawChar(int x, int y, int num, short colorIndex, int scale);
void lcdDrawNumber(int x, int y, int number, short colorIndex, int scale);
void lcdDrawString(int x, int y, const char *string, short colorIndex, int scale);
void setRandomSeed();
void setSeedADC();
uint32_t Read_ADC_Noise();
int max(int a, int b);
int min(int a, int b);
int randInt(int min, int max);
void drawRectangle(int x, int y, int sizeX, int sizeY, short colorIndex);
void drawCircle(int x, int y, int sizeX, int sizeY, short colorIndex);
void drawInvalid(int x, int y, int sizeX, int sizeY, short colorIndex);

/*==========================================================*
 *
 * FUNCTION PROTOTYPE END
 *
 *==========================================================*/

/*==========================================================*
 *
 * GLOBAL VAR
 *
 *==========================================================*/

static unsigned short color[] = {RED, YELLOW, GREEN, BLUE, WHITE, VIOLET, BLACK};
volatile int i = 0;
static unsigned short carY[] = {140, 100, 180, 60, 120};
// static unsigned short healY[] = {20, 80, 160, 200};

extern volatile int TIM4_expired;
extern volatile int USART1_rx_ready;
extern volatile int USART1_rx_data;
extern volatile int Jog_key_in;
extern volatile int Jog_key;

const static unsigned char fontNum[10][5] = {
    {0b111, 0b101, 0b101, 0b101, 0b111}, // 0
    {0b010, 0b110, 0b010, 0b010, 0b010}, // 1
    {0b111, 0b001, 0b111, 0b100, 0b111}, // 2
    {0b111, 0b001, 0b111, 0b001, 0b111}, // 3
    {0b101, 0b101, 0b111, 0b001, 0b001}, // 4
    {0b111, 0b100, 0b111, 0b001, 0b111}, // 5
    {0b111, 0b100, 0b111, 0b101, 0b111}, // 6
    {0b111, 0b001, 0b001, 0b001, 0b001}, // 7
    {0b111, 0b101, 0b111, 0b101, 0b111}, // 8
    {0b111, 0b101, 0b111, 0b001, 0b111}  // 9
};

const static unsigned char fontAlpha[26][5] = {
    {0b010, 0b101, 0b111, 0b101, 0b101}, // A
    {0b110, 0b101, 0b110, 0b101, 0b110}, // B
    {0b011, 0b100, 0b100, 0b100, 0b011}, // C
    {0b110, 0b101, 0b101, 0b101, 0b110}, // D
    {0b111, 0b100, 0b110, 0b100, 0b111}, // E
    {0b111, 0b100, 0b110, 0b100, 0b100}, // F
    {0b011, 0b100, 0b100, 0b101, 0b011}, // G
    {0b101, 0b101, 0b111, 0b101, 0b101}, // H
    {0b111, 0b010, 0b010, 0b010, 0b111}, // I
    {0b111, 0b001, 0b001, 0b101, 0b010}, // J
    {0b101, 0b101, 0b110, 0b101, 0b101}, // K
    {0b100, 0b100, 0b100, 0b100, 0b111}, // L
    {0b101, 0b111, 0b101, 0b101, 0b101}, // M
    {0b101, 0b111, 0b111, 0b101, 0b101}, // N
    {0b010, 0b101, 0b101, 0b101, 0b010}, // O
    {0b110, 0b101, 0b110, 0b100, 0b100}, // P
    {0b010, 0b101, 0b101, 0b011, 0b001}, // Q
    {0b110, 0b101, 0b110, 0b101, 0b101}, // R
    {0b011, 0b100, 0b010, 0b001, 0b110}, // S
    {0b111, 0b010, 0b010, 0b010, 0b010}, // T
    {0b101, 0b101, 0b101, 0b101, 0b011}, // U
    {0b101, 0b101, 0b101, 0b101, 0b010}, // V
    {0b101, 0b101, 0b101, 0b111, 0b101}, // W
    {0b101, 0b101, 0b010, 0b101, 0b101}, // X
    {0b101, 0b101, 0b010, 0b010, 0b010}, // Y
    {0b111, 0b001, 0b010, 0b100, 0b111}  // Z
};

DrawFunc drawFunctions[SHAPE_COUNT] = {
    drawCircle,    // CIRCLE
    drawRectangle, // RECTANGLE
    drawInvalid    // INVALID
};

/*==========================================================*
 *
 * GLOBAL VAR END
 *
 *==========================================================*/

/*==========================================================*
 *
 * CLASS
 *
 *==========================================================*/

class IObject
{
protected:
    int x, y;
    int sizeX, sizeY;
    int speed;
    DIR dir;
    short colorIndex;
    SHAPE shape;

public:
    IObject() {}
    IObject(int x, int y, int sizeX, int sizeY, short colorIndex, SHAPE shape) : x(x), y(y), sizeX(sizeX), sizeY(sizeY), speed(0), dir(NONE), colorIndex(colorIndex), shape(shape) {}
    virtual ~IObject() {}

    virtual bool isOutOfBound()
    {
        if (x < X_MIN || x + sizeX > X_MAX)
            return true;
        if (y < Y_MIN || y + sizeY > Y_MAX)
            return true;
        return false;
    }
    virtual bool isCollideWith(IObject *object)
    {
        if (object->getX() + object->getSizeX() < x || object->getX() > x + sizeX)
            return false;
        if (object->getY() + object->getSizeY() < y || object->getY() > y + sizeY)
            return false;
        return true;
    }
    virtual void draw()
    {
        drawFunctions[shape](x, y, sizeX, sizeY, colorIndex);
    }
    virtual void clear()
    {
        drawFunctions[shape](x, y, sizeX, sizeY, BACK_COLOR);
    }
    virtual void setX(int x) { this->x = x; }
    virtual void setY(int y) { this->y = y; }
    virtual int getX() { return x; }
    virtual int getY() { return y; }
    virtual int getSpeed() { return speed; }
    virtual DIR getDir() { return dir; }
    virtual int getSizeX() { return sizeX; }
    virtual int getSizeY() { return sizeY; }
    virtual void setSpeed(int speed) { this->speed = speed; }
    virtual void setDir(DIR dir) { this->dir = dir; }
    virtual short getColorIndex() { return colorIndex; }
    virtual SHAPE getShape() { return shape; }
    virtual void move() = 0;
};

class Player : public IObject
{
public:
    Player() {}
    Player(int x, int y, int sizeX, int sizeY, short colorIndex, SHAPE shape, int speed, DIR dir) : IObject(x, y, sizeX, sizeY, colorIndex, shape)
    {
        this->speed = speed;
        this->dir = dir;
    }
    virtual ~Player() {}
    virtual void move() = 0;
};

class Frog : public Player
{
private:
    FROG_GOAL goal;

public:
    Frog() {}
    Frog(int x, int y, int sizeX, int sizeY, short colorIndex, SHAPE shape, int speed, DIR dir, FROG_GOAL goal) : Player(x, y, sizeX, sizeY, colorIndex, shape, speed, dir), goal(goal) {}
    ~Frog() {}
    void setGoal(FROG_GOAL goal) { this->goal = goal; }
    FROG_GOAL getGoal() { return goal; }
    bool isReachGoal(FROG_GOAL goal)
    {
        switch (goal)
        {
        case SCHOOL:
            return y == Y_MIN;
        case HOME:
            return y == Y_MAX - sizeY;
        }
        return false;
    }
    void setDir()
    {
        IObject::setDir(static_cast<DIR>(Jog_key));
    }
    void move() override
    {
        switch (dir)
        {
        case UP:
            y = max(y - speed, Y_MIN);
            break;
        case DOWN:
            y = min(y + speed, Y_MAX - sizeY);
            break;
        case LEFT:
            x = max(x - speed, X_MIN);
            break;
        case RIGHT:
            x = min(x + speed, X_MAX - sizeX);
            break;
        default:
            break;
        }
    }
};

class Obstacle : public IObject
{
public:
    Obstacle() {}
    Obstacle(int x, int y, int sizeX, int sizeY, short colorIndex, SHAPE shape) : IObject(x, y, sizeX, sizeY, colorIndex, shape) {}
    ~Obstacle() {}
    virtual void move() = 0;
};

class Car : public Obstacle
{
public:
    Car() {}
    Car(int x, int y, int sizeX, int sizeY, short colorIndex, SHAPE shape, int speed) : Obstacle(x, y, sizeX, sizeY, colorIndex, shape)
    {
        this->speed = speed;
        this->dir = RIGHT;
    }
    ~Car() {}
    void move() override
    {
        switch (dir)
        {
        case LEFT:
            x = max(x - speed, X_MIN);
            if (x == X_MIN)
                dir = RIGHT;
            break;
        case RIGHT:
            x = min(x + speed, X_MAX - sizeX);
            if (x == X_MAX - sizeX)
                dir = LEFT;
            break;
        default:
            break;
        }
    }
};

class Game
{
protected:
    int life;
    int score;

public:
    Game() {}
    Game(int life, int score) : life(life), score(score) {}
    virtual ~Game() {}

    int getScore() { return score; }
    int getLife() { return life; }
    void addScore(int score) { this->score += score; }
    void addLife(int life) { this->life += life; }

    virtual void init() = 0;
    virtual void updatePerUser() = 0;
    virtual void updatePerTick() = 0;
    virtual bool isGameOver() = 0;
};

class FrogGame : public Game
{
private:
    Frog frog;
    Car cars[MAX_CAR];
    int currentCar;

public:
    FrogGame() {}
    ~FrogGame() {}
    FrogGame(int life, int score) : Game(life, score) {}
    void addCar()
    {
        if (currentCar >= MAX_CAR)
            return;
        currentCar++;
        cars[currentCar - 1] = Car(randInt(X_MIN, X_MAX - CAR_SIZE_X), carY[currentCar - 1], CAR_SIZE_X, CAR_SIZE_Y, currentCar, RECTANGLE, randInt(MIN_CAR_STEP, MAX_CAR_STEP));
        cars[currentCar - 1].draw();
    }
    void addMultipleCars(int num)
    {
        for (int i = 0; i < num; i++)
        {
            addCar();
        }
    }
    void init() override
    {
        life = 3;
        score = 0;
        frog = Frog(150, 220, FROG_SIZE_X, FROG_SIZE_Y, 0, RECTANGLE, FROG_STEP, UP, SCHOOL);
        frog.draw();
        currentCar = 0;
        addCar();
        for (i = 0; i < currentCar; i++)
        {
            cars[i].draw();
        }
    }
    void updatePerUser() override
    {
        frog.setDir();
        frog.clear();
        frog.move();
        frog.draw();
        Jog_key_in = 0;
    }
    void updatePerTick() override
    {
        if (frog.isReachGoal(frog.getGoal()))
        {
            if (frog.getGoal() == HOME)
            {
                score++;
                addCar();
            }
            frog.setGoal(frog.getGoal() == SCHOOL ? HOME : SCHOOL);
        }

        static int prevKey = 0;
        int currentKey = Jog_Get_Pressed_Calm();

        if (currentKey && prevKey)
        {
            frog.clear();
            frog.move();
            frog.draw();
        }

        prevKey = currentKey;

        for (int i = 0; i < currentCar; i++)
        {
            cars[i].clear();
            cars[i].move();
            cars[i].draw();
            if (cars[i].isCollideWith(&frog))
            {
                life--;
                ScoreSound(2, 3);
                if (life < 0)
                {
                    return;
                }
                frog.clear();
                frog.setX(150), frog.setY(220);
                frog.setGoal(SCHOOL);
                frog.draw();
            }
        }
    }
    bool isGameOver() override
    {
        if (life < 0)
        {
            return 1;
        }
        return 0;
    }
};

class Man : public Player
{
public:
    Man() {}
    Man(int x, int y, int sizeX, int sizeY, short colorIndex, SHAPE shape, int speed, DIR dir) : Player(x, y, sizeX, sizeY, colorIndex, shape, speed, dir) {}
    ~Man() {}
    void setDir()
    {
        if (Jog_key == 2 || Jog_key == 3)
            IObject::setDir(static_cast<DIR>(Jog_key));
    }
    void move() override
    {
        switch (dir)
        {
        case LEFT:
            x = max(x - speed, X_MIN);
            break;
        case RIGHT:
            x = min(x + speed, X_MAX - sizeX);
            break;
        default:
            break;
        }
    }
};

class Poop : public Obstacle
{
public:
    Poop() {}
    Poop(int x, int y, int sizeX, int sizeY, short colorIndex, SHAPE shape, int speed) : Obstacle(x, y, sizeX, sizeY, colorIndex, shape)
    {
        this->speed = speed;
        this->dir = DOWN;
    }
    ~Poop() {}
    void move() override
    {
        y = min(y + speed, Y_MAX - sizeY);
        speed++;
        if (speed > 20)
        {
            speed = 20;
        }
        if (y >= Y_MAX - sizeY)
        {
            clear();
            y = -30;
        }
    }
};

class PoopGame : public Game
{
private:
    Man man;
    Poop poops[MAX_POOP];
    int currentPoop;

public:
    PoopGame() {}
    ~PoopGame() {}
    PoopGame(int life, int score) : Game(life, score) {}
    void addPoop()
    {
        if (currentPoop >= MAX_POOP)
            return;
        currentPoop++;
        for (i = 0; i < MAX_POOP; i++)
        {
            if (poops[i].getY() < 0)
            {
                poops[i].setX(randInt(X_MIN, X_MAX - POOP_SIZE_X));
                poops[i].setY(0);
                poops[i].setSpeed(POOP_STEP);
                poops[i].draw();
                break;
            }
        }
    }
    void init() override
    {
        life = 3;
        score = 0;
        currentPoop = 0;
        for (i = 0; i < MAX_POOP; i++)
        {
            poops[i] = Poop(0, -30, POOP_SIZE_X, POOP_SIZE_Y, 3, RECTANGLE, POOP_STEP);
        }
        man = Man(150, 220, FROG_SIZE_X, FROG_SIZE_Y, 0, RECTANGLE, FROG_STEP, RIGHT);
        man.draw();
        addPoop();
    }
    void updatePerUser() override
    {
        man.setDir();
        man.clear();
        man.move();
        man.draw();
        Jog_key_in = 0;
    }
    void updatePerTick() override
    {
        static int prevKey = 0;
        int currentKey = Jog_Get_Pressed_Calm();
        if (currentKey && prevKey && (currentKey == 8 || currentKey == 4))
        {
            man.clear();
            man.move();
            man.draw();
        }

        prevKey = currentKey;
        // randomlly add poop
        static bool canAddPoop = false;
        canAddPoop = (rand() % 100) < 50;
        if (canAddPoop)
        {
            addPoop();
        }
        // move all valid poops
        for (i = 0; i < MAX_POOP; i++)
        {
            if (poops[i].getY() == -30)
            {
                continue;
            }
            poops[i].clear();
            poops[i].move();
            if (poops[i].getY() == -30)
            {
                currentPoop--;
                score++;
                continue;
            }
            if (poops[i].isCollideWith(&man))
            {
                ScoreSound(2, 3);
                currentPoop--;
                poops[i].setY(-30);
                man.clear();
                man.draw();
                life--;
                if (life < 0)
                {
                    return;
                }
            }
            poops[i].draw();
            poops[i].setSpeed(poops[i].getSpeed() + 1);
        }
    }
    bool isGameOver() override
    {
        if (life < 0)
        {
            return 1;
        }
        return 0;
    }
};

class GameManager
{
private:
    FrogGame frogGame;
    PoopGame poopGame;
    CURRENT_GAME currentGame;
    GAME_STATUS status;
    bool isTitleDrawn;
    bool isGameChanged;

public:
    GameManager()
    {
        frogGame = FrogGame(3, 0);
        poopGame = PoopGame(3, 0);
        isTitleDrawn = false;
        isGameChanged = true;
        currentGame = FROGGER;
        status = STANDBY;
    }
    ~GameManager() {}
    void init()
    {
        isTitleDrawn = false;
        isGameChanged = true;
        currentGame = FROGGER;
        status = STANDBY;
        while (status == STANDBY)
        {
            standby();
            if (Jog_key_in)
            {
                Jog_key_in = 0;
                if (Jog_key == 4)
                {
                    break;
                }
                selectGame();
            }
        }
    }
    void setGame(CURRENT_GAME game)
    {
        currentGame = game;
    }
    void selectGame()
    {
        switch (Jog_key)
        {
        case (2):
            currentGame = static_cast<CURRENT_GAME>(max(static_cast<int>(currentGame) - 1, 0));
            isGameChanged = true;
            break;
        case (3):
            currentGame = static_cast<CURRENT_GAME>(min(static_cast<int>(currentGame) + 1, static_cast<int>(GAME_COUNT) - 1));
            isGameChanged = true;
            break;
        default:
            break;
        }
    }
    void standby()
    {
        drawStandbyTitle();
        drawStandbyGameName();
    }
    void drawStandbyTitle()
    {
        if (isTitleDrawn)
            return;
        Lcd_Draw_Box(0, 0, LCDW, 30, color[BACK_COLOR]);
        lcdDrawString(30, 0, "SELECT GAME", 4, 6);
        Lcd_Draw_Box(0, 150, LCDW, 50, color[BACK_COLOR]);
        lcdDrawString(58, 150, "PUSH BUTTON A", 4, 4);
        lcdDrawString(98, 180, "TO START", 4, 4);
        isTitleDrawn = true;
    }
    void drawStandbyGameName()
    {
        if (!isGameChanged)
            return;
        Lcd_Draw_Box(0, 80, LCDW, 30, color[BACK_COLOR]);
        switch (currentGame)
        {
        case FROGGER:
            lcdDrawString(73, 80, "FROGGER", 4, 6);
            break;
        case POOP:
            lcdDrawString(109, 80, "POOP", 4, 6);
            break;
        default:
            break;
        }
        isGameChanged = false;
    }
    void initGame()
    {
        Lcd_Clr_Screen();
        switch (currentGame)
        {
        case FROGGER:
            frogGame.init();
            break;
        case POOP:
            poopGame.init();
            break;
        default:
            break;
        }
    }
    void startGame()
    {
        status = RUNNING;
        TIM4_Repeat_Interrupt_Enable(1, TIMER_PERIOD * 10);
    }
    void updateGamePerUser()
    {
        switch (currentGame)
        {
        case FROGGER:
            frogGame.updatePerUser();
            break;
        case POOP:
            poopGame.updatePerUser();
            break;
        default:
            break;
        }
    }
    void updateGamePerTick()
    {
        switch (currentGame)
        {
        case FROGGER:
            frogGame.updatePerTick();
            if (frogGame.isGameOver())
            {
                status = GAMEOVER;
            }
            break;
        case POOP:
            poopGame.updatePerTick();
            if (poopGame.isGameOver())
            {
                status = GAMEOVER;
            }
            break;
        default:
            break;
        }
    }
    void gameOver()
    {
        static int score = 0;
        Uart1_Printf("Game Over, press Push Button to continue.\n");
        switch (currentGame)
        {
        case FROGGER:
            score = frogGame.getScore();
            break;
        case POOP:
            score = poopGame.getScore();
            break;
        default:
            break;
        }
        Uart1_Printf("Your score is %d\n", score);
        Lcd_Clr_Screen();
        lcdDrawString(84, 0, "SCORE", 4, 8);
        lcdDrawNumber(90, 60, score, SCORE_COLOR, SCORE_SCALE);
        lcdDrawString(58, 190, "PUSH BUTTON B", 4, 4);
        lcdDrawString(98, 220, "TO RESET", 4, 4);
        while (1)
        {
            if (Jog_key_in)
            {
                Jog_key_in = 0;
                if (Jog_key == 5)
                {
                    break;
                }
            }
        }
    }
    GAME_STATUS getStatus()
    {
        return status;
    }
};

/*==========================================================*
 *
 * CLASS END
 *
 *==========================================================*/

/*==========================================================*
 *
 * LCD RELATED FUNCTION
 *
 *==========================================================*/

void lcdDrawSingleNum(int x, int y, int num, short colorIndex, int scale)
{
    if (num < 0 || num > 9)
        return;

    const unsigned char *bitmap = fontNum[num];
    for (int row = 0; row < 5; row++)
    {
        for (int col = 0; col < 3; col++)
        {
            if (bitmap[row] & (1 << (2 - col)))
            {
                Lcd_Draw_Box(x + col * scale, y + row * scale, scale, scale, color[colorIndex]);
            }
        }
    }
}

void lcdDrawSingleChar(int x, int y, char c, short colorIndex, int scale)
{
    if (c < 'A' || c > 'Z')
        return;

    const unsigned char *bitmap = fontAlpha[c - 'A'];
    for (int row = 0; row < 5; row++)
    {
        for (int col = 0; col < 3; col++)
        {
            if (bitmap[row] & (1 << (2 - col)))
            {
                Lcd_Draw_Box(x + col * scale, y + row * scale, scale, scale, color[colorIndex]);
            }
        }
    }
}

void lcdDrawNumber(int x, int y, int number, short colorIndex, int scale)
{
    if (number < 0 || number > 99)
        return;

    lcdDrawSingleNum(x, y, number / 10, colorIndex, scale);
    // Char width + spaceing added to x
    lcdDrawSingleNum(x + 3 * scale + scale, y, number % 10, colorIndex, scale);
}

void lcdDrawString(int x, int y, const char *string, short colorIndex, int scale)
{
    int i = 0;
    while (string[i] != '\0')
    {
        if (string[i] == ' ')
        {
            i++;
            continue;
        }
        lcdDrawSingleChar(x + i * 4 * scale, y, string[i], colorIndex, scale);
        i++;
    }
}

/*==========================================================*
 *
 * LCD RELATED FUNCTION END
 *
 *==========================================================*/

/*==========================================================*
 *
 * Something Spooky
 *
 *==========================================================*/

void setRandomSeed()
{
    extern volatile uint32_t __ZI_LIMIT__;
    srand(*(uint32_t *)(&__ZI_LIMIT__));
}

uint32_t Read_ADC_Noise()
{
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    ADC1->SQR3 = 0;
    ADC1->CR2 |= ADC_CR2_ADON;
    for (volatile int i = 0; i < 1000; i++)
        ;
    ADC1->CR2 |= ADC_CR2_ADON;
    while (!(ADC1->SR & ADC_SR_EOC))
        ;
    return ADC1->DR;
}

void setSeedADC()
{
    srand(Read_ADC_Noise());
}

int max(int a, int b)
{
    return a > b ? a : b;
}

int min(int a, int b)
{
    return a < b ? a : b;
}

int randInt(int min, int max)
{
    return rand() % (max - min + 1) + min;
}

/*==========================================================*
 *
 * Spooky Scary Skeletons
 *
 *==========================================================*/

/*==========================================================*
 *
 * DRAW FUNCTION
 *
 *==========================================================*/

void drawRectangle(int x, int y, int sizeX, int sizeY, short colorIndex)
{
    Lcd_Draw_Box(x, y, sizeX, sizeY, color[colorIndex]);
}

void drawCircle(int x, int y, int sizeX, int sizeY, short colorIndex)
{
    Uart1_Printf("Currently not supported\n");
    // Lcd_Draw_Circle(x, y, size, color[colorIndex]);
}

void drawInvalid(int x, int y, int sizeX, int sizeY, short colorIndex)
{
    Uart1_Printf("Invalid shape\n");
}

/*==========================================================*
 *
 * DRAW FUNCTION END
 *
 *==========================================================*/

/*==========================================================*
 *
 * SYSTEM RELATED FUNCTION
 *
 *==========================================================*/

extern "C" void abort(void)
{
    while (1)
        ;
}

static void Sys_Init(void)
{
    setSeedADC();
    Clock_Init();
    LED_Init();
    Uart_Init(115200);

    SCB->VTOR = 0x08003000;
    SCB->SHCSR = 7 << 16;
}

/*==========================================================*
 *
 * SYSTEM RELATED FUNCTION END
 *
 *==========================================================*/

/*==========================================================*
 *
 * MAIN
 *
 *==========================================================*/

extern "C" void Main()
{
    Sys_Init();
    Uart_Printf("Game Example\n\n");

    Lcd_Init();
    Jog_Poll_Init();
    Jog_ISR_Enable(1);
    Uart1_RX_Interrupt_Enable(1);

    GameManager gameManager;

    for (;;)
    {
        Lcd_Clr_Screen();
        gameManager.init();
        gameManager.initGame();
        gameManager.startGame();
        while (1)
        {
            if (Jog_key_in)
            {
                gameManager.updateGamePerUser();
                Jog_key_in = 0;
            }
            if (TIM4_expired)
            {
                gameManager.updateGamePerTick();
                TIM4_expired = 0;
            }
            if (gameManager.getStatus() == GAMEOVER)
            {
                gameManager.gameOver();
                break;
            }
        }
    }
}


/*==========================================================*
 *
 * MAIN END
 *
 *==========================================================*/

/*==========================================================*
 *
 * MAIN END
 *
 *==========================================================*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// #include "device_driver.h"
// #include "graphics.h"

// /*---------- STL related function override START ----------*/
// extern "C" void __cxa_pure_virtual()
// {
// 	Uart1_Printf("Pure virtual function call\n");
// 	while (1)
// 		;
// }

// extern "C" void __cxa_deleted_virtual()
// {
// 	Uart1_Printf("Deleted virtual function call\n");
// 	while (1)
// 		;
// }

// namespace __gnu_cxx
// {
// 	void __verbose_terminate_handler()
// 	{
// 		Uart1_Printf("Unexpected error\n");
// 		while (1)
// 			;
// 	}
// }

// /*========== STL related function override END ==========*/

// /*---------- DEFINE START ----------*/

// #define LCDW (320)
// #define LCDH (240)
// #define X_MIN (0)
// #define X_MAX (LCDW - 1)
// #define Y_MIN (20)
// #define Y_MAX (LCDH - 1)

// #define TIMER_PERIOD (10)
// #define RIGHT (1)
// #define LEFT (-1)

// #define CAR_STEP (5)
// #define CAR_SIZE_X (20)
// #define CAR_SIZE_Y (20)
// #define FROG_STEP (10)
// #define FROG_SIZE_X (20)
// #define FROG_SIZE_Y (20)

// #define BACK_COLOR (6)
// #define FROG_COLOR (0)

// #define SCORE_X (90)
// #define SCORE_Y (50)
// #define SCORE_COLOR (4)
// #define SCORE_SCALE (20)

// #define GAME_OVER (1)

// #define MAX_CAR 5

// /*========== DEFINE END ==========*/

// /*---------- ENUM START ----------*/

// enum BORDER
// {
// 	LEFT_BORDER = 0,
// 	RIGHT_BORDER = 1,
// 	TOP_BORDER = 2,
// 	BOTTOM_BORDER = 3,
// 	NO_BORDER = -1
// };

// enum FROG_GOAL
// {
// 	HOME,
// 	SCHOOL
// };

// enum GAME_STATUS
// {
// 	RUNNING,
// 	GAMEOVER
// };

// /*========== ENUM END ==========*/

// /*---------- CLASS DECLARATION START ----------*/

// class IObject;
// class Square;
// class Frog;
// class Obstacle;
// class DynamicObstacle;
// class Car;
// class ScoreBoard;
// class GameManager;

// /*========== CLASS DECLARATION END ==========*/

// /*---------- FUNCTION DECLARATION START ----------*/

// void lcdDrawChar(int x, int y, int num, unsigned short color, int scale);
// void lcdDrawNumber(int x, int y, int number, unsigned short color, int scale);
// void lcdDrawString(int x, int y, const char * string, unsigned short color, int scale);

// /*========== FUNCTION DECLARATION END ==========*/

// /*---------- GLOBAL VAR START ----------*/

// static unsigned short color[] = {RED, YELLOW, GREEN, BLUE, WHITE, VIOLET, BLACK};
// volatile int i = 0;

// extern volatile int TIM4_expired;
// extern volatile int USART1_rx_ready;
// extern volatile int USART1_rx_data;
// extern volatile int Jog_key_in;
// extern volatile int Jog_key;

// const static unsigned char fontNum[10][5] = {
// 	{0b111, 0b101, 0b101, 0b101, 0b111}, // 0
// 	{0b010, 0b010, 0b010, 0b010, 0b010}, // 1
// 	{0b111, 0b001, 0b111, 0b100, 0b111}, // 2
// 	{0b111, 0b001, 0b111, 0b001, 0b111}, // 3
// 	{0b101, 0b101, 0b111, 0b001, 0b001}, // 4
// 	{0b111, 0b100, 0b111, 0b001, 0b111}, // 5
// 	{0b111, 0b100, 0b111, 0b101, 0b111}, // 6
// 	{0b111, 0b001, 0b001, 0b001, 0b001}, // 7
// 	{0b111, 0b101, 0b111, 0b101, 0b111}, // 8
// 	{0b111, 0b101, 0b111, 0b001, 0b111}	 // 9
// };

// const static unsigned char fontAlpha[26][5] = {
// 	{0b010, 0b101, 0b111, 0b101, 0b101}, // A
// 	{0b110, 0b101, 0b110, 0b101, 0b110}, // B
// 	{0b011, 0b100, 0b100, 0b100, 0b011}, // C
// 	{0b110, 0b101, 0b101, 0b101, 0b110}, // D
// 	{0b111, 0b100, 0b110, 0b100, 0b111}, // E
// 	{0b111, 0b100, 0b110, 0b100, 0b100}, // F
// 	{0b011, 0b100, 0b100, 0b101, 0b011}, // G
// 	{0b101, 0b101, 0b111, 0b101, 0b101}, // H
// 	{0b111, 0b010, 0b010, 0b010, 0b111}, // I
// 	{0b111, 0b001, 0b001, 0b101, 0b010}, // J
// 	{0b101, 0b101, 0b110, 0b101, 0b101}, // K
// 	{0b100, 0b100, 0b100, 0b100, 0b111}, // L
// 	{0b101, 0b111, 0b101, 0b101, 0b101}, // M
// 	{0b101, 0b111, 0b111, 0b101, 0b101}, // N
// 	{0b010, 0b101, 0b101, 0b101, 0b010}, // O
// 	{0b110, 0b101, 0b110, 0b100, 0b100}, // P
// 	{0b010, 0b101, 0b101, 0b011, 0b001}, // Q
// 	{0b110, 0b101, 0b110, 0b101, 0b101}, // R
// 	{0b011, 0b100, 0b010, 0b001, 0b110}, // S
// 	{0b111, 0b010, 0b010, 0b010, 0b010}, // T
// 	{0b101, 0b101, 0b101, 0b101, 0b011}, // U
// 	{0b101, 0b101, 0b101, 0b101, 0b010}, // V
// 	{0b101, 0b101, 0b101, 0b111, 0b101}, // W
// 	{0b101, 0b101, 0b010, 0b101, 0b101}, // X
// 	{0b101, 0b101, 0b010, 0b010, 0b010}, // Y
// 	{0b111, 0b001, 0b010, 0b100, 0b111}	 // Z
// };

// /*========== GLOBAL VAR END ==========*/

// /*---------- CLASS START ----------*/

// // Interface for all objects
// class IObject
// {
// public:
// 	IObject() {}
// 	virtual ~IObject() {}
// 	virtual void draw() = 0;
// 	virtual void move() = 0;
// 	virtual int isSquare() = 0;
// 	virtual int checkObjCollision(IObject *obj) = 0;
// };

// class Square : public IObject
// {
// protected:
// 	int x, y;
// 	int w, h;
// 	int ci;

// public:
// 	Square() {}
// 	Square(int x, int y, int w, int h, int ci) : x(x), y(y), w(w), h(h), ci(ci) {}
// 	~Square() {}
// 	void draw() override;
// 	int isSquare() override;
// 	int checkObjCollision(IObject *obj) override;
// 	BORDER checkBorderCollision();
// 	virtual void move() = 0;
// };

// class Frog : public Square
// {
// private:
// 	int goal;
// 	int dir;

// public:
// 	Frog() {}
// 	~Frog() {}
// 	Frog(int x, int y, int w, int h, int ci, int goal, int dir) : Square(x, y, w, h, ci), goal(goal), dir(dir) {}
// 	void move() override;
// 	int checkBorder();
// 	void drawBlack();
// };

// class Obstacle : public Square
// {
// protected:
// 	int isMoving;
// 	int isGameOver;

// public:
// 	Obstacle() {}
// 	~Obstacle() {}
// 	Obstacle(int x, int y, int w, int h, int ci) : Square(x, y, w, h, ci), isMoving(0), isGameOver(1) {}
// 	virtual void move() = 0;
// };

// class DynamicObstacle : public Obstacle
// {
// protected:
// 	int dir;

// public:
// 	DynamicObstacle() {}
// 	~DynamicObstacle() {}
// 	DynamicObstacle(int x, int y, int w, int h, int ci, int dir) : Obstacle(x, y, w, h, ci), dir(dir)
// 	{
// 		this->isMoving = 1;
// 	}
// 	virtual void move() = 0;
// };

// class Car : public DynamicObstacle
// {
// public:
// 	Car() {}
// 	~Car() {}
// 	Car(int x, int y, int w, int h, int ci, int dir) : DynamicObstacle(x, y, w, h, ci, dir) {}
// 	void move() override;
// };

// class ScoreBoard
// {
// private:
// 	int isScoreChanged;
// 	int score;
// 	int life;

// public:
// 	ScoreBoard() : isScoreChanged(1), score(0), life(3) {}
// 	~ScoreBoard() {}
// 	void addScore(int var);
// 	void addLife(int var);
// 	void resetScore();
// 	void drawScoreCenter();
// 	void drawHUD();
// 	int getScore();
// 	int getLife();

// 	friend class GameManager;
// };
// class GameManager
// {
// private:
// 	Frog frog;
// 	int curCar;
// 	Car cars[MAX_CAR];
// 	ScoreBoard scoreBoard;
// 	int gameStatus;

// public:
// 	GameManager() {}
// 	~GameManager() {}
// 	void initGame();
// 	void startGame();
// 	void updateGamePerTick();
// 	void updateGamePerUser();
// 	void endGame();
// 	void isGameOver();
// 	void addCar();
// };

// /*========== CLASS END ==========*/

// /*---------- CLASS FUNCTION DEFINITION START ----------*/

// // draw a square
// void Square::draw()
// {
// 	Lcd_Draw_Box(x, y, w, h, color[ci]);
// }

// // return 1 if the object is a square
// int Square::isSquare()
// {
// 	return 1;
// }

// // if two squares collide, return 1
// // if parameter obj is not a square, return -1
// int Square::checkObjCollision(IObject *obj)
// {
// 	if (obj->isSquare() == 0)
// 		return -1;
// 	Square *temp = (Square *)obj;
// 	if (x > temp->x + temp->w || temp->x > x + w)
// 		return 0;
// 	if (y > temp->y + temp->h || temp->y > y + h)
// 		return 0;
// 	return 1;
// }

// // if the square collides with the border, return BORDER
// // otherwise, return NO_BORDER
// BORDER Square::checkBorderCollision()
// {
// 	if (this->x <= X_MIN)
// 		return LEFT_BORDER;
// 	if (this->x + this->w >= X_MAX)
// 		return RIGHT_BORDER;
// 	if (this->y <= Y_MIN)
// 		return TOP_BORDER;
// 	if (this->y + this->h >= Y_MAX)
// 		return BOTTOM_BORDER;
// 	return NO_BORDER;
// }

// // move the frog according to the Jog_key
// void Frog::move()
// {
// 	int temp = this->ci;
// 	this->ci = BACK_COLOR;
// 	this->draw();
// 	switch (Jog_key)
// 	{
// 	case 0:
// 		if (this->y > Y_MIN)
// 			this->y -= FROG_STEP;
// 		break;
// 	case 1:
// 		if (this->y + this->h < Y_MAX)
// 			this->y += FROG_STEP;
// 		break;
// 	case 2:
// 		if (this->x > X_MIN)
// 			this->x -= FROG_STEP;
// 		break;
// 	case 3:
// 		if (this->x + this->w < X_MAX)
// 			this->x += FROG_STEP;
// 		break;
// 	}
// 	this->ci = temp;
// 	this->draw();
// }

// // check the border and change the goal if necessary
// // return 1 if the frog gains a score
// int Frog::checkBorder()
// {
// 	static int collidedBorder = NO_BORDER;
// 	collidedBorder = this->checkBorderCollision();
// 	switch (collidedBorder)
// 	{
// 	// Nothing happens if the frog collides with the left or right border
// 	case NO_BORDER:
// 	case LEFT_BORDER:
// 	case RIGHT_BORDER:
// 		break;
// 	// If the frog collides with the top border and goal is SCHOOL, change the goal to HOME
// 	case TOP_BORDER:
// 		if (this->goal == SCHOOL)
// 		{
// 			Uart1_Printf("SCHOOL\n");
// 			this->goal = HOME;
// 		}
// 		break;
// 	// If the frog collides with the bottom border and goal is HOME, add a car and increase the score
// 	case BOTTOM_BORDER:
// 		if (this->goal == HOME)
// 		{
// 			Uart1_Printf("HOME\n");
// 			this->goal = SCHOOL;
// 			return 1;
// 		}
// 		break;
// 	}
// 	return 0;
// }

// void Frog::drawBlack()
// {
// 	Lcd_Draw_Box(this->x, this->y, this->w, this->h, BLACK);
// }

// // move the car
// void Car::move()
// {
// 	int temp = this->ci;
// 	this->ci = BACK_COLOR;
// 	this->draw();
// 	this->x += CAR_STEP * this->dir;
// 	// check border and change the direction if necessary
// 	if (this->checkBorderCollision() != NO_BORDER)
// 		this->dir = -this->dir;
// 	this->ci = temp;
// 	this->draw();
// }

// // add score
// void ScoreBoard::addScore(int var)
// {
// 	this->isScoreChanged = 1;
// 	this->score += var;
// 	if (this->score > 99)
// 		this->score = 99;
// }

// // add life
// void ScoreBoard::addLife(int var)
// {
// 	this->isScoreChanged = 1;
// 	this->life += var;
// 	if (this->life > 3)
// 		this->life = 3;
// }

// // reset score
// void ScoreBoard::resetScore()
// {
// 	this->score = 0;
// }

// // draw score
// void ScoreBoard::drawScoreCenter()
// {
// 	lcdDrawNumber(SCORE_X, SCORE_Y, this->score, color[SCORE_COLOR], SCORE_SCALE);
// }

// // get score
// int ScoreBoard::getScore()
// {
// 	return this->score;
// }

// // get life
// int ScoreBoard::getLife()
// {
// 	return this->life;
// }

// // draw the HUD
// void ScoreBoard::drawHUD()
// {
// 	Lcd_Draw_Box(0, 0, 320, 20, WHITE);
// 	lcdDrawString(0, 0, "LIFE", RED, 4);
// 	lcdDrawNumber(68, 0, this->life, RED, 4);
// 	lcdDrawString(104, 0, "SCORE", BLACK, 4);
// 	lcdDrawNumber(184, 0, this->score, BLACK, 4);
// 	this->isScoreChanged = 0;
// }

// // initialize the game
// void GameManager::initGame()
// {
// 	Lcd_Clr_Screen();
// 	this->scoreBoard.resetScore();
// 	this->curCar = 0;
// 	this->frog = Frog(150, 220, FROG_SIZE_X, FROG_SIZE_Y, FROG_COLOR, SCHOOL, 0);
// 	this->frog.draw();
// 	this->addCar();
// }

// // start the game
// void GameManager::startGame()
// {
// 	this->gameStatus = RUNNING;
// 	TIM4_Repeat_Interrupt_Enable(1, TIMER_PERIOD * 10);
// }

// // update the game
// void GameManager::updateGamePerTick()
// {
// 	if (this->scoreBoard.isScoreChanged)
// 	{
// 		this->scoreBoard.drawHUD();
// 	}
// 	for (i = 0; i < this->curCar; i++)
// 	{
// 		this->cars[i].move();
// 	}
// 	// check collision with cars
// 	for (i = 0; i < this->curCar; i++)
// 	{
// 		if (this->frog.checkObjCollision(&this->cars[i]) == 1)
// 		{
// 			this->scoreBoard.addLife(-1);
// 			this->frog.drawBlack();
// 			this->frog = Frog(150, 220, FROG_SIZE_X, FROG_SIZE_Y, FROG_COLOR, SCHOOL, 0);
// 			this->frog.draw();
// 			if (this->scoreBoard.getLife() == 0)
// 			{
// 				this->gameStatus = GAMEOVER;
// 			}
// 			break;
// 		}
// 	}
// }

// // update the game
// void GameManager::updateGamePerUser()
// {
// 	this->frog.move();
// 	if (this->frog.checkBorder())
// 	{
// 		this->scoreBoard.addScore(1);
// 		Uart1_Printf("Score: %d\n", this->scoreBoard.getScore());
// 		this->addCar();
// 	}
// }

// // end the game
// void GameManager::endGame()
// {
// 	TIM4_Repeat_Interrupt_Enable(0, 0);
// 	Uart_Printf("Game Over, your score is %d.\nPress Push Button to continue.\n", this->scoreBoard.getScore());
// 	Lcd_Clr_Screen();
// 	this->scoreBoard.drawScoreCenter();
// 	while (1)
// 	{
// 		if (Jog_key == 4 || Jog_key == 5)
// 		{
// 			this->initGame();
// 			this->startGame();
// 			Jog_key_in = 0;
// 			break;
// 		}
// 	}
// }

// // check if the game is over
// void GameManager::isGameOver()
// {
// 	if (this->gameStatus == GAMEOVER)
// 	{
// 		this->endGame();
// 	}
// }

// // add a car
// void GameManager::addCar()
// {
// 	if (this->curCar >= MAX_CAR)
// 		return;
// 	this->curCar++;
// 	this->cars[this->curCar - 1] = Car(10, 30 + 30 * curCar, CAR_SIZE_X, CAR_SIZE_Y, this->curCar, RIGHT);
// 	this->cars[this->curCar - 1].draw();
// }

// /*========== CLASS FUNCTION DEFINITION END ==========*/

// /*---------- LCD RELATED FUNCTION START ----------*/

// void lcdDrawSingleNum(int x, int y, int num, unsigned short color, int scale)
// {
// 	if (num < 0 || num > 9)
// 		return;

// 	const unsigned char *bitmap = fontNum[num];
// 	for (int row = 0; row < 5; row++)
// 	{
// 		for (int col = 0; col < 3; col++)
// 		{
// 			if (bitmap[row] & (1 << (2 - col)))
// 			{
// 				Lcd_Draw_Box(x + col * scale, y + row * scale, scale, scale, color);
// 			}
// 		}
// 	}
// }

// void lcdDrawSingleChar(int x, int y, char c, unsigned short color, int scale)
// {
// 	if (c < 'A' || c > 'Z')
// 		return;

// 	const unsigned char *bitmap = fontAlpha[c - 'A'];
// 	for (int row = 0; row < 5; row++)
// 	{
// 		for (int col = 0; col < 3; col++)
// 		{
// 			if (bitmap[row] & (1 << (2 - col)))
// 			{
// 				Lcd_Draw_Box(x + col * scale, y + row * scale, scale, scale, color);
// 			}
// 		}
// 	}
// }

// void lcdDrawNumber(int x, int y, int number, unsigned short color, int scale)
// {
// 	if (number < 0 || number > 99)
// 		return;

// 	lcdDrawSingleNum(x, y, number / 10, color, scale);
// 	// Char width + spaceing added to x
// 	lcdDrawSingleNum(x + 3 * scale + scale, y, number % 10, color, scale);
// }

// void lcdDrawString(int x, int y, const char * string, unsigned short color, int scale)
// {
// 	int i = 0;
// 	while (string[i] != '\0')
// 	{
// 		if (string[i] == ' ')
// 		{
// 			i++;
// 			continue;
// 		}
// 		lcdDrawSingleChar(x + i * 4 * scale, y, string[i], color, scale);
// 		i++;
// 	}
// }

// /*========== LCD RELATED FUNCTION END ==========*/

// /*---------- SYSTEM RELATED FUNCTION START ----------*/

// extern "C" void abort(void)
// {
// 	while (1)
// 		;
// }

// static void Sys_Init(void)
// {
// 	Clock_Init();
// 	LED_Init();
// 	Uart_Init(115200);

// 	SCB->VTOR = 0x08003000;
// 	SCB->SHCSR = 7 << 16;
// }

// /*========== SYSTEM RELATED FUNCTION END ==========*/

// /*---------- MAIN START ----------*/

// extern "C" void Main()
// {
// 	Sys_Init();
// 	Uart_Printf("Game Example\n\n");

// 	Lcd_Init();
// 	Jog_Poll_Init();
// 	Jog_ISR_Enable(1);
// 	Uart1_RX_Interrupt_Enable(1);

// 	GameManager gameManager;

// 	for (;;)
// 	{
// 		gameManager.initGame();
// 		gameManager.startGame();
// 		while (1)
// 		{
// 			if (Jog_key_in)
// 			{
// 				gameManager.updateGamePerUser();
// 				Jog_key_in = 0;
// 			}
// 			if (TIM4_expired)
// 			{
// 				gameManager.updateGamePerTick();
// 				TIM4_expired = 0;
// 			}
// 			gameManager.isGameOver();
// 		}
// 	}
// }

/*========== MAIN END ==========*/


// //final code

// #include "device_driver.h"

// /*---------- STL related function override START ----------*/
// extern "C" void __cxa_pure_virtual()
// {
// 	Uart1_Printf("Pure virtual function call\n");
// 	while (1)
// 		;
// }

// extern "C" void __cxa_deleted_virtual()
// {
// 	Uart1_Printf("Deleted virtual function call\n");
// 	while (1)
// 		;
// }

// namespace __gnu_cxx
// {
// 	void __verbose_terminate_handler()
// 	{
// 		Uart1_Printf("Unexpected error\n");
// 		while (1)
// 			;
// 	}
// }

// int _write(int file, char *ptr, int len)
// {
// 	int DataIdx;
// 	for (DataIdx = 0; DataIdx < len; DataIdx++)
// 	{
// 		Uart_Send_Byte(*ptr);
// 		ptr++;
// 	}
// 	return len;
// }

// /*========== STL related function override END ==========*/

// /*---------- DEFINE START ----------*/

// #define LCDW (320)
// #define LCDH (240)
// #define X_MIN (0)
// #define X_MAX (LCDW - 1)
// #define Y_MIN (0)
// #define Y_MAX (LCDH - 1)

// #define TIMER_PERIOD (10)
// #define RIGHT (1)
// #define LEFT (-1)

// #define CAR_STEP (5)
// #define CAR_SIZE_X (20)
// #define CAR_SIZE_Y (20)
// #define FROG_STEP (10)
// #define FROG_SIZE_X (20)
// #define FROG_SIZE_Y (20)

// #define BACK_COLOR (6)
// #define FROG_COLOR (0)

// #define SCORE_X (90)
// #define SCORE_Y (50)
// #define SCORE_COLOR (4)
// #define SCORE_SCALE (20)

// #define GAME_OVER (1)

// #define MAX_CAR 5

// /*========== DEFINE END ==========*/

// /*---------- ENUM START ----------*/

// enum BORDER
// {
// 	LEFT_BORDER = 0,
// 	RIGHT_BORDER = 1,
// 	TOP_BORDER = 2,
// 	BOTTOM_BORDER = 3,
// 	NO_BORDER = -1
// };

// enum FROG_GOAL
// {
// 	HOME,
// 	SCHOOL
// };

// enum GAME_STATUS
// {
// 	RUNNING,
// 	GAMEOVER
// };

// /*========== ENUM END ==========*/

// /*---------- CLASS DECLARATION START ----------*/

// class IObject;
// class Square;
// class Frog;
// class Obstacle;
// class DynamicObstacle;
// class Car;
// class ScoreBoard;
// class GameManager;

// /*========== CLASS DECLARATION END ==========*/

// /*---------- FUNCTION DECLARATION START ----------*/

// void lcdDrawChar(int x, int y, int num, unsigned short color, int scale);
// void lcdDrawNumber(int x, int y, int number, unsigned short color, int scale);

// /*========== FUNCTION DECLARATION END ==========*/

// /*---------- GLOBAL VAR START ----------*/

// static unsigned short color[] = {RED, YELLOW, GREEN, BLUE, WHITE, VIOLET, BLACK};
// volatile int i = 0;

// extern volatile int TIM4_expired;
// extern volatile int USART1_rx_ready;
// extern volatile int USART1_rx_data;
// extern volatile int Jog_key_in;
// extern volatile int Jog_key;

// const static unsigned char font[10][5] = {
// 	{0b111, 0b101, 0b101, 0b101, 0b111}, // 0
// 	{0b010, 0b010, 0b010, 0b010, 0b010}, // 1
// 	{0b111, 0b001, 0b111, 0b100, 0b111}, // 2
// 	{0b111, 0b001, 0b111, 0b001, 0b111}, // 3
// 	{0b101, 0b101, 0b111, 0b001, 0b001}, // 4
// 	{0b111, 0b100, 0b111, 0b001, 0b111}, // 5
// 	{0b111, 0b100, 0b111, 0b101, 0b111}, // 6
// 	{0b111, 0b001, 0b001, 0b001, 0b001}, // 7
// 	{0b111, 0b101, 0b111, 0b101, 0b111}, // 8
// 	{0b111, 0b101, 0b111, 0b001, 0b111}	 // 9
// };

// /*========== GLOBAL VAR END ==========*/

// /*---------- CLASS START ----------*/

// // Interface for all objects
// class IObject
// {
// public:
// 	IObject() {}
// 	virtual ~IObject() {}
// 	virtual void draw() = 0;
// 	virtual void move() = 0;
// 	virtual int isSquare() = 0;
// 	virtual int checkObjCollision(IObject *obj) = 0;
// };

// class Square : public IObject
// {
// protected:
// 	int x, y;
// 	int w, h;
// 	int ci;

// public:
// 	Square() {}
// 	Square(int x, int y, int w, int h, int ci) : x(x), y(y), w(w), h(h), ci(ci) {}
// 	virtual ~Square() {}
// 	void draw();
// 	int isSquare();
// 	int checkObjCollision(IObject *obj);
// 	BORDER checkBorderCollision();
// 	virtual void move() = 0;
// };

// class Frog : public Square
// {
// private:
// 	int goal;
// 	int dir;

// public:
// 	Frog() {}
// 	~Frog() {}
// 	Frog(int x, int y, int w, int h, int ci, int goal, int dir) : Square(x, y, w, h, ci), goal(goal), dir(dir) {}
// 	void move();
// 	int checkBorder();
// };

// class Obstacle : public Square
// {
// protected:
// 	int isMoving;
// 	int isGameOver;

// public:
// 	Obstacle() {}
// 	virtual ~Obstacle() {}
// 	Obstacle(int x, int y, int w, int h, int ci) : Square(x, y, w, h, ci), isMoving(0), isGameOver(1) {}
// 	virtual void move() = 0;
// };

// class DynamicObstacle : public Obstacle
// {
// protected:
// 	int dir;


// public:
// 	DynamicObstacle() {}
// 	virtual ~DynamicObstacle() {}
// 	DynamicObstacle(int x, int y, int w, int h, int ci, int dir) : Obstacle(x, y, w, h, ci), dir(dir)
// 	{
// 		this->isMoving = 1;
// 	}
// 	virtual void move() = 0;
// };

// class Car : public DynamicObstacle
// {
// public:
// 	Car() {}
// 	~Car() {}
// 	Car(int x, int y, int w, int h, int ci, int dir) : DynamicObstacle(x, y, w, h, ci, dir) {}
// 	void move();
// };

// class ScoreBoard
// {
// private:
// 	int score;

// public:
// 	ScoreBoard() : score(0) {}
// 	~ScoreBoard() {}
// 	void addScore(int var);
// 	void resetScore();
// 	void drawScoreCenter();
// 	int getScore();
// };

// class GameManager
// {
// private:
// 	Frog frog;
// 	int curCar;
// 	Car cars[MAX_CAR];
// 	ScoreBoard scoreBoard;
// 	int gameStatus;

// public:
// 	GameManager() {}
// 	~GameManager() {}
// 	void initGame();
// 	void startGame();
// 	void updateGamePerTick();
// 	void updateGamePerUser();
// 	void endGame();
// 	void isGameOver();
// 	void addCar();
// };

// /*========== CLASS END ==========*/

// /*---------- CLASS FUNCTION DEFINITION START ----------*/

// // draw a square
// void Square::draw()
// {
// 	Lcd_Draw_Box(x, y, w, h, color[ci]);
// }

// // return 1 if the object is a square
// int Square::isSquare()
// {
// 	return 1;
// }

// // if two squares collide, return 1
// // if parameter obj is not a square, return -1
// int Square::checkObjCollision(IObject *obj)
// {
// 	if (obj->isSquare() == 0)
// 		return -1;
// 	Square *temp = (Square *)obj;
// 	if (x > temp->x + temp->w || temp->x > x + w)
// 		return 0;
// 	if (y > temp->y + temp->h || temp->y > y + h)
// 		return 0;
// 	return 1;
// }

// // if the square collides with the border, return BORDER
// // otherwise, return NO_BORDER
// BORDER Square::checkBorderCollision()
// {
// 	if (this->x <= X_MIN)
// 		return LEFT_BORDER;
// 	if (this->x + this->w >= X_MAX)
// 		return RIGHT_BORDER;
// 	if (this->y <= Y_MIN)
// 		return TOP_BORDER;
// 	if (this->y + this->h >= Y_MAX)
// 		return BOTTOM_BORDER;
// 	return NO_BORDER;
// }

// // move the frog according to the Jog_key
// void Frog::move()
// {
// 	int temp = this->ci;
// 	this->ci = BACK_COLOR;
// 	this->draw();
// 	switch (Jog_key)
// 	{
// 	case 0:
// 		if (this->y > Y_MIN)
// 			this->y -= FROG_STEP;
// 		break;
// 	case 1:
// 		if (this->y + this->h < Y_MAX)
// 			this->y += FROG_STEP;
// 		break;
// 	case 2:
// 		if (this->x > X_MIN)
// 			this->x -= FROG_STEP;
// 		break;
// 	case 3:
// 		if (this->x + this->w < X_MAX)
// 			this->x += FROG_STEP;
// 		break;
// 	}
// 	this->ci = temp;
// 	this->draw();
// }

// // check the border and change the goal if necessary
// // return 1 if the frog gains a score
// int Frog::checkBorder()
// {
// 	static int collidedBorder = NO_BORDER;
// 	collidedBorder = this->checkBorderCollision();
// 	switch (collidedBorder)
// 	{
// 	// Nothing happens if the frog collides with the left or right border
// 	case NO_BORDER:
// 	case LEFT_BORDER:
// 	case RIGHT_BORDER:
// 		break;
// 	// If the frog collides with the top border and goal is SCHOOL, change the goal to HOME
// 	case TOP_BORDER:
// 		if (this->goal == SCHOOL)
// 		{
// 			Uart1_Printf("SCHOOL\n");
// 			this->goal = HOME;
// 		}
// 		break;
// 	// If the frog collides with the bottom border and goal is HOME, add a car and increase the score
// 	case BOTTOM_BORDER:
// 		if (this->goal == HOME)
// 		{
// 			Uart1_Printf("HOME\n");
// 			this->goal = SCHOOL;
// 			return 1;
// 		}
// 		break;
// 	}
// 	return 0;
// }

// // move the car
// void Car::move()
// {
// 	int temp = this->ci;
// 	this->ci = BACK_COLOR;
// 	this->draw();
// 	this->x += CAR_STEP * this->dir;
// 	// check border and change the direction if necessary
// 	if (this->checkBorderCollision() != NO_BORDER)
// 		this->dir = -this->dir;
// 	this->ci = temp;
// 	this->draw();
// }

// // add score
// void ScoreBoard::addScore(int var)
// {
// 	this->score += var;
// 	if (this->score > 99)
// 		this->score = 99;
// }

// // reset score
// void ScoreBoard::resetScore()
// {
// 	this->score = 0;
// }

// // draw score
// void ScoreBoard::drawScoreCenter()
// {
// 	lcdDrawNumber(SCORE_X, SCORE_Y, this->score, color[SCORE_COLOR], SCORE_SCALE);
// }

// // get score
// int ScoreBoard::getScore()
// {
// 	return this->score;
// }

// // initialize the game
// void GameManager::initGame()
// {
// 	Lcd_Clr_Screen();
// 	this->scoreBoard.resetScore();
// 	this->curCar = 0;
// 	this->frog = Frog(150, 220, FROG_SIZE_X, FROG_SIZE_Y, FROG_COLOR, SCHOOL, 0);
// 	// this->frog = Frog(150, 220, FROG_SIZE_X, FROG_SIZE_Y, FROG_COLOR, SCHOOL);
// 	this->frog.draw();
// 	this->addCar();
// }

// // start the game
// void GameManager::startGame()
// {
// 	this->gameStatus = RUNNING;
// 	TIM4_Repeat_Interrupt_Enable(1, TIMER_PERIOD * 10);
// }

// // update the game
// void GameManager::updateGamePerTick()
// {
// 	for (i = 0; i < this->curCar; i++)
// 	{
// 		this->cars[i].move();
// 	}
// 	// check collision with cars
// 	for (i = 0; i < this->curCar; i++)
// 	{
// 		if (this->frog.checkObjCollision(&this->cars[i]) == 1)
// 		{
// 			this->gameStatus = GAMEOVER;
// 			break;
// 		}
// 	}
// }

// // update the game
// void GameManager::updateGamePerUser()
// {
// 	this->frog.move();
// 	if (this->frog.checkBorder())
// 	{
// 		this->scoreBoard.addScore(1);
// 		this->addCar();
// 	}
// }

// // end the game
// void GameManager::endGame()
// {
// 	TIM4_Repeat_Interrupt_Enable(0, 0);
// 	Uart_Printf("Game Over, your score is %d.\nPress Push Button to continue.\n", this->scoreBoard.getScore());
// 	Lcd_Clr_Screen();
// 	this->scoreBoard.drawScoreCenter();
// 	while (1)
// 	{
// 		if (Jog_key == 4 || Jog_key == 5)
// 		{
// 			this->initGame();
// 			this->startGame();
// 			Jog_key_in = 0;
// 			break;
// 		}
// 	}
// }

// // check if the game is over
// void GameManager::isGameOver()
// {
// 	if (this->gameStatus == GAMEOVER)
// 	{
// 		this->endGame();
// 	}
// }

// // add a car
// void GameManager::addCar()
// {
// 	if (this->curCar >= MAX_CAR)
// 		return;
// 	this->curCar++;
// 	this->cars[this->curCar - 1] = Car(10, 40 + 30 * curCar, CAR_SIZE_X, CAR_SIZE_Y, this->curCar, RIGHT);
// 	this->cars[this->curCar - 1].draw();
// }

// /*========== CLASS FUNCTION DEFINITION END ==========*/

// /*---------- LCD RELATED FUNCTION START ----------*/

// void lcdDrawChar(int x, int y, int num, unsigned short color, int scale)
// {
// 	if (num < 0 || num > 9)
// 		return;

// 	const unsigned char *bitmap = font[num];
// 	for (int row = 0; row < 5; row++)
// 	{
// 		for (int col = 0; col < 3; col++)
// 		{
// 			if (bitmap[row] & (1 << (2 - col)))
// 			{
// 				Lcd_Draw_Box(x + col * scale, y + row * scale, scale, scale, color);
// 			}
// 		}
// 	}
// }

// void lcdDrawNumber(int x, int y, int number, unsigned short color, int scale)
// {
// 	if (number < 0 || number > 99)
// 		return;

// 	lcdDrawChar(x, y, number / 10, color, scale);
// 	// Char width + spaceing added to x
// 	lcdDrawChar(x + 3 * scale + scale, y, number % 10, color, scale);
// }

// /*========== LCD RELATED FUNCTION END ==========*/

// /*---------- SYSTEM RELATED FUNCTION START ----------*/

// extern "C" void abort(void)
// {
// 	while (1)
// 		;
// }

// static void Sys_Init(void)
// {
// 	Clock_Init();
// 	LED_Init();
// 	Uart_Init(115200);

// 	SCB->VTOR = 0x08003000;
// 	SCB->SHCSR = 7 << 16;
// }

// /*========== SYSTEM RELATED FUNCTION END ==========*/

// /*---------- MAIN START ----------*/

// extern "C" void Main()
// {
// 	Sys_Init();
// 	Uart_Printf("Game Example\n\n");

// 	Lcd_Init();
// 	Jog_Poll_Init();
// 	Jog_ISR_Enable(1);
// 	Uart1_RX_Interrupt_Enable(1);

// 	GameManager gameManager;

// 	for (;;)
// 	{
// 		gameManager.initGame();
// 		gameManager.startGame();
// 		while (1)
// 		{
// 			if (Jog_key_in)
// 			{
// 				gameManager.updateGamePerUser();
// 				Jog_key_in = 0;
// 			}
// 			if (TIM4_expired)
// 			{
// 				gameManager.updateGamePerTick();
// 				TIM4_expired = 0;
// 			}
// 			gameManager.isGameOver();
// 		}
// 	}
// }

/*========== MAIN END ==========*/

/*---------- CLASS FUNCTION DEFINITION END ----------*/

/*---------- REMAINING FUNCTIONS OMITTED FOR BREVITY ----------*/

///////////////////////////////////////////////////////////////////////////////////////////////////////
// //base code
// #include "device_driver.h"

// /*---------- DEFINE START ----------*/

// #define LCDW (320)
// #define LCDH (240)
// #define X_MIN (0)
// #define X_MAX (LCDW - 1)
// #define Y_MIN (0)
// #define Y_MAX (LCDH - 1)

// #define TIMER_PERIOD (10)
// #define RIGHT (1)
// #define LEFT (-1)

// #define CAR_STEP (5)
// #define CAR_SIZE_X (20)
// #define CAR_SIZE_Y (20)
// #define FROG_STEP (10)
// #define FROG_SIZE_X (20)
// #define FROG_SIZE_Y (20)

// #define BACK_COLOR (6)
// #define FROG_COLOR (0)

// #define GAME_OVER (1)

// #define MAX_CAR 5

// /*---------- DEFINE END ----------*/

// /*---------- ENUM START ----------*/

// enum BORDER
// {
// 	LEFT_BORDER = 0,
// 	RIGHT_BORDER = 1,
// 	TOP_BORDER = 2,
// 	BOTTOM_BORDER = 3,
// 	NO_BORDER = -1
// };

// enum FROG_GOAL
// {
// 	HOME,
// 	SCHOOL
// };

// /*---------- ENUM END ----------*/

// /*---------- CLASS DECLARATION START ----------*/

// class square;
// class frog;
// class car;

// /*---------- CLASS DECLARATION END ----------*/

// /*---------- FUNCTION DECLARATION START ----------*/

// void lcdDrawChar(int x, int y, int num, unsigned short color, int scale);
// void lcdDrawNumber(int x, int y, int number, unsigned short color, int scale);
// int checkCollisionWithCars(frog *FUCKINGFROG, car *cars, int curCar);
// void addCar();
// static void Game_Init();
// static int jogKeyHandler();
// static int timerHandler();
// static void Game_Over();

// /*---------- FUNCTION DECLARATION END ----------*/

// /*---------- GLOBAL VAR START ----------*/

// static int score;
// static unsigned short color[] = {RED, YELLOW, GREEN, BLUE, WHITE, VIOLET, BLACK};
// static int curCar = 1;
// volatile int i = 0;

// extern volatile int TIM4_expired;
// extern volatile int USART1_rx_ready;
// extern volatile int USART1_rx_data;
// extern volatile int Jog_key_in;
// extern volatile int Jog_key;

// const unsigned char font[10][5] = {
// 	{0b111, 0b101, 0b101, 0b101, 0b111}, // 0
// 	{0b010, 0b010, 0b010, 0b010, 0b010}, // 1
// 	{0b111, 0b001, 0b111, 0b100, 0b111}, // 2
// 	{0b111, 0b001, 0b111, 0b001, 0b111}, // 3
// 	{0b101, 0b101, 0b111, 0b001, 0b001}, // 4
// 	{0b111, 0b100, 0b111, 0b001, 0b111}, // 5
// 	{0b111, 0b100, 0b111, 0b101, 0b111}, // 6
// 	{0b111, 0b001, 0b001, 0b001, 0b001}, // 7
// 	{0b111, 0b101, 0b111, 0b101, 0b111}, // 8
// 	{0b111, 0b101, 0b111, 0b001, 0b111}	 // 9
// };

// /*---------- GLOBAL VAR END ----------*/

// /*---------- CLASS START ----------*/

// class square
// {
// protected:
// 	int x, y;
// 	int w, h;
// 	int ci;
// 	int dir;

// public:
// 	square() {}
// 	void drawSquare();
// 	int checkCollision(square *obj);
// 	BORDER checkCollisionBorder();
// };

// class frog : public square
// {
// private:
// 	int goal;

// public:
// 	frog() {}
// 	frog(int x, int y, int w, int h, int ci, int dir)
// 	{
// 		this->goal = SCHOOL;
// 		this->x = x;
// 		this->y = y;
// 		this->w = w;
// 		this->h = h;
// 		this->ci = ci;
// 		this->dir = dir;
// 	}
// 	void moveFrog(int Jog_key);
// 	void checkBorder();
// };

// class car : public square
// {
// public:
// 	car() {}
// 	car(int x, int y, int w, int h, int ci, int dir)
// 	{
// 		this->x = x;
// 		this->y = y;
// 		this->w = w;
// 		this->h = h;
// 		this->ci = ci;
// 		this->dir = dir;
// 	}
// 	void moveCar();
// };

// // This should be here because it uses the class definition
// frog FUCKINGFROG;
// car cars[MAX_CAR];

// /*---------- CLASS END ----------*/

// /*---------- CLASS FUNCTION DEFINITION START ----------*/

// // draw a square
// void square::drawSquare()
// {
// 	Lcd_Draw_Box(x, y, w, h, color[ci]);
// }

// // if two squares collide, return 1
// int square::checkCollision(square *obj)
// {
// 	if (x > obj->x + obj->w || obj->x > x + w)
// 		return 0;
// 	if (y > obj->y + obj->h || obj->y > y + h)
// 		return 0;
// 	return 1;
// }

// // if the square collides with the border, return BORDER
// // otherwise, return -1
// BORDER square::checkCollisionBorder()
// {
// 	if (this->x <= X_MIN)
// 		return LEFT_BORDER;
// 	if (this->x + this->w >= X_MAX)
// 		return RIGHT_BORDER;
// 	if (this->y <= Y_MIN)
// 		return TOP_BORDER;
// 	if (this->y + this->h >= Y_MAX)
// 		return BOTTOM_BORDER;
// 	return NO_BORDER;
// }

// // move the frog according to the Jog_key
// void frog::moveFrog(int Jog_key)
// {
// 	int temp = this->ci;
// 	this->ci = BACK_COLOR;
// 	this->drawSquare();
// 	switch (Jog_key)
// 	{
// 	case 0:
// 		if (this->y > Y_MIN)
// 			this->y -= FROG_STEP;
// 		break;
// 	case 1:
// 		if (this->y + this->h < Y_MAX)
// 			this->y += FROG_STEP;
// 		break;
// 	case 2:
// 		if (this->x > X_MIN)
// 			this->x -= FROG_STEP;
// 		break;
// 	case 3:
// 		if (this->x + this->w < X_MAX)
// 			this->x += FROG_STEP;
// 		break;
// 	}
// 	this->ci = temp;
// 	this->drawSquare();
// }

// // check the border and change the goal if necessary
// void frog::checkBorder()
// {
// 	static int collidedBorder = NO_BORDER;
// 	collidedBorder = this->checkCollisionBorder();
// 	switch (collidedBorder)
// 	{
// 	// Nothing happens if the frog collides with the left or right border
// 	case NO_BORDER:
// 	case LEFT_BORDER:
// 	case RIGHT_BORDER:
// 		break;
// 	// If the frog collides with the top border and goal is SCHOOL, change the goal to HOME
// 	case TOP_BORDER:
// 		if (this->goal == SCHOOL)
// 		{
// 			Uart1_Printf("SCHOOL\n");
// 			this->goal = HOME;
// 		}
// 		break;
// 	// If the frog collides with the bottom border and goal is HOME, add a car and increase the score
// 	case BOTTOM_BORDER:
// 		if (this->goal == HOME)
// 		{
// 			addCar();
// 			score++;
// 			Uart1_Printf("HOME, SCORE: %d\n", score);
// 			this->goal = SCHOOL;
// 		}
// 		break;
// 	}
// }

// // move the car
// void car::moveCar()
// {
// 	int temp = this->ci;
// 	this->ci = BACK_COLOR;
// 	this->drawSquare();
// 	this->x += CAR_STEP * this->dir;
// 	if ((this->x + this->w >= X_MAX) || (this->x <= X_MIN))
// 		this->dir = -this->dir;
// 	this->ci = temp;
// 	this->drawSquare();
// }

// /*---------- CLASS FUNCTION DEFINITION END ----------*/

// /*---------- SYSTEM RELATED FUNCTION START ----------*/

// extern "C" void abort(void)
// {
// 	while (1)
// 		;
// }

// static void Sys_Init(void)
// {
// 	Clock_Init();
// 	LED_Init();
// 	Uart_Init(115200);

// 	SCB->VTOR = 0x08003000;
// 	SCB->SHCSR = 7 << 16;
// }

// /*---------- SYSTEM RELATED FUNCTION END ----------*/

// /*---------- LCD RELATED FUNCTION START ----------*/

// void lcdDrawChar(int x, int y, int num, unsigned short color, int scale)
// {
// 	if (num < 0 || num > 9)
// 		return;

// 	const unsigned char *bitmap = font[num];
// 	for (int row = 0; row < 5; row++)
// 	{
// 		for (int col = 0; col < 3; col++)
// 		{
// 			if (bitmap[row] & (1 << (2 - col)))
// 			{
// 				Lcd_Draw_Box(x + col * scale, y + row * scale, scale, scale, color);
// 			}
// 		}
// 	}
// }

// void lcdDrawNumber(int x, int y, int number, unsigned short color, int scale)
// {
// 	if (number < 0 || number > 99)
// 		return;

// 	lcdDrawChar(x, y, number / 10, color, scale);
// 	// Char width + spaceing added to x
// 	lcdDrawChar(x + 3 * scale + scale, y, number % 10, color, scale);
// }

// /*---------- LCD RELATED FUNCTION END ----------*/

// /*---------- GAME RELATED FUNCTION START ----------*/

// // check collision with cars
// int checkCollisionWithCars(frog *FUCKINGFROG, car cars[], int curCar)
// {
// 	for (int i = 0; i < curCar; i++)
// 		if (FUCKINGFROG->checkCollision(&cars[i]))
// 			return 1;
// 	return 0;
// }

// // add a car
// void addCar()
// {
// 	if (curCar < MAX_CAR)
// 	{
// 		cars[curCar] = car(10, 40 + 30 * curCar, CAR_SIZE_X, CAR_SIZE_Y, curCar + 1, RIGHT);
// 		cars[curCar].drawSquare();
// 		curCar++;
// 	}
// }

// // initialize the game
// static void Game_Init()
// {
// 	score = 0;
// 	curCar = 0;
// 	Lcd_Clr_Screen();
// 	FUCKINGFROG = frog(150, 220, FROG_SIZE_X, FROG_SIZE_Y, FROG_COLOR, SCHOOL);
// 	FUCKINGFROG.drawSquare();
// 	addCar();
// 	Uart_Printf("Game Start\n");
// }

// // return -1 if game over
// // else return 0
// static int jogKeyHandler()
// {
// 	FUCKINGFROG.moveFrog(Jog_key);
// 	// check collision with cars
// 	if (checkCollisionWithCars(&FUCKINGFROG, cars, curCar))
// 	{
// 		return GAME_OVER;
// 	}
// 	// check border
// 	FUCKINGFROG.checkBorder();
// 	Jog_key_in = 0;
// 	return 0;
// }

// // return -1 if game over
// // else return 0
// static int timerHandler()
// {
// 	for (i = 0; i < curCar; i++)
// 	{
// 		cars[i].moveCar();
// 	}
// 	// check collision with frog
// 	if (checkCollisionWithCars(&FUCKINGFROG, cars, curCar))
// 	{
// 		return GAME_OVER;
// 	}
// 	TIM4_expired = 0;
// 	return 0;
// }

// // game is over game is over
// static void Game_Over()
// {
// 	TIM4_Repeat_Interrupt_Enable(0, 0);
// 	Uart_Printf("Game Over, your score is %d.\nPress Push Button to continue.\n", score);
// 	Lcd_Clr_Screen();
// 	lcdDrawNumber(80, 50, score % 100, WHITE, 20);
// 	static int k = 0;
// 	while(1)
// 	{
// 		if (Jog_key == 4 || Jog_key == 5)
// 		{
// 			Jog_key_in = 0;
// 			break;
// 		}
// 	}
// }

// /*---------- GAME RELATED FUNCTION END ----------*/

// /*---------- MAIN START ----------*/

// extern "C" void Main()
// {
// 	Sys_Init();
// 	Uart_Printf("Game Example\n\n");

// 	Lcd_Init();
// 	Jog_Poll_Init();
// 	Jog_ISR_Enable(1);
// 	Uart1_RX_Interrupt_Enable(1);

// 	for (;;)
// 	{
// 		Game_Init();
// 		TIM4_Repeat_Interrupt_Enable(1, TIMER_PERIOD * 10);
// 		int gameOver = 0;

// 		for (;;)
// 		{
// 			// jog key input
// 			if (Jog_key_in)
// 			{
// 				gameOver = jogKeyHandler();
// 			}
// 			// timer interrupt
// 			if (TIM4_expired)
// 			{
// 				gameOver = timerHandler();
// 			}
// 			// game over
// 			if (gameOver)
// 			{
// 				Game_Over();
// 				break;
// 			}
// 		}
// 	}
// }

// /*---------- MAIN END ----------*/

// //////////////////////////////////////////////////////////////////////////////////////////



// //////////lims 
// #include "device_driver.h"

// #define LCDW (320)
// #define LCDH (240)
// #define X_MIN (0)
// #define X_MAX (LCDW - 1)
// #define Y_MIN (0)
// #define Y_MAX (LCDH - 1)

// #define TIMER_PERIOD (10)
// #define RIGHT (1)
// #define LEFT (-1)
// #define HOME (0)
// #define SCHOOL (1)

// #define CAR_STEP (10)
// #define CAR_SIZE_X (20)
// #define CAR_SIZE_Y (20)
// #define FROG_STEP (10)
// #define FROG_SIZE_X (20)
// #define FROG_SIZE_Y (20)

// #define BACK_COLOR (6)
// #define CAR_COLOR (1)
// #define FROG_COLOR (0)

// #define GAME_OVER (1)

// #define MAX_CAR 5
// static void Game_Init();

// static int score;
// static unsigned short color[] = {RED, YELLOW, GREEN, BLUE, WHITE, VIOLET,BLACK};
// static int curCar=1;
// volatile int i = 0;

// extern volatile int TIM4_expired;
// extern volatile int USART1_rx_ready;
// extern volatile int USART1_rx_data;
// extern volatile int Jog_key_in;
// extern volatile int Jog_key;

// class square
// {
// public:
// 	int x, y;
// 	int w, h;
// 	int ci;
// 	int dir;

// public:
// 	square() {}
// };

// void Draw_Object(square *obj)
// {
// 	Lcd_Draw_Box(obj->x, obj->y, obj->w, obj->h, color[obj->ci]);
// }

// class frog : public square
// {
// public:
// 	frog() {}
// 	frog(int x, int y, int w, int h, int ci, int dir)
// 	{
// 		this->x = x;
// 		this->y = y;
// 		this->w = w;
// 		this->h = h;
// 		this->ci = ci;
// 		this->dir = dir;
// 	}
// 	int move(int k);
// };

// class car : public square
// {
// public:
// 	car() {}
// 	car(int x, int y, int w, int h, int ci, int dir)
// 	{
// 		this->x = x;
// 		this->y = y;
// 		this->w = w;
// 		this->h = h;
// 		this->ci = ci;
// 		this->dir = dir;
// 	}
// 	int move(frog *myFROG);
// };

// frog myFROG;
// car cars[MAX_CAR];

// int checkCollisionSingleCar(frog *myFROG, car myCAR)
// {
// 	int col = 0;

// 	if ((myCAR.x >= myFROG->x) && ((myFROG->x + FROG_STEP) >= myCAR.x))
// 		col |= 1 << 0;
// 	else if ((myCAR.x < myFROG->x) && ((myCAR.x + CAR_STEP) >= myFROG->x))
// 		col |= 1 << 0;

// 	if ((myCAR.y >= myFROG->y) && ((myFROG->y + FROG_STEP) >= myCAR.y))
// 		col |= 1 << 1;
// 	else if ((myCAR.y < myFROG->y) && ((myCAR.y + CAR_STEP) >= myFROG->y))
// 		col |= 1 << 1;

// 	if (col == 3)
// 	{
// 		Uart_Printf("SCORE = %d\n", score);
// 		return GAME_OVER;
// 	}

// 	return 0;
// }

// int checkCollisionAllCar(frog *myFROG, car cars[], int size)
// {
// 	int col = 0;

// 	for (int i = 0; i < size; i++)
// 	{
// 		col |= checkCollisionSingleCar(myFROG, cars[i]);
// 	}

// 	if (col == 3)
// 	{
// 		Uart_Printf("SCORE = %d\n", score);
// 		return GAME_OVER;
// 	}

// 	if ((myFROG->dir == SCHOOL) && (myFROG->y == Y_MIN))
// 	{
// 		Uart_Printf("SCHOOL\n");
// 		myFROG->dir = HOME;
// 	}



// 	if ((myFROG->dir == HOME) && (myFROG->y == LCDH - myFROG->h))
// 	{
// 		myFROG->dir = SCHOOL;
		
// 		score++;
// 		curCar++;
// 		Game_Init();
	
		
		
// 		Uart_Printf("HOME, %d\n", score);
// 		Uart_Printf("Car, %d\n", curCar);
// 	}

// 	return 0;
// }

// int car::move(frog *myFROG)
// {
// 	this->x += ((CAR_STEP*(curCar+1))/2) * this->dir;
// 	if ((this->x + this->w >= X_MAX) || (this->x <= X_MIN))
// 		this->dir = -this->dir;
// 	return checkCollisionSingleCar(myFROG, *this);
// }

// int frog::move(int k)
// {
// 	switch (k)
// 	{
// 	case 0:
// 		if (this->y > Y_MIN)
// 			this->y -= FROG_STEP;
// 		break;
// 	case 1:
// 		if (this->y + this->h < Y_MAX)
// 			this->y += FROG_STEP;
// 		break;
// 	case 2:
// 		if (this->x > X_MIN)
// 			this->x -= FROG_STEP;
// 		break;
// 	case 3:
// 		if (this->x + this->w < X_MAX)
// 			this->x += FROG_STEP;
// 		break;
// 	}
// 	return checkCollisionAllCar(this, cars, curCar);
// }

// static void Game_Init()
// {
// 	score = 0;
// 	Lcd_Clr_Screen();
// 	myFROG = frog(150, 220, FROG_SIZE_X, FROG_SIZE_Y, FROG_COLOR, SCHOOL);
// 	Draw_Object(&myFROG);
// 	// cars[0] = car(10, 100, CAR_SIZE_X, CAR_SIZE_Y, BLUE, RIGHT);
//     // cars[1] = car(100, 150, CAR_SIZE_X, CAR_SIZE_Y, GREEN, LEFT);
//     // cars[2] = car(200, 50, CAR_SIZE_X, CAR_SIZE_Y, BLUE, RIGHT);
// 	// Draw_Object(&cars[0]);
// 	// Draw_Object(&cars[1]);
// 	// Draw_Object(&cars[2]);


// 	for (i = 0; i < curCar; i++)
// 	{
// 		cars[i] = car((10 + i * 100)%310, 20+(40*i), CAR_SIZE_X, CAR_SIZE_Y, (CAR_COLOR+i), RIGHT);
// 		Draw_Object(&cars[i]);
// 	}
// }

// extern "C" void abort(void)
// {
// 	while (1)
// 		;
// }

// static void Sys_Init(void)
// {
// 	Clock_Init();
// 	LED_Init();
// 	Uart_Init(115200);

// 	SCB->VTOR = 0x08003000;
// 	SCB->SHCSR = 7 << 16;
// }

// extern "C" void Main()
// {
// 	Sys_Init();
// 	Uart_Printf("Game Example\n");

// 	Lcd_Init();
// 	Jog_Poll_Init();
// 	Jog_ISR_Enable(1);
// 	Uart1_RX_Interrupt_Enable(1);

// 	for (;;)
// 	{
// 		Game_Init();
// 		TIM4_Repeat_Interrupt_Enable(1, TIMER_PERIOD * 10);

// 		for (;;)
// 		{
// 			int game_over = 0;

// 			if (Jog_key_in)
// 			{
// 				Uart1_Printf("KEY\n");
// 				myFROG.ci = BACK_COLOR;
// 				Draw_Object(&myFROG);
// 				game_over = myFROG.move(Jog_key);
// 				myFROG.ci = FROG_COLOR;
// 				Draw_Object(&myFROG);
// 				Jog_key_in = 0;
// 			}

// 			if (TIM4_expired)
// 			{
// 				for (i = 0; i < curCar; i++)
// 				{
// 					cars[i].ci = BACK_COLOR;
// 					Draw_Object(&cars[i]);
// 					if (cars[i].move(&myFROG) == GAME_OVER)
// 					{
// 						game_over = 1;
// 						break;
// 					}
// 					cars[i].ci = (CAR_COLOR+i);
// 					Draw_Object(&cars[i]);
					
					
// 				}
// 				TIM4_expired = 0;
// 			}

// 			if (game_over)
// 			{
// 				TIM4_Repeat_Interrupt_Enable(0, 0);
// 				Uart_Printf("Game Over, Please press any key to continue.\n");
// 				Jog_Wait_Key_Pressed();
// 				Jog_Wait_Key_Released();
// 				Uart_Printf("Game Start\n");
// 				break;
// 			}
// 		}
// 	}
// }



///////before class
// #include "device_driver.h"

// #define LCDW			(320)
// #define LCDH			(240)
// #define X_MIN	 		(0)
// #define X_MAX	 		(LCDW - 1)
// #define Y_MIN	 		(0)
// #define Y_MAX	 		(LCDH - 1)

// #define TIMER_PERIOD	(10)
// #define RIGHT			(1)
// #define LEFT			(-1)
// #define HOME			(0)
// #define SCHOOL			(1)

// #define CAR_STEP		(10)
// #define CAR_SIZE_X		(20)
// #define CAR_SIZE_Y		(20)
// #define FROG_STEP		(10)
// #define FROG_SIZE_X		(20)
// #define FROG_SIZE_Y		(20)

// #define BACK_COLOR		(5)
// #define CAR_COLOR		(0)
// #define FROG_COLOR		(1)

// #define GAME_OVER		(1)

// typedef struct
// {
// 	int x,y;
// 	int w,h;
// 	int ci;
// 	int dir;
// }QUERY_DRAW;

//static QUERY_DRAW car;
//static QUERY_DRAW frog;

// static int score;
// static unsigned short color[] = {RED, YELLOW, GREEN, BLUE, WHITE, BLACK};

// static int Check_Collision(void)
// {
// 	int col = 0;

// 	if((car.x >= frog.x) && ((frog.x + FROG_STEP) >= car.x)) col |= 1<<0;
// 	else if((car.x < frog.x) && ((car.x + CAR_STEP) >= frog.x)) col |= 1<<0;
	
// 	if((car.y >= frog.y) && ((frog.y + FROG_STEP) >= car.y)) col |= 1<<1;
// 	else if((car.y < frog.y) && ((car.y + CAR_STEP) >= frog.y)) col |= 1<<1;

// 	if(col == 3)
// 	{
// 		Uart_Printf("SCORE = %d\n", score);	
// 		return GAME_OVER;
// 	}

// 	if((frog.dir == SCHOOL) && (frog.y == Y_MIN)) 
// 	{
// 		Uart_Printf("SCHOOL\n");		
// 		frog.dir = HOME;
// 	}

// 	if((frog.dir == HOME) && (frog.y == LCDH - frog.h))
// 	{
// 		frog.dir = SCHOOL;
// 		score++;
// 		Uart_Printf("HOME, %d\n", score);
// 	}

// 	return 0;
// }

// static int Car_Move(void)
// {
// 	car.x += CAR_STEP * car.dir;
// 	if((car.x + car.w >= X_MAX) || (car.x <= X_MIN)) car.dir = -car.dir;
// 	return Check_Collision();
// }

// static void k0(void)
// {
// 	if(frog.y > Y_MIN) frog.y -= FROG_STEP;
// }

// static void k1(void)
// {
// 	if(frog.y + frog.h < Y_MAX) frog.y += FROG_STEP;
// }

// static void k2(void)
// {
// 	if(frog.x > X_MIN) frog.x -= FROG_STEP;
// }

// static void k3(void)
// {
// 	if(frog.x + frog.w < X_MAX) frog.x += FROG_STEP;
// }

// static int Frog_Move(int k)
// {
// 	// UP(0), DOWN(1), LEFT(2), RIGHT(3)
// 	static void (*key_func[])(void) = {k0, k1, k2, k3};
// 	if(k <= 3) key_func[k]();
// 	return Check_Collision();
// }

// static void Game_Init(void)
// {
// 	score = 0;
// 	Lcd_Clr_Screen();
// 	frog.x = 150; frog.y = 220; frog.w = FROG_SIZE_X; frog.h = FROG_SIZE_Y; frog.ci = FROG_COLOR; frog.dir = SCHOOL;
// 	car.x = 0; car.y = 110; car.w = CAR_SIZE_X; car.h = CAR_SIZE_Y; car.ci = CAR_COLOR; car.dir = RIGHT;
// 	Lcd_Draw_Box(frog.x, frog.y, frog.w, frog.h, color[frog.ci]);
// 	Lcd_Draw_Box(car.x, car.y, car.w, car.h, color[car.ci]);
// }

// static void Draw_Object(QUERY_DRAW * obj)
// {
// 	Lcd_Draw_Box(obj->x, obj->y, obj->w, obj->h, color[obj->ci]);
// }

// extern volatile int TIM4_expired;
// extern volatile int USART1_rx_ready;
// extern volatile int USART1_rx_data;
// extern volatile int Jog_key_in;
// extern volatile int Jog_key;

// extern "C" void abort(void)
// {
//   while (1);
// }

// static void Sys_Init(void)
// {
// 	Clock_Init();
// 	LED_Init();
// 	Uart_Init(115200);

// 	SCB->VTOR = 0x08003000;
// 	SCB->SHCSR = 7<<16;	
// }

// extern "C" void Main()
// {
// 	Sys_Init();
// 	Uart_Printf("Game Example\n");

// 	Lcd_Init();
// 	Jog_Poll_Init();
// 	Jog_ISR_Enable(1);
// 	Uart1_RX_Interrupt_Enable(1);

// 	for(;;)
// 	{
// 		Game_Init();
// 		TIM4_Repeat_Interrupt_Enable(1, TIMER_PERIOD*10);

// 		for(;;)
// 		{
// 			int game_over = 0;

// 			if(Jog_key_in) 
// 			{
// 				Uart_Printf("KEY\n");
// 				frog.ci = BACK_COLOR;
// 				Draw_Object(&frog);
// 				game_over = Frog_Move(Jog_key);
// 				frog.ci = FROG_COLOR;
// 				Draw_Object(&frog);
// 				Jog_key_in = 0;				
// 			}

// 			if(TIM4_expired) 
// 			{
// 				car.ci = BACK_COLOR;
// 				Draw_Object(&car);
// 				game_over = Car_Move();
// 				car.ci = CAR_COLOR;
// 				Draw_Object(&car);
// 				TIM4_expired = 0;
// 			}

// 			if(game_over)
// 			{
// 				TIM4_Repeat_Interrupt_Enable(0, 0);
// 				Uart_Printf("Game Over, Please press any key to continue.\n");
// 				Jog_Wait_Key_Pressed();
// 				Jog_Wait_Key_Released();
// 				Uart_Printf("Game Start\n");
// 				break;
// 			}
// 		}
// 	}
// }
