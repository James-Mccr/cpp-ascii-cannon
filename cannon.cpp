#include "lib/console.h"
#include "lib/frame.h"
#include "lib/grid.h"
#include "lib/input.h"
#include "lib/render.h"
#include <algorithm>
#include <memory>

using namespace std;

static Grid grid{0, 0};
static UserInput userInput;

class Target
{
public:
    Target()
    {
        x = rand() % grid.GetWidth();
        y = rand() % grid.GetHeight();
        grid.SetTile(x, y, ascii);
    }

    void Update()
    {
        grid.SetTile(x,y,ascii);
    }

    static constexpr char ascii = '+';

private:
    int x{};
    int y{};
};

class Player
{
public:

    void Fire(int xSpeed, int ySpeed)
    {
        grid.SetTile(x, y);
        speed[0] = xSpeed*2;
        speed[1] = (grid.GetHeight()-ySpeed-1)*2;
        x = 0;
        y = grid.GetHeight()-1;
        momentum[0] = 0;
        momentum[1] = 0;
        fire = true;
    }

    void Update()
    {
        if (!fire) return;

        grid.SetTile(x, y);
    
        bool grounded{grid.IsOutOfBounds(x, y+1)};
        if (grid.IsOutOfBounds(x+1, y))
            speed[0] = 0;
        if (!grounded)
            speed[1] -= GRAVITY;
            
        momentum[0] += speed[0];
        momentum[1] += speed[1];

        if (momentum[0] > 30 || momentum[0] < -30)
        {
            int direction = momentum[0] > 0 ? 1 : -1;
            if (!grid.IsOutOfBounds(x+direction, y))
                x += direction;
            momentum[0] = 0;
        }
            
        if (momentum[1] > 30 || momentum[1] < -30)
        {
            int direction = momentum[1] > 0 ? -1 : 1;
            if (grounded && direction == 1)
                speed[0] = 0;
            else if (!grid.IsOutOfBounds(x, y+direction))
                y += direction;
            momentum[1] = 0;
        }

        if (grid.IsCollision(x, y, Target::ascii))
            hitTarget = true;

        grid.SetTile(x, y, ascii);
    }

    const bool GetFire() const { return fire; }
    const bool GetHitTarget() const { return hitTarget; }

    static constexpr char ascii = '@';

private:
    bool hitTarget{};
    bool fire{};
    int x{0};
    int y{grid.GetHeight()-1};
    int speed[2]{30, 10};
    int momentum[2]{0,0};
    static constexpr int GRAVITY = 1;

};

class Cannon
{
public:
    void Update()
    {
        grid.SetTile(x, y);

        launch = false;
        int _x = x;
        int _y = y;

        if (userInput == UserInput::Down) y++;
        else if (userInput == UserInput::Up) y--;
        else if (userInput == UserInput::Right) x++;
        else if (userInput == UserInput::Left) x--;
        else if (userInput == UserInput::Space) launch = true;

        if (grid.IsOutOfBounds(x, y))
        {
            x = _x;
            y = _y;
        }

        grid.SetTile(x, y, ascii);
    }

    const int GetX() { return x; }
    const int GetY() { return y; }
    const bool GetLaunch() { return launch; }

    static constexpr char ascii = 'x';

private:
    int x{};
    int y{};
    bool launch{};
};

class Game
{
public:

    void Update()
    {
        target.Update();
        cannon.Update();
        if (cannon.GetLaunch())
            player.Fire(cannon.GetX(), cannon.GetY());            
        else 
        {
            player.Update();
            win = player.GetHitTarget();
        }
    }

    const bool IsWin() { return win; }

private:
    Cannon cannon{};
    Player player{};
    Target target{};
    bool win{};
};

int main()
{
    srand(time(NULL));
    Console console{};
    grid = {console.width, console.height};
    Frame frame{30};
    Input input{};
    Render render{console};
    Game game{};

    while (1)
    {
        frame.limit();

        userInput = input.Read();

        if (userInput == UserInput::Quit) break;

        game.Update();

        if (game.IsWin())
        {
            console.moveCursor(console.height/2, console.width/3);
            console.print("You win!");
            break;
        }

        render.Draw(grid.GetTiles());
    }

    frame = {1};
    frame.limit();
    frame.limit();
    frame.limit();

    return 0;
}