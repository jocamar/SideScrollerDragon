#include "Game.h"

Game::Game()
{
	this->mManager = new MediaManager();
	this->window = new RenderWindow(VideoMode(WINDOW_WIDTH,WINDOW_HEIGHT), "SideScrollerDragon");
	mainView = sf::View(sf::FloatRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT));
}

Game::~Game()
{
}

void Game::init(){
	jumpKeyDown = false;
	moveRequest = false;
	subjectA = new bases::Player(0,0,0,0,16,16,1,1);
	subjectA->setPosition(200, 0);
	int id = this->mManager->loadTexture("hitboxes.png");
	subjectA->setTexture(*this->mManager->getTexture(id));
	int cameraX = (int)subjectA->getPosition().x;
	int cameraY = (int)subjectA->getPosition().y;

	map = new GameMap();
	string src = map->loadMap("testmap");
	//map = new GameMap();
	map->setId(this->mManager->loadTexture(src));
	/* MAP TESTING */
	/*id = this->mManager->loadTexture("tiles.png");
	int x;
	for (size_t i = 0; i < 10; i++){
		for (size_t j = 0; j < 10; j++){
			mapa[i][j] = new bases::Tile(0, 0, 0, 0, 16, 16, 2, 2);
			mapa[i][j]->setPosition(16 * (float)j, 16 * (float)i);
			mapa[i][j]->nextFrame();
			mapa[i][j]->setTexture(*this->mManager->getTexture(id));
			x = mapa[i][j]->getImageX() + mapa[i][j]->getImageWidth()*mapa[i][j]->getMaxFrame()*mapa[i][j]->getAnimSet();
			mapa[i][j]->setTextureRect(sf::IntRect(x, mapa[i][j]->getImageY(), mapa[i][j]->getImageWidth(), mapa[i][j]->getImageHeight()));
		}
	}

	for (size_t i = 0; i < 10; i++){
		mapa[9][i]->previousFrame();
		x = mapa[9][i]->getImageX() + mapa[9][i]->getImageWidth()*mapa[9][i]->getMaxFrame()*mapa[9][i]->getAnimSet();
		mapa[9][i]->setTextureRect(sf::IntRect(x, mapa[9][i]->getImageY(), mapa[9][i]->getImageWidth(), mapa[9][i]->getImageHeight()));
		mapa[9][i]->setType(TileTypes::SOLID);
	}*/
	/* END MAP TESTING */

	window->setView(mainView);
}

void Game::loop(){
	const int TICKS_SECOND = 25;
	const int TICKS_SKIP = 1000 / TICKS_SECOND;
	const int MAX_FRAMESKIP = 5;
	sf::Clock clock;
	sf::Int32 ticks = sf::Time(clock.getElapsedTime()).asMilliseconds();

	int loops;
	//float interpolation; //needed for the draw method

	while (window->isOpen())
	{
		Event event;
		while (window->pollEvent(event))
		{
			if (event.type == Event::Closed)
				window->close();
		}
		window->clear();
		loops = 0;
		while (sf::Time(clock.getElapsedTime()).asMilliseconds() > ticks && loops < MAX_FRAMESKIP) {
			update();
			ticks += TICKS_SKIP;
			loops++;
		}
		map->drawBackground(window, *this->mManager->getTexture(map->getId()));
		window->draw(*subjectA);
		window->display();
	}
}

void Game::update(){
	inputHandler::update();
	updateTick = GetTickCount();
	nextMoveX = nextMoveY = 0;

	//nextMoveX = linearMovement(subjectA->getSpeed().x, updateTick);
	//nextMoveY = linearMovement(subjectA->getSpeed().y, updateTick);
	nextMoveX = subjectA->getSpeed().x / 10;
	nextMoveY = subjectA->getSpeed().y / 100;
	Command * cmd;
	cmd = new MoveCommand(subjectA, nextMoveX, nextMoveY);
	Camera::centerCamera(subjectA, &mainView, WINDOW_WIDTH, WINDOW_HEIGHT, map->getSize().x * 16, map->getSize().y * 16);
	window->setView(mainView);
	cmd->execute();
	delete cmd;

	if (inputHandler::checkKey(sf::Keyboard::Up).first && !subjectA->isJumping() && !jumpKeyDown)
	{
		subjectA->setJumping(true);
		jumpKeyDown = true;
		moveRequest = true;
		subjectA->addSpeed(0, -subjectA->getJumpStartSpeed());
		//nextMoveY -= 5;
	}

	if (!inputHandler::checkKey(sf::Keyboard::Up).first)
	{
		jumpKeyDown = false;
	}

	/*if (inputHandler::checkKey(sf::Keyboard::Down).first)
	{
		subjectA->addSpeed(subjectA->getAccX(), 0);
		nextMoveY += 5;
	}*/

	if (inputHandler::checkKey(sf::Keyboard::Left).first)
	{
		subjectA->addSpeed(-subjectA->getAccX(),0);
		moveRequest = true;
		//nextMoveX -= 5;
	}
	if (inputHandler::checkKey(sf::Keyboard::Right).first)
	{
		subjectA->addSpeed(subjectA->getAccX(), 0);
		moveRequest = true;
		//nextMoveX += 5;
	}

	checkMovement();
	checkCollisions();

	//cout << nextMoveX << endl;
	int x = subjectA->getImageX() + subjectA->getImageWidth()*subjectA->getMaxFrame()*subjectA->getAnimSet();
	subjectA->setTextureRect(sf::IntRect(x, subjectA->getImageY(), subjectA->getImageWidth(), subjectA->getImageHeight()));

	/* TEST MAP DRAW */
	/*for (size_t i = 0; i < 10; i++){
	for (size_t j = 0; j < 10; j++){
	window->draw(*mapa[i][j]);
	}
	}*/
	/* TEST MAP DRAW END */

	subjectA->nextFrame();

	moveRequest = false;
	lastUpdateTime = updateTick;
}

void Game::checkMovement()
{
	// Limit the sideways acceleration of the player
	if (subjectA->getSpeed().x > subjectA->getMaxSpeed().x) subjectA->setSpeed(subjectA->getMaxSpeed().x,subjectA->getSpeed().y);
	if (subjectA->getSpeed().x < -subjectA->getMaxSpeed().x) subjectA->setSpeed(-subjectA->getMaxSpeed().x, subjectA->getSpeed().y); // Limit the force of gravity (terminal velocity) if (speedY > maxSpeedY) speedY = maxSpeedY;
	if (subjectA->getSpeed().y < -subjectA->getMaxSpeed().y) subjectA->setSpeed(subjectA->getSpeed().x, -subjectA->getMaxSpeed().y);

	// Apply the force of gravity
	subjectA->addSpeed(0, subjectA->getAccY());

	if (!moveRequest)
	{
		if (subjectA->getSpeed().x < 0) subjectA->addSpeed(subjectA->getDecX(), 0);
		if (subjectA->getSpeed().x > 0) subjectA->addSpeed(-subjectA->getDecX(), 0);

		// Deceleration may produce a speed that is greater than zero but
		// smaller than the smallest unit of deceleration. These lines ensure
		// that the player does not keep travelling at slow speed forever after
		// decelerating.
		if (subjectA->getSpeed().x > 0 && subjectA->getSpeed().x < subjectA->getDecX()) subjectA->setSpeed(0, subjectA->getSpeed().y);
		if (subjectA->getSpeed().x < 0 && subjectA->getSpeed().x > -subjectA->getDecX()) subjectA->setSpeed(0, subjectA->getSpeed().y);
	}
}

void Game::checkCollisions()
{
	float projectedMoveX, projectedMoveY, originalMoveX, originalMoveY;

	// Store the original final expected movement of the player so we can
	// see if it has been modified due to a collision or potential collision later
	
	subjectA->setContactX(true); subjectA->setContactYTop(true); subjectA->setContactYBottom(true);

	for (int iteration = 0; iteration < iterations && (subjectA->hasContactX() || subjectA->hasContactYTop() || subjectA->hasContactYBottom()); iteration++)
	{
		nextMoveX = subjectA->getSpeed().x / 10;
		nextMoveY = subjectA->getSpeed().y / 100;

		originalMoveX = nextMoveX;
		originalMoveY = nextMoveY;

		subjectA->setContactX(false); subjectA->setContactYTop(false); subjectA->setContactYBottom(false);

		// Iterate over each object whose bounding box intersects with the player's bounding box
		// until a collision is found
		int noOfTiles = map->getSize().x*map->getSize().y;
		for (int o = 0; o < noOfTiles && !subjectA->hasContactX() && !subjectA->hasContactYTop() && !subjectA->hasContactYBottom(); o++)
		{
			// We will test the four possible directions of player movement individually
			// dir: 0 = top, 1 = bottom, 2 = left, 3 = right
			for (int dir = 0; dir < 4; dir++) {     // Skip the test if the expected direction of movement makes the test irrelevant
				if (dir == 1 && nextMoveY < 0) continue;     if (dir == 2 && nextMoveX > 0) continue;
				if (dir == 0 && nextMoveY > 0) continue; // For example, we only want to test the top of the player's head when movement is upwards, not downwards. This is really important! If we don't do this, we can     // make corrections in the wrong direction, causing the player to magically jump, up to platforms or stick to ceilings.
				if (dir == 3 && nextMoveX < 0) continue;
				projectedMoveX = (dir >= 2 ? nextMoveX : 0);
				projectedMoveY = (dir < 2 ? nextMoveY : 0);

				while ((map->getTileNumber(o)->containsPoint(subjectA->col_points[dir * 2].first + subjectA->getPosition().x + projectedMoveX,
															 subjectA->col_points[dir * 2].second + subjectA->getPosition().y + projectedMoveY)
					|| map->getTileNumber(o)->containsPoint(subjectA->col_points[dir * 2 + 1].first + subjectA->getPosition().x + projectedMoveX,
															subjectA->col_points[dir * 2 + 1].second + subjectA->getPosition().y + projectedMoveY))
					&& (map->getPassabilityAt(map->getTileNumber(o)->getId()) != 0))
				{
					if (dir == 0) { projectedMoveY++; /*cout << "top established contact" << endl;*/ }
					if (dir == 1) { projectedMoveY--; /*cout << "bottom established contact" << endl;*/ }
					if (dir == 2) { projectedMoveX++; /*cout << "left established contact" << endl;*/ }
					if (dir == 3) { projectedMoveX--; /*cout << "right established contact" << endl;*/ }
				}
				if (dir >= 2 && dir <= 3) nextMoveX = projectedMoveX;
				if (dir >= 0 && dir <= 1) nextMoveY = projectedMoveY;
			}

			// Detect what type of contact has occurred based on a comparison of
			// the original expected movement vector and the new one
			if (nextMoveY > originalMoveY && originalMoveY < 0)
			{
				//cout << "top contact" << endl;
				subjectA->setContactYTop(true);
			}

			if (nextMoveY < originalMoveY && originalMoveY > 0)
			{
				//cout << "bottom contact" << endl;
				subjectA->setContactYBottom(true);
			}

			if (abs(nextMoveX - originalMoveX) > 0.01f)
			{
				//cout << "X contact" << endl;
				subjectA->setContactX(true);
			}

			// The player can't continue jumping if we hit the side of something, must fall instead
			// Without this, a player hitting a wall during a jump will continue trying to travel
			// upwards
			if (subjectA->hasContactX() && subjectA->hasContactYTop() && subjectA->getSpeed().y < 0)
			{
				subjectA->setSpeed(subjectA->getSpeed().x, 0);
				nextMoveY = 0;
			}
		}

		// If a contact has been detected, apply the re-calculated movement vector
		// and disable any further movement this frame (in either X or Y as appropriate)
		if (subjectA->hasContactYBottom() || subjectA->hasContactYTop())
		{
			subjectA->setSpeed(subjectA->getSpeed().x, 0);
			Command * cmd = new MoveCommand(subjectA, 0, nextMoveY);
			cmd->execute();
			delete cmd;
			if (subjectA->hasContactYBottom())
				subjectA->setJumping(false);
		}

		if (subjectA->hasContactX())
		{
			Command * cmd = new MoveCommand(subjectA, nextMoveX, 0);
			cmd->execute();
			subjectA->setSpeed(0, subjectA->getSpeed().y);
			delete cmd;
		}
	}
}

float Game::linearMovement(float pixelsPerSecond, DWORD tickCount)
{
	if (!tickCount)
		tickCount = GetTickCount();

	float secs_elapsed = (tickCount - lastUpdateTime) / 1000.0f;
	return secs_elapsed * pixelsPerSecond;
}