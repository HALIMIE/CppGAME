#include "device_driver.h"
#include "graphics.h"
#include <stdlib.h>

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

#define BASE (500) // msec

#define MAX_CAR (5)
#define MAX_CAR_STEP (10)
#define MIN_CAR_STEP (3)
#define CAR_SIZE_X (20)
#define CAR_SIZE_Y (20)

#define PLAYER_INIT_X (150)
#define PLAYER_INIT_Y (220)
#define PLAYER_SIZE_X (20)
#define PLAYER_SIZE_Y (20)
#define PLAYER_STEP (10)
#define PLAYER_COLOR (0)

#define MAX_POOP (10)
#define POOP_SIZE_X (20)
#define POOP_SIZE_Y (20)
#define POOP_STEP (1)
#define MAX_POOP_STEP (20)
#define MIN_POOP_STEP (1)
#define INVALID_POOP_Y (-30)
#define POOP_COLOR (3)
#define POOP_SPEED_INCREASE (1)
#define POOP_INIT_X (0)

#define INITIAL_LIFE (3)
#define INITIAL_SCORE (0)

#define BACK_COLOR (6)

#define SCORE_COLOR (4)
#define SCORE_SCALE (20)

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

static void buzzerBeep(unsigned char tone, int duration);
void ScoreSound(int index1, int index2);
void lcdDrawChar(int x, int y, int num, short colorIndex, int scale);
void lcdDrawNumber(int x, int y, int number, short colorIndex, int scale);
void lcdDrawString(int x, int y, const char *string, short colorIndex, int scale);
void setRandomSeed();
uint32_t getADCNoise();
inline void setSeedADC();
inline int max(int a, int b);
inline int min(int a, int b);
inline int randInt(int min, int max);
inline void drawRectangle(int x, int y, int sizeX, int sizeY, short colorIndex);
inline void drawCircle(int x, int y, int sizeX, int sizeY, short colorIndex);
inline void drawInvalid(int x, int y, int sizeX, int sizeY, short colorIndex);

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
        cars[currentCar] = Car(randInt(X_MIN, X_MAX - CAR_SIZE_X), carY[currentCar], CAR_SIZE_X, CAR_SIZE_Y, currentCar + 1, RECTANGLE, randInt(MIN_CAR_STEP, MAX_CAR_STEP));
        cars[currentCar].draw();
        currentCar++;
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
        life = INITIAL_LIFE;
        score = INITIAL_SCORE;
        frog = Frog(PLAYER_INIT_X, PLAYER_INIT_Y, PLAYER_SIZE_X, PLAYER_SIZE_Y, PLAYER_COLOR, RECTANGLE, PLAYER_STEP, RIGHT, SCHOOL);
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
                score = min(score + 1, 99);
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
    Poop(int x, int y, int sizeX, int sizeY, short colorIndex, SHAPE shape) : Obstacle(x, y, sizeX, sizeY, colorIndex, shape)
    {
        this->speed = MIN_POOP_STEP;
        this->dir = DOWN;
    }
    ~Poop() {}

    void poopSpeedIncrease()
    {
        speed = min(speed + 1, MAX_POOP_STEP);
    }

    void move() override
    {
        y = min(y + speed, Y_MAX - sizeY);
        if (y >= Y_MAX - sizeY)
        {
            clear();
            y = INVALID_POOP_Y;
            return;
        }
        poopSpeedIncrease();
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
        life = INITIAL_LIFE;
        score = INITIAL_SCORE;
        currentPoop = 0;
        for (i = 0; i < MAX_POOP; i++)
        {
            poops[i] = Poop(0, INVALID_POOP_Y, POOP_SIZE_X, POOP_SIZE_Y, POOP_COLOR, RECTANGLE);
        }
        man = Man(PLAYER_INIT_X, PLAYER_INIT_Y, PLAYER_SIZE_X, PLAYER_SIZE_Y, PLAYER_COLOR, RECTANGLE, PLAYER_STEP, RIGHT);
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
            if (poops[i].getY() == INVALID_POOP_Y)
            {
                continue;
            }
            poops[i].clear();
            poops[i].move();
            if (poops[i].getY() == INVALID_POOP_Y)
            {
                currentPoop--;
                score = min(score + 1, 99);
                continue;
            }
            if (poops[i].isCollideWith(&man))
            {
                ScoreSound(2, 3);
                currentPoop--;
                poops[i].setY(INVALID_POOP_Y);
                man.clear();
                man.draw();
                life--;
                if (life < 0)
                {
                    return;
                }
            }
            poops[i].draw();
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
    void drawStandbyTitle()
    {
        Lcd_Draw_Box(0, 0, LCDW, 30, color[BACK_COLOR]);
        lcdDrawString(30, 0, "SELECT GAME", 4, 6);
        Lcd_Draw_Box(0, 150, LCDW, 50, color[BACK_COLOR]);
        lcdDrawString(58, 150, "PUSH BUTTON A", 4, 4);
        lcdDrawString(98, 180, "TO START", 4, 4);
        isTitleDrawn = true;
    }
    void drawStandbyGameName()
    {
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

public:
    GameManager()
    {
        frogGame = FrogGame(INITIAL_LIFE, INITIAL_SCORE);
        poopGame = PoopGame(INITIAL_LIFE, INITIAL_SCORE);
        isTitleDrawn = false;
        isGameChanged = true;
        currentGame = FROGGER;
        status = STANDBY;
    }
    ~GameManager() {}
    void gameManagerInit()
    {
        Lcd_Clr_Screen();
        isTitleDrawn = false;
        isGameChanged = true;
        currentGame = FROGGER;
        status = STANDBY;
        while (status == STANDBY)
        {
            if (!isTitleDrawn)
                drawStandbyTitle();
            if (isGameChanged)
                drawStandbyGameName();
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
 * SOUND RELATED FUNCTION
 *
 *==========================================================*/

static void buzzerBeep(unsigned char tone, int duration)
{
    const static unsigned short tone_value[] = {261, 277, 293, 311, 329, 349, 369, 391, 415, 440, 466, 493, 523, 554, 587, 622, 659, 698, 739, 783, 830, 880, 932, 987};

    TIM3_Out_Freq_Generation(tone_value[tone]);
    TIM2_Delay(duration);
    TIM3_Out_Stop();
}

void ScoreSound(int index1, int index2)
{
    const int song1[][2] = {{G1, N4}, {G1, N4}, {E1, N8}, {F1, N8}, {G1, N4}, {A1, N4}, {A1, N4}, {G1, N2}, {G1, N4}, {C2, N4}, {E2, N4}, {D2, N8}, {C2, N8}, {D2, N2}};
    // const char *note_name[] = {"C1", "C1#", "D1", "D1#", "E1", "F1", "F1#", "G1", "G1#", "A1", "A1#", "B1", "C2", "C2#", "D2", "D2#", "E2", "F2", "F2#", "G2", "G2#", "A2", "A2#", "B2"};

    int n = song1[index1][0]; // key
    int m = song1[index2][1]; // note
    TIM3_Out_Init();
    buzzerBeep(n, m);
}

/*==========================================================*
 *
 * SOUND RELATED FUNCTION END
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

uint32_t getADCNoise()
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

inline void setSeedADC()
{
    srand(getADCNoise());
}

inline int max(int a, int b)
{
    return a > b ? a : b;
}

inline int min(int a, int b)
{
    return a < b ? a : b;
}

inline int randInt(int min, int max)
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

inline void drawRectangle(int x, int y, int sizeX, int sizeY, short colorIndex)
{
    Lcd_Draw_Box(x, y, sizeX, sizeY, color[colorIndex]);
}

inline void drawCircle(int x, int y, int sizeX, int sizeY, short colorIndex)
{
    Uart1_Printf("Currently not supported\n");
    // Lcd_Draw_Circle(x, y, size, color[colorIndex]);
}

inline void drawInvalid(int x, int y, int sizeX, int sizeY, short colorIndex)
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
    Lcd_Init();
    Jog_Poll_Init();
    Jog_ISR_Enable(1);
    Uart1_RX_Interrupt_Enable(1);

    Uart1_Printf("HEY!\n\n");
    Uart1_Printf("Game Project initiated, please enjoy.\n\n");

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
    GameManager gameManager;

    for (;;)
    {
        gameManager.gameManagerInit();
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