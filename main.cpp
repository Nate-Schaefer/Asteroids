//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// programmer: Nate Schaefer
// date: 11/23/2022
// name: hw7
// desc: asteroids! Move around with the arrow keys and shoot with the spacebar, destroy all the asteroids!
// extra features:
// reformatted code
// background music
// explosion sound
// shooting sound
// made a new that UFO tracks the player and shoots at the player
// game over screen
// lives display
// score display
// respawn forcefield
// instructions window
// power ups:
// shotgun powerup, fire five bullets at a time
// sniper powerup, bullets go straight through every object
// forcefield powerup, invincible to anything
// bomb powerup, shoots bullets all around the player once by hitting B on the keyboard
// speed powerup, player moves faster, higher max speed, and rotates faster
// teleport powerup, when player hits the T key, player teleports to a random spot on the map
// cluster powerup, when player dies, shoots a ton of bullets where they died
// extra life powerup, adds a life when picked up
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <SFML/Graphics.hpp>
#include <ctime>
#include <list>
#include <cmath>
#include <SFML/Audio.hpp>

using namespace sf;

// used as two of the outside borders of x and y, also for the size of the window
const int W = 1200;
const int H = 800;

// for conversions of degrees to radians
float DEGTORAD = 0.017453f;

// class for animations
class Animation {
public:
    float Frame{}, speed{};
    Sprite sprite;
    std::vector<IntRect> frames;

    Animation() = default;

    // constructor for an animation
    // inputs: t(the texture of explosion), x and y are position, dx and dy are change in position,
    // count is how many frames there are, and speed is how fast the animation happens
    // outputs: displays a destruction animation for the necessary objects
    Animation(Texture &t, int x, int y, int w, int h, int count, float Speed) {
        Frame = 0;
        speed = Speed;

        // puts each frame into  the frame vector
        for (int i = 0; i < count; i++) {
            frames.push_back(IntRect(x + i * w, y, w, h));
        }

        // these 3 lines set the sprite be written to the window and use the first frame
        sprite.setTexture(t);
        sprite.setOrigin(w / 2, h / 2);
        sprite.setTextureRect(frames[0]);
    }


    // function for updating the board
    // inputs: none
    // outputs: puts the necessary frame as the sprite texture
    void update() {
        // the frames vector is incremented by the value that the speed holds (Frame is the variable used for indexing)
        Frame += speed;
        int n = frames.size();

        // if the Frame val is out of range, subtract the vector size form it
        if (Frame >= n) {
            Frame -= n;
        }
        if (n > 0) {
            sprite.setTextureRect(frames[int(Frame)]);
        }
    }

    // function for checking if adding the speed would cause the Frame variable to be out of index for the framse vector
    // inputs: none
    // outputs: true if the frame and speed sum is greater than the size of the framse vector, false if otherwise
    bool isEnd() const {
        return Frame + speed >= frames.size();
    }

};

// class for each entity in the game, astroids, players, bullets, explosions,
class Entity {
public:
    float x{}, y{},  // x and y position
    dx{}, dy{},   // change in x and y position
    R{},       // R is the hit radius, if anything is inside it, it triggers a collision,
    angle{}; // angle of direction

    // life is a bool, true if entity is alive, false if not, if its not alive, it will be deleted from the list
    bool life;
    std::string name;
    Animation anim;

    // default constructor for Entity
    Entity() {
        life = true;
    }

    // basically the constructor for the entity, function for settings
    // initializes the animation of moving, position, angle that it's moving at, and hit radius
    void settings(Animation &a, int X, int Y, float Angle = 0, int radius = 1) {
        anim = a;
        x = X;
        y = Y;
        angle = Angle;
        R = radius;
    }

    virtual void update() {}; // allows polymorphism, this function is overrided by the derived classes fucntions

    // function for drawing the entity to the window
    // inputs: uses the window (pass by ref to be able to draw the sprite to it)
    // outputs: draws the entity's sprite to the window
    // made thsis function virtual because I override it with the enemy UFO code
    virtual void draw(RenderWindow &app) {
        anim.sprite.setPosition(x, y);
        anim.sprite.setRotation(angle + 90);
        app.draw(anim.sprite);

        // this is just to illustrate the blast radius of the object
        CircleShape circle(R);
        circle.setFillColor(Color(255, 0, 0, 170));
        circle.setPosition(x, y);
        circle.setOrigin(R, R);
        //app.draw(circle);
    }

    virtual void change_ang(const float playerx, const float playery) {}

    virtual ~Entity() = default;; // also virtual because of the virtual function
};

// asteroid, inherits entity
class asteroid : public Entity {
public:
    // static variabel to count number of asteroids
    static int asteroid_count;

    // constructor, initializes a random direction to the dx and dy variables, increment asteroid count everytime an asteroid object is made
    asteroid() {
        dx = rand() % 8 - 4;
        dy = rand() % 8 - 4;
        name = "asteroid";
        asteroid_count++;
    }

    // function for updating
    // inputs: none
    // outputs: updates the position of the asteroid
    void update() override {
        // changes the position of the asteroid
        x += dx;
        y += dy;

        // these if statements reset the players position to the opposite side of the screen if they go out of bounds
        // ex: if they go
        if (x > W) {
            x = 0;
        }

        if (x < 0) {
            x = W;
        }

        if (y > H) {
            y = 0;
        }

        if (y < 0) {
            y = H;
        }
    }

    // make it so asteroid count decreases everytime an asteroid object is destroyed
    ~asteroid() {
        asteroid_count--;
    }
};
// initializing the static variable to 0
int asteroid::asteroid_count = 0;

// class for bullet
class bullet : public Entity {
public:
    // just initializes name
    bullet() {
        name = "bullet";
    }

    // function for updating
    // inputs: none
    // outputs: updates the position of the bullet, changes x and y components
    void update() override {
        // uses the angle of the player to
        dx = cos(angle * DEGTORAD) * 6;
        dy = sin(angle * DEGTORAD) * 6;

        // curvy bullets
        // angle+=rand()%6-3;

        // change the position
        x += dx;
        y += dy;

        // if it goes out of the screen, it is dead
        if (x > W || x < 0 || y > H || y < 0) {
            life = 0;
        }
    }

};

// class for player
class player : public Entity {
public:
    // thrust is used for the jets, moving the player
    bool thrust{};
    bool forcefield;
    bool speedUp;

    // initializes name
    player() {
        name = "player";
    }

    void FFOn(bool isOn) {
        if (isOn) {
            R = 40;
            forcefield = true;
        }
        else {
            forcefield = false;
            R = 20;
        }
    }

    void speedOn(bool isOn) {
        if (isOn) {
            speedUp = true;
        }
        else {
            speedUp = false;
        }
    }

    // function for drawing the entity to the window
    // inputs: uses the window (pass by ref to be able to draw the sprite to it)
    // outputs: draws the entity's sprite to the window
    void draw(RenderWindow &app) override {
        anim.sprite.setPosition(x, y);
        anim.sprite.setRotation(angle + 90);
        app.draw(anim.sprite);

        // this is just to illustrate the blast radius of the object
        CircleShape circle(R);
        circle.setFillColor(Color(0, 0, 255, 125));
        circle.setPosition(x, y);
        circle.setOrigin(R, R);
        if (forcefield) {
            app.draw(circle);
        }

    }

    // function for updating
    // inputs: none
    // outputs: updates the position of the player
    void update() override {
        // if player is thrusting, make it going faster, otherwise, slowly slow it down
        if (thrust) {
            // accelerates up the player in the angle that they are facing
            dx += cos(angle * DEGTORAD) * 0.2;
            dy += sin(angle * DEGTORAD) * 0.2;
            // acceleration will be faster if speedup powerup function is on
            if (speedUp) {
                dx += cos(angle * DEGTORAD) * 0.3;
                dy += sin(angle * DEGTORAD) * 0.3;
            }
            else {
                dx += cos(angle * DEGTORAD) * 0.2;
                dy += sin(angle * DEGTORAD) * 0.2;
            }
        } else {
            // slows down player
            dx *= 0.99;
            dy *= 0.99;
        }

        // this keeps the player from going faster than the top speed
        int maxSpeed;
        // allow for higher max speed if speedup powerup is on
        if (speedUp) {
            maxSpeed = 25;
        }
        else {
            maxSpeed = 15;
        }

        // uses pythagorean theorem to find speed, makes sure that speed is less than max speed
        float speed = sqrt(dx * dx + dy * dy);
        if (speed > maxSpeed) {
            dx *= maxSpeed / speed;
            dy *= maxSpeed / speed;
        }

        // move player
        x += dx;
        y += dy;

        // these if statements reset the position of the player to the other side of the screen if they go out of the screen
        if (x > W) {
            x = 0;
        }
        if (x < 0) {
            x = W;
        }
        if (y > H) {
            y = 0;
        }
        if (y < 0) {
            y = H;
        }
    }

};

// class for rubric UFO
class UFO : public Entity {
public:
    // constructor
    UFO() {
        dx = rand() % 3 + 1;
        name = "UFO";
    }

    // update function for UFO
    // inputs: none
    // outputs: updates the position of the UFO, ends UFO if it goes out of bounds
    void update() override {
        x += dx;
        if (x < 0 || x > W) {
            life = 0;
        }
    }
};

// this is the extra credit UFO that I created
class enemyUFO : public Entity {
public:
    // these hold the position of the player
    int px;
    int py;

    // initializes name
    enemyUFO() {
        dx = rand() % 8 - 4;
        dy = rand() % 8 - 4;
        name = "enemyUFO";
    }

    // this is just a setter for px and py
    // desc: takes in the position of the player to be used in the update function
    // inputs: int playerx, int playery, these are the x and y positions of the player
    // outputs: sets the px and py variables
    void change_ang(const float playerx, const float playery) override {
        px = playerx;
        py = playery;
    }

    // update function for enemyUFO
    // inputs: none
    // outputs: adjusts the angle to track the player, keeps the UFO from going out of bounds
    void update() override {
        x += dx;
        y += dx;
        // using the inverse tan function, using opposite (difference in x values) over adjacent (difference in y values), had to make it negative, otherwise the UFO would move in the opposite angular direction
        angle = -atan((px - x) / (py - y)) / DEGTORAD;

        // UFO is in 2nd quadrant in reference to player
        // adding degrees to the angle to adjust the inverse tan function
        if(px >= x && py >= y) {
            angle += 270;
        }
        // 1st quadrant
        else if (px <= x && py >= y) {
            angle += 270;
        }
        // 3rd quadrant
        else if (px >= x && py <= y) {
            angle += 90;
        }
        // 4th quadrant
        else if (px <= x && py <= y) {
            angle += 90;
        }
        // to make sure it doesn't go out of bounds
        if (x > W) {
            x = 0;
        }
        if (x < 0) {
            x = W;
        }
        if (y > H) {
            y = 0;
        }
        if (y < 0) {
            y = H;
        }
    }
};

// class for powerups
class powerup : public Entity {
public:
    // need to input to differentiate powerups
    powerup(String Name) {
        name = Name;
    }
};

// class for red bullets shot from UFOs
class UFObullet : public bullet {
public:
    UFObullet() {
        name = "UFObullet";
    }
};

// function name: isCollide
// inputs: two entities, a and b
// outputs: true if the entities hit radii infringe on each others' radii by using a mathematical formula, false if otherwise
bool isCollide(Entity *a, Entity *b) {
    return (b->x - a->x) * (b->x - a->x) + (b->y - a->y) * (b->y - a->y) < (a->R + b->R) * (a->R + b->R);
}


int main() {
    srand(time(nullptr));

    // rendering instruction window
    RenderWindow instruct(VideoMode(W, H), "Asteroids!");
    instruct.setFramerateLimit(60);


    //
    Music background_music;

    // this sound is from https://mixkit.co/free-sound-effects/cinematic/ (I used "Space Soundscape"), license: free for commercial and non-commercial use (Mixkit license)
    if (!background_music.openFromFile("sounds/background.wav")) {
        return EXIT_FAILURE;
    }

    background_music.setLoop(true);

    SoundBuffer enemyUFO_warning_buffer;                                   // creates buffer to load in match sound
    SoundBuffer exp_buffer;
    SoundBuffer shot_buffer;
    SoundBuffer UFO_warning_buffer;

    // this is the warning sound that I prefer, but it is too large, so I just use it for my own enjoyment
    // this sound is from https://mixkit.co/free-sound-effects/cinematic/, (I used "Cinematic trailer apocalypse horn"), license: free for commercial and non-commercial use (Mixkit license)
//    if (!enemyUFO_warning_buffer.loadFromFile("sounds/UFO_warning_big.ogg")) {     // checks if sound was loaded
//        return EXIT_FAILURE;
//    }

    // this sound is from https://mixkit.co/free-sound-effects/cinematic/?page=2, (I used "cinematic woosh"), license: free for commercial and non-commercial use (Mixkit license)
    if (!enemyUFO_warning_buffer.loadFromFile("sounds/UFO_warning_small.ogg")) {     // checks if sound was loaded
        return EXIT_FAILURE;
    }

    // this sound is from https://mixkit.co/free-sound-effects/explosion/, (I used "fast game explosion"), license: free for commercial and non-commercial use (Mixkit license)
    if (!exp_buffer.loadFromFile("sounds/explosion.ogg")) {     // checks if sound was loaded
        return EXIT_FAILURE;
    }

    // this sound is from https://mixkit.co/free-sound-effects/gun/, (I used "laser cannon shot"), license: free for commercial and non-commercial use (Mixkit license)
    if (!shot_buffer.loadFromFile("sounds/laser_shot.ogg")) {     // checks if sound was loaded
        return EXIT_FAILURE;
    }

    // this sound is from https://mixkit.co/free-sound-effects/alarm/, (I used "classic short alarm"), license: free for commercial and non-commercial use (Mixkit license)
    if (!UFO_warning_buffer.loadFromFile("sounds/UFO_on_screen.ogg")) {     // checks if sound was loaded
        return EXIT_FAILURE;
    }

    Sound enemyUFO_warning_sound;
    enemyUFO_warning_sound.setBuffer(enemyUFO_warning_buffer);

    Sound UFO_warning_sound;
    UFO_warning_sound.setBuffer(UFO_warning_buffer);
    UFO_warning_sound.setLoop(true);

    Sound exp_sound;
    exp_sound.setBuffer(exp_buffer);

    Sound shot_sound;
    shot_sound.setBuffer(shot_buffer);

    Texture t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11;
    if(!t1.loadFromFile("images/spaceship.png")) {
        return EXIT_FAILURE;
    }
    if(!t2.loadFromFile("images/background.jpg")) {
        return EXIT_FAILURE;
    }
    if(!t3.loadFromFile("images/explosions/type_C.png")) {
        return EXIT_FAILURE;
    }
    if(!t4.loadFromFile("images/rock.png")) {
        return EXIT_FAILURE;
    }
    if(!t5.loadFromFile("images/fire_blue.png")) {
        return EXIT_FAILURE;
    }
    if(!t6.loadFromFile("images/rock_small.png")) {
        return EXIT_FAILURE;
    }
    if(!t7.loadFromFile("images/explosions/type_B.png")) {
        return EXIT_FAILURE;
    }
    // got this from flaticon.com, license: free for personal or commercial use with attrition
    if(!t8.loadFromFile("images/enemy_ufo.png")) {
        return EXIT_FAILURE;
    }
    // got this from https://opengameart.org/content/breakout-graphics-no-shadow by Scribe. license: Creative commons, free to use personally and commercially
    if(!t9.loadFromFile("images/power_ups.png")) {
        return EXIT_FAILURE;
    }
    if(!t10.loadFromFile("images/fire_red.png")) {
        return EXIT_FAILURE;
    }
    // got this from flaticon.com, license: free for personal or commercial use with attrition
    if(!t11.loadFromFile("images/ufo.png")) {
        return EXIT_FAILURE;
    }

    // makes texture appear smoother
    t1.setSmooth(true);
    t2.setSmooth(true);

    Sprite background(t2);

    // initializes the animations for each entity
    Animation sExplosion(t3, 0, 0, 256, 256, 48, 0.5);
    Animation sRock(t4, 0, 0, 64, 64, 16, 0.2);
    Animation sRock_small(t6, 0, 0, 64, 64, 16, 0.2);
    Animation sBullet(t5, 0, 0, 32, 64, 16, 0.8);
    Animation sUFOBullet(t10, 0, 0, 32, 64, 16, 0.8);
    Animation sPlayer(t1, 40, 0, 40, 40, 1, 0);
    Animation sPlayer_go(t1, 40, 40, 40, 40, 1, 0);
    Animation sExplosion_ship(t7, 0, 0, 192, 192, 64, 0.5);
    Animation sUFO(t11, 0, 0, 40, 40, 1, 0);
    Animation sEnemyUFO(t8, 0, 0, 40, 40, 1, 0);
    Animation sShotgunPow(t9, 0, 0, 40, 40, 1, 0);
    Animation sSniperPow(t9, 50, 0, 40, 40 , 1, 0);
    Animation sForcefieldPow(t9, 200, 0, 40, 40, 1, 0);
    Animation sBombPow(t9, 250, 150, 40, 40, 1, 0);
    Animation sSpeedPow(t9, 0, 200, 40, 40, 1, 0);
    Animation sTelePow(t9, 100, 150, 40, 40, 1, 0);
    Animation sClusterPow(t9, 100, 100, 40, 40, 1, 0);
    Animation sExtraLifePow(t9, 150, 100, 40, 40, 1, 0);

    // this list holds all of the entities in the game
    std::list<Entity *> entities;

    // makes 15 asteroids at the start dynamically
    for (int i = 0; i < 15; i++) {
        asteroid *a = new asteroid();
        // initializes each asteroid, random place inside the window, random angle under 360
        a->settings(sRock, rand() % W, rand() % H, rand() % 360, 25);
        entities.push_back(a);
    }

    // make player, same starting place,
    player *p = new player();
    p->settings(sPlayer, 200, 200, 0, 20);
    entities.push_back(p);

    // bools for when an object exists, or when a powerup is on or off
    bool gameover;
    bool enemyExist = false;
    bool UFOExist = false;
    bool shotgunPowOn = false;
    bool sniperPowOn = false;
    bool FFPowOn = false;
    bool FFRespOn = false;
    bool bombPowOn = false;
    bool speedPowOn = false;
    bool telePowOn = false;
    bool clusterPowOn = false;

    // bools for when the powerup is on screen, waiting to be picked up
    bool shotgunPowOnScrn = false;
    bool sniperPowOnScrn = false;
    bool FFPowOnScrn = false;
    bool bombPowOnScrn = false;
    bool speedPowOnScrn = false;
    bool telePowOnScrn = false;
    bool clusterPowOnScrn = false;
    bool extraLifePowOnScrn = false;

    // these are used for timing something like a timed powerup
    Clock shotPowTimer;
    Clock snipePowTimer;
    Clock FFPowTimer;
    Clock speedPowTimer;
    Clock UFOShotTimer;
    Clock clusterPowTimer;
    Clock FFRespTimer;
    Clock gameoverTimer;
    // used for comparing the timers
    Time maxPowTime = seconds(7.f);
    Time respawnTime = seconds(3.f);
    Time UFOShotTime = seconds(3.f);
    Time gameoverTime = seconds(2.f);

    // lives and score
    int lives = 10;
    int score = 0;

    Font font;
    // font from https://www.1001freefonts.com/famous-fonts.php, license: free for personal use
    if(!font.loadFromFile("fonts/METALORD.TTF")) {
        return EXIT_FAILURE;
    }
    Text gameOverMessage;
    Text scoreMessage;
    Text livesMessage;

    // setting up texts
    scoreMessage.setFont(font);
    scoreMessage.setPosition(10, 50);
    scoreMessage.setCharacterSize(30);
    scoreMessage.setFillColor(Color::White);

    livesMessage.setFont(font);
    livesMessage.setPosition(65, 10);
    livesMessage.setCharacterSize(30);
    livesMessage.setFillColor(Color::White);

    gameOverMessage.setFont(font);
    gameOverMessage.setPosition(360, 250);
    gameOverMessage.setCharacterSize(100);
    gameOverMessage.setFillColor(Color::White);

    // ship sprite used for lives
    Sprite fakeShip;
    fakeShip.setTexture(t1);
    fakeShip.setOrigin(40 / 2, 40 / 2);
    fakeShip.setTextureRect(IntRect(40, 0, 40, 40));
    fakeShip.setPosition(30, 30);

    // in order to use angle for enemy bullets
    enemyUFO * u;

    Text instructions;
    instructions.setFont(font);
    instructions.setPosition(120, 25);
    instructions.setCharacterSize(35);
    instructions.setFillColor(Color::White);
    instructions.setString("Power Ups: \n\n"
                           "shotgun: shoot multiple shots at once\n\n"
                           "sniper: bullets go through any object\n\n"
                           "forcefield: invincibility for a short time\n\n"
                           "bomb: shoot a ton of bullets by hitting B\n\n"
                           "speedup: move around faster\n\n"
                           "teleport: teleport to a random spot by hitting T\n\n"
                           "cluster: sprays a ton of bullets when colliding\n\n"
                           "extra life: get one extra life");


    // these are for the sprites at the instructiosn screen
    Sprite shotgun;
    shotgun.setTexture(t9);
    shotgun.setOrigin(40 / 2, 40 / 2);
    shotgun.setTextureRect(IntRect(0, 0, 40, 40));
    shotgun.setPosition(60, 130);

    Sprite sniper;
    sniper.setTexture(t9);
    sniper.setOrigin(40 / 2, 40 / 2);
    sniper.setTextureRect(IntRect(50, 0, 40, 40));
    sniper.setPosition(60, 220);

    Sprite forcefield;
    forcefield.setTexture(t9);
    forcefield.setOrigin(40 / 2, 40 / 2);
    forcefield.setTextureRect(IntRect(200, 0, 40, 40));
    forcefield.setPosition(60, 310);

    Sprite bomb;
    bomb.setTexture(t9);
    bomb.setOrigin(40 / 2, 40 / 2);
    bomb.setTextureRect(IntRect(250, 150, 40, 40));
    bomb.setPosition(60, 390);

    Sprite speed;
    speed.setTexture(t9);
    speed.setOrigin(40 / 2, 40 / 2);
    speed.setTextureRect(IntRect(0, 200, 40, 40));
    speed.setPosition(60, 480);

    Sprite teleport;
    teleport.setTexture(t9);
    teleport.setOrigin(40 / 2, 40 / 2);
    teleport.setTextureRect(IntRect(100, 150, 40, 40));
    teleport.setPosition(60, 560);

    Sprite cluster;
    cluster.setTexture(t9);
    cluster.setOrigin(40 / 2, 40 / 2);
    cluster.setTextureRect(IntRect(100, 100, 40, 40));
    cluster.setPosition(60, 650);

    Sprite extralife;
    extralife.setTexture(t9);
    extralife.setOrigin(40 / 2, 40 / 2);
    extralife.setTextureRect(IntRect(150, 100, 40, 40));
    extralife.setPosition(60, 735);


    instruct.clear(Color::Black);

    while(instruct.isOpen()) {
        Event event{};
        // if player closes window
        while (instruct.pollEvent(event)) {
            if (event.type == Event::Closed) {
                instruct.close();
            }

        }
        instruct.draw(instructions);
        instruct.draw(shotgun);
        instruct.draw(sniper);
        instruct.draw(forcefield);
        instruct.draw(bomb);
        instruct.draw(speed);
        instruct.draw(teleport);
        instruct.draw(cluster);
        instruct.draw(extralife);

        instruct.display();
    }

    RenderWindow app(VideoMode(W, H), "Asteroids!");
    app.setFramerateLimit(60);

    background_music.play();
    /////main loop/////
    while (app.isOpen()) {
        Event event{};
        // if player closes window
        while (app.pollEvent(event)) {
            if (event.type == Event::Closed) {
                app.close();
            }
            if(lives != 0) {
                if (event.type == Event::KeyPressed) {
                    if (event.key.code == Keyboard::Space) {
                        shot_sound.play();
                        if (!shotgunPowOn) {
                            bullet *b = new bullet();
                            b->settings(sBullet, p->x, p->y, p->angle + 3, 10);
                            entities.push_back(b);
                        }
                            // shotgun powerup is in effect
                        else {
                            // if maxPowTime seconds have elapsed, turn the powerup off
                            if (shotPowTimer.getElapsedTime() > maxPowTime) {
                                shotgunPowOn = false;
                            }
                            // makes 5 shots, two with a smaller angle, two with a large angle, and one with the same angle
                           for (int i = -2; i < 3; i++) {
                                bullet *b = new bullet();
                                b->settings(sBullet, p->x, p->y, p->angle + (i * 5), 10);
                                entities.push_back(b);
                            }
                        }
                    }
                }
            }
        }
        if(!gameover) {
            // changes players angle by incrementing by three when player uses left or right keys
            if (Keyboard::isKeyPressed(Keyboard::Right)) {
                if (speedPowOn) {
                    p->angle += 6;
                } else {
                    p->angle += 3;
                }
            }
            if (Keyboard::isKeyPressed(Keyboard::Left)) {
                if (speedPowOn) {
                    p->angle -= 6;
                } else {
                    p->angle -= 3;
                }
            }
            // turn thrust on when player uses up key
            if (Keyboard::isKeyPressed(Keyboard::Up)) {
                p->thrust = true;
            }
                // turns thrust off
            else {
                p->thrust = false;
            }

            if (Keyboard::isKeyPressed(Keyboard::B) && bombPowOn) {
                shot_sound.play();
                for (int i = 0; i < 40; i++) {
                    bullet *b = new bullet();
                    b->settings(sBullet, p->x, p->y, p->angle + (i * 9), 10);
                    entities.push_back(b);
                }
                bombPowOn = false;
            }

            // if player presses T, they are teleported to a random location
            if (Keyboard::isKeyPressed(Keyboard::T) && telePowOn) {
                p->x = rand() % W;
                p->y = rand() % H;
                telePowOn = false;
            }
            p->speedOn(speedPowOn);

            // goes through each entity and checks if they are colliding each time the code runs through this loop
            // goes through with each element and compares it with the rest of the list of elements
            for (auto a: entities) {
                for (auto b: entities) {
                    // if a bullet collides with an asteroid, it makes the smaller asteroids
                    if (a->name == "asteroid" && b->name == "bullet") {
                        //stops both of them
                        if (isCollide(a, b)) {
                            exp_sound.play();
                            score += 1;
                            a->life = false;
                            // bullet keeps on livin if sniper powerup is on
                            if (!sniperPowOn) {
                                b->life = false;
                            }
                            // makes an explosion
                            Entity *e = new Entity();
                            e->settings(sExplosion, a->x, a->y);
                            e->name = "explosion";
                            entities.push_back(e);

                            // makes two new asteroids if the asteroid is a big one
                            // checks this by seeing if the radius is equal to 15, if it is, breaks out of the for loop
                            for (int i = 0; i < 2; i++) {
                                if (a->R == 15) {
                                    continue;
                                }
                                Entity *e = new asteroid();
                                e->settings(sRock_small, a->x, a->y, rand() % 360, 15);
                                entities.push_back(e);
                            }
                        }
                    }
                    // if the player collides with an asteroid or UFO or UFO bullet
                    if (a->name == "player" && b->name == "asteroid" || a->name == "player" && b->name == "enemyUFO" ||
                        a->name == "player" && b->name == "UFObullet" || a->name == "player" && b->name == "UFO") {
                        // stops only the asteroid
                        if (isCollide(a, b)) {
                            exp_sound.play();
                            b->life = false;
                            // makes an explosion
                            Entity *e = new Entity();
                            e->settings(sExplosion_ship, a->x, a->y);
                            e->name = "explosion";
                            entities.push_back(e);

                            if (clusterPowOn) {
                                for (int i = 0; i < 60; i++) {
                                    bullet *b = new bullet();
                                    b->settings(sBullet, p->x, p->y, p->angle + (i * 6), 10);
                                    entities.push_back(b);
                                }
                            }

                            // reinitializes the players settings, to the middle of the map
                            if (!FFPowOn && !FFRespOn) {
                                p->settings(sPlayer, W / 2, H / 2, 0, 20);
                                p->dx = 0;
                                p->dy = 0;
                                FFRespOn = true;
                                FFRespTimer.restart();
                                lives -= 1;
                            }
                            if (lives == 0) {
                                gameoverTimer.restart();
                            }

                            // stop warning sound if it is happening and resume background sound
                            if (b->name == "enemyUFO") {
                                enemyUFO_warning_sound.stop();
                            }
                            if (b->name == "UFO") {
                                UFO_warning_sound.stop();
                            }
                        }
                    }
                    if (a->name == "enemyUFO" && b->name == "bullet" || a->name == "UFO" && b->name == "bullet") {
                        //stops both of them
                        if (isCollide(a, b)) {
                            exp_sound.play();
                            score += 5;
                            a->life = false;
                            // bullet doesn't die if sniper powerup is on
                            if (!sniperPowOn) {
                                b->life = false;
                            }
                            enemyExist = false;
                            // makes an explosion
                            Entity *e = new Entity();
                            e->settings(sExplosion, a->x, a->y);
                            e->name = "explosion";
                            entities.push_back(e);

                            // stop warning sound if it is happening
                            if (a->name == "enemyUFO") {
                                score += 5;
                                enemyUFO_warning_sound.stop();
                            }
                            if(a->name == "UFO") {
                                UFOExist = false;
                            }
                        }
                    }
                    if (a->name == "UFObullet" && b->name == "bullet") {
                        //stops both of them
                        if (isCollide(a, b)) {
                            exp_sound.play();
                            a->life = false;
                            // bullet doesn't die if sniper powerup is on
                            if (!sniperPowOn) {
                                b->life = false;
                            }
                            enemyExist = false;
                            // makes an explosion
                            Entity *e = new Entity();
                            e->settings(sExplosion, a->x, a->y);
                            e->name = "explosion";
                            entities.push_back(e);
                        }
                    }

                    // These next if statements are for when the player interacts with power ups
                    if (a->name == "player" && b->name == "shotgun") {
                        if (isCollide(a, b)) {
                            // start timer, get rid of power up
                            shotgunPowOn = true;
                            shotgunPowOnScrn = false;
                            shotPowTimer.restart();
                            b->life = false;
                        }
                    }
                    if (a->name == "player" && b->name == "sniper") {
                        if (isCollide(a, b)) {
                            // start timer, get rid of power up
                            sniperPowOn = true;
                            sniperPowOnScrn = false;
                            snipePowTimer.restart();
                            b->life = false;
                        }
                    }
                    if (a->name == "player" && b->name == "forcefield") {
                        if (isCollide(a, b)) {
                            // start timer, get rid of power up
                            FFPowOn = true;
                            FFPowOnScrn = false;
                            FFPowTimer.restart();
                            b->life = false;
                        }
                    }
                    if (a->name == "player" && b->name == "bomb") {
                        if (isCollide(a, b)) {
                            bombPowOn = true;
                            bombPowOnScrn = false;
                            b->life = false;
                        }
                    }
                    if (a->name == "player" && b->name == "teleport") {
                        if (isCollide(a, b)) {
                            telePowOn = true;
                            telePowOnScrn = false;
                            b->life = false;
                        }
                    }
                    if (a->name == "player" && b->name == "speed") {
                        if (isCollide(a, b)) {
                            // start timer, get rid of power up
                            speedPowOn = true;
                            speedPowOnScrn = false;
                            speedPowTimer.restart();
                            b->life = false;
                        }
                    }
                    if (a->name == "player" && b->name == "cluster") {
                        if (isCollide(a, b)) {
                            // start timer, get rid of power up
                            clusterPowOn = true;
                            clusterPowOnScrn = false;
                            clusterPowTimer.restart();
                            b->life = false;
                        }
                    }
                    if (a->name == "player" && b->name == "extraLife") {
                        if (isCollide(a, b)) {
                            lives += 1;
                            extraLifePowOnScrn = false;
                            b->life = false;
                        }
                    }
                }
            }


            // changes the player's animation depending on if they are using the thrust or not
            if (p->thrust) {
                p->anim = sPlayer_go;
            } else {
                p->anim = sPlayer;
            }


            // checks the explosions, if they are at the end, stop them by setting life to zero
            for (auto e: entities) {
                if (e->name == "explosion") {
                    if (e->anim.isEnd()) {
                        e->life = false;
                    }
                }
            }

            // makes a new asteroid if a random generated number is dividable by 150
            // decrease the number and more asteroids will show up
//        if (rand() % 150 == 0) {
//            asteroid *a = new asteroid();
//            a->settings(sRock, 0, rand() % H, rand() % 360, 25);
//            entities.push_back(a);
//        }

            // code to create a new enemyUFO, if there isn't one already
            if (!enemyExist && rand() % 2000 == 0) {
                enemyExist = true;
                enemyUFO_warning_sound.play();
                u = new enemyUFO();
                u->settings(sEnemyUFO, rand() % W, rand() % H, rand() % 360, 25);
                entities.push_back(u);
                UFOShotTimer.restart();
            }
            if (enemyExist) {
                if (UFOShotTimer.getElapsedTime() > UFOShotTime) {
                    shot_sound.play();
                    for (int i = -1; i < 1; i++) {
                        UFObullet *UFOb = new UFObullet();
                        UFOb->settings(sUFOBullet, u->x, u->y, u->angle + 180 + (i * 5), 10);
                        entities.push_back(UFOb);
                    }
                    UFOShotTimer.restart();
                }
            }

            // this block creates a rubric UFO
            if (!UFOExist && rand() % 2000 == 0) {
                UFO_warning_sound.play();
                UFOExist = true;
                UFO *UF = new UFO();
                UF->settings(sUFO, 40, rand() % H, 270, 25);
                entities.push_back(UF);
            }

            // powerup code
            if (!shotgunPowOnScrn && rand() % 2000 == 0) {
                shotgunPowOnScrn = true;
                powerup *shotPow = new powerup("shotgun");
                shotPow->settings(sShotgunPow, rand() % W, rand() % H, 0, 20);
                entities.push_back(shotPow);
            }

            if (!sniperPowOnScrn && rand() % 2000 == 0) {
                sniperPowOnScrn = true;
                powerup *snipePow = new powerup("sniper");
                snipePow->settings(sSniperPow, rand() % W, rand() % H, 0, 20);
                entities.push_back(snipePow);
            }

            if (!FFPowOnScrn && rand() % 2000 == 0) {
                FFPowOnScrn = true;
                powerup *FFPow = new powerup("forcefield");
                FFPow->settings(sForcefieldPow, rand() % W, rand() % H, 0, 20);
                entities.push_back(FFPow);
            }

            if (!bombPowOnScrn && rand() % 200 == 0) {
                bombPowOnScrn = true;
                powerup *bombPow = new powerup("bomb");
                bombPow->settings(sBombPow, rand() % W, rand() % H, 0, 20);
                entities.push_back(bombPow);
            }

            if (!speedPowOnScrn && rand() % 2000 == 0) {
                speedPowOnScrn = true;
                powerup *speedPow = new powerup("speed");
                speedPow->settings(sSpeedPow, rand() % W, rand() % H, 0, 20);
                entities.push_back(speedPow);
            }

            if (!telePowOnScrn && rand() % 2000 == 0) {
                telePowOnScrn = true;
                powerup *telePow = new powerup("teleport");
                telePow->settings(sTelePow, rand() % W, rand() % H, 0, 20);
                entities.push_back(telePow);
            }

            if (!clusterPowOnScrn && rand() % 2000 == 0) {
                clusterPowOnScrn = true;
                powerup *clusterPow = new powerup("cluster");
                clusterPow->settings(sClusterPow, rand() % W, rand() % H, 0, 20);
                entities.push_back(clusterPow);
            }

            if (!extraLifePowOnScrn && rand() % 2000 == 0) {
                extraLifePowOnScrn = true;
                powerup *extraPow = new powerup("extraLife");
                extraPow->settings(sExtraLifePow, rand() % W, rand() % H, 0, 20);
                entities.push_back(extraPow);
            }

            // checking power ups
            // checking sniper power up
            if (sniperPowOn) {
                if (snipePowTimer.getElapsedTime() > maxPowTime) {
                    sniperPowOn = false;
                }
            }

            // checking forcefield power up
            if (FFPowOn || FFRespOn) {
                if (FFPowTimer.getElapsedTime() > maxPowTime) {
                    FFPowOn = false;
                }
                if (FFRespTimer.getElapsedTime() > respawnTime) {
                    FFRespOn = false;
                }
                p->FFOn(true);
            } else {
                p->FFOn(false);
            }


            if (speedPowOn) {
                if (speedPowTimer.getElapsedTime() > maxPowTime) {
                    speedPowOn = false;
                }
            }

            if (clusterPowOn) {
                if (clusterPowTimer.getElapsedTime() > maxPowTime) {
                    clusterPowOn = false;
                }
            }

            if (lives == 0) {
                p->life = false;
                // wait a little bit to make the game over more fluid
                if (gameoverTimer.getElapsedTime() > gameoverTime) {
                    gameover = true;
                }
            }


            // goes through the list updates each element, if the life variable equals 0(equals false), then erase it and delete it
            for (auto i = entities.begin(); i != entities.end();) {
                Entity *e = *i;

                if (e->name == "enemyUFO") {
                    e->change_ang(p->x, p->y);
                }

                e->update();
                e->anim.update();


                if (!e->life) {
                    if (e->name == "UFO") {
                        UFO_warning_sound.stop();
                    }
                    i = entities.erase(i);
                    delete e;
                } else {
                    i++;
                }
            }

            // checks if all asteroids were destroyed, if then, initializes 15 more asteroids
            if (asteroid::asteroid_count == 0) {
                for (int i = 0; i < 15; i++) {
                    asteroid *a = new asteroid();
                    a->settings(sRock, 0, rand() % H, rand() % 360, 25);
                    entities.push_back(a);
                }
            }

            // draws everything that is alive to the window
            //////draw//////
            app.draw(background);
            for (auto i: entities) {
                i->draw(app);
            }

            scoreMessage.setString("Score: " + std::to_string(score));
            livesMessage.setString(std::to_string(lives));

            app.draw(scoreMessage);
            app.draw(livesMessage);
            app.draw(fakeShip);
        }
        else {
            UFO_warning_sound.stop();
            enemyUFO_warning_sound.stop();
            background_music.stop();

            app.clear(sf::Color(0, 0, 0, 255));

            gameOverMessage.setFont(font);
            gameOverMessage.setPosition(360, 250);
            gameOverMessage.setCharacterSize(100);
            gameOverMessage.setFillColor(Color::White);
            gameOverMessage.setString("Game Over! \n Score: " + std::to_string(score));


            app.draw(gameOverMessage);
        }
        app.display();

    }

    return 0;
}