#include <algorithm>
#include <functional>
#include <iostream>
#include <fstream>
#include <locale>
#include <sstream>

#include "Asteroid.h"
#include "Asteroids.h"
#include "Animation.h"
#include "AnimationManager.h"
#include "GameUtil.h"
#include "GameWindow.h"
#include "GameWorld.h"
#include "GameDisplay.h"
#include "Spaceship.h"
#include "BoundingShape.h"
#include "BoundingSphere.h"
#include "GUILabel.h"
#include "Explosion.h"
#include "Alien.h"

// PUBLIC INSTANCE CONSTRUCTORS ///////////////////////////////////////////////

/** Constructor. Takes arguments from command line, just in case. */
Asteroids::Asteroids(int argc, char *argv[])
	: GameSession(argc, argv)
{
	mLevel = 0;
	mAsteroidCount = 0;
	mAlienCount = 0;
}

/** Destructor. */
Asteroids::~Asteroids(void)
{
}

// PUBLIC INSTANCE METHODS ////////////////////////////////////////////////////

/** Start an asteroids game. */
void Asteroids::Start()
{
	// Create a shared pointer for the Asteroids game object - DO NOT REMOVE
	shared_ptr<Asteroids> thisPtr = shared_ptr<Asteroids>(this);

	// Add this class as a listener of the game world
	mGameWorld->AddListener(thisPtr.get());

	// Add this as a listener to the world and the keyboard
	mGameWindow->AddKeyboardListener(thisPtr);

	// Add a score keeper to the game world
	mGameWorld->AddListener(&mScoreKeeper);

	// Add this class as a listener of the score keeper
	mScoreKeeper.AddListener(thisPtr);

	// Create an ambient light to show sprite textures
	GLfloat ambient_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat diffuse_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient_light);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse_light);
	glEnable(GL_LIGHT0);

	Animation *explosion_anim = AnimationManager::GetInstance().CreateAnimationFromFile("explosion", 64, 1024, 64, 64, "explosion_fs.png");
	Animation *asteroid1_anim = AnimationManager::GetInstance().CreateAnimationFromFile("asteroid1", 128, 8192, 128, 128, "asteroid1_fs.png");
	Animation *spaceship_anim = AnimationManager::GetInstance().CreateAnimationFromFile("spaceship", 128, 128, 128, 128, "spaceship_fs.png");
	Animation *alien_anim = AnimationManager::GetInstance().CreateAnimationFromFile("alien", 128, 128, 128, 128, "alien_fs.png");

	//Create the GUI
	CreateGUI();

	// Add a player (watcher) to the game world
	mGameWorld->AddListener(&mPlayer);

	// Add this class as a listener of the player
	mPlayer.AddListener(thisPtr);

	// Start the game
	GameSession::Start();
}

/** Stop the current game. */
void Asteroids::Stop()
{
	// Stop the game
	GameSession::Stop();
}

// PUBLIC INSTANCE METHODS IMPLEMENTING IKeyboardListener /////////////////////

void Asteroids::OnKeyPressed(uchar key, int x, int y)
{
	if (game_over && !name_entered)
	{
		if (mName.size() == 3)
		{
			name_entered = true;
		}
		if (mName.size() < 3 && key != '\r')
		{
			std::ostringstream msg_stream;
			msg_stream << key;
			OnInputReceived(msg_stream.str());
		}
		else if (key != '\r')
		{
			mUserInputLimitLabel->SetVisible(true); 
		}
	}
	switch (key)
	{
	case ' ':
		if (!started && !game_over) {
			// Hide press space label
			mStartLabel->SetVisible(false);
			// Show score label
			mScoreLabel->SetVisible(true);
			// Show lives label
			mLivesLabel->SetVisible(true);
			// Set boolean to started so we know the user has ok'd the start
			started = true;
			// Create a spaceship and add it to the world
			mGameWorld->AddObject(CreateSpaceship());
			// Create some asteroids and add them to the world
			CreateAsteroids(10);
			// Create some aliens and add them to the world
			CreateAliens(1);
		} else {
			mSpaceship->Shoot();
		}
		break;
	case 'r':
		if (game_over && name_entered) {
			// Save the current score to the file
			SaveScoreToFile(mName, mScoreKeeper.GetScore());
			// Remove asteroids left over
			RemoveAsteroids();
			// Hide game over label
			mGameOverLabel->SetVisible(false);
			// Hide final score label
			mFinalScoreLabel->SetVisible(false);
			// Hide restart label
			mRestartLabel->SetVisible(false);
			// Hide top scores label
			for (int i = 0; i < mHighScoresLabel.size(); ++i)
			{
				mHighScoresLabel[i]->SetVisible(false);
			}
			mUserInputLabel->SetVisible(false);
			mUserInputLimitLabel->SetVisible(false);
			// Reset score
			mScoreKeeper.ResetScore();
			// Reset score label
			mScoreKeeper.FireScoreChanged();
			// Reset lives
			mPlayer.ResetLives();
			// Reset lives label
			mPlayer.FirePlayerKilled();
			// Reset levels
			mLevel = 0;
			// Reset asteroids
			mAsteroidCount = 0;
			// Reset aliens
			mAlienCount = 0;
			// Reset name
			mName = "";
			mUserInputLabel->SetText("Name: ");
			// Set boolean to started so we know the user has ok'd the start
			started = true;
			// Set boolean to false so we know the users has restarted
			game_over = false;
			name_entered = false;
			// Create a spaceship and add it to the world
			SetTimer(0, CREATE_NEW_PLAYER);
			// Create some asteroids and add them to the world
			CreateAsteroids(10);
			// Create some aliens and add them to the world
			CreateAliens(1);
		}
		else if (!game_over)
		{
			mRespawnLabel->SetVisible(false);
			respawn = true;
			SetTimer(0, CREATE_NEW_PLAYER);
		}
		break;
	// Delete last character entered
	case 8:
		if (game_over && mName.size() > 0)
		{
			mName.erase(mName.size() - 1, 1);
			mUserInputLimitLabel->SetVisible(false);
		}
		break;
	default:
		break;
	}
}

void Asteroids::OnKeyReleased(uchar key, int x, int y) {}

void Asteroids::OnSpecialKeyPressed(int key, int x, int y)
{
	if (started) {
		switch (key)
		{
			// If up arrow key is pressed start applying forward thrust
		case GLUT_KEY_UP: mSpaceship->Thrust(10); break;
			// If up dowm key is pressed start applying backward thrust
		case GLUT_KEY_DOWN: mSpaceship->Thrust(-10); break;
			// If left arrow key is pressed start rotating anti-clockwise
		case GLUT_KEY_LEFT: mSpaceship->Rotate(90); break;
			// If right arrow key is pressed start rotating clockwise
		case GLUT_KEY_RIGHT: mSpaceship->Rotate(-90); break;
			// Default case - do nothing
		default: break;
		}
	}
}

void Asteroids::OnSpecialKeyReleased(int key, int x, int y)
{
	if (started) {
		switch (key)
		{
			// If up arrow key is released stop applying forward thrust
		case GLUT_KEY_UP: mSpaceship->Thrust(0); break;
			// If up dowm key is released stop applying backward thrust
		case GLUT_KEY_DOWN: mSpaceship->Thrust(0); break;
			// If left arrow key is released stop rotating
		case GLUT_KEY_LEFT: mSpaceship->Rotate(0); break;
			// If right arrow key is released stop rotating
		case GLUT_KEY_RIGHT: mSpaceship->Rotate(0); break;
			// Default case - do nothing
		default: break;
		}
	}
}


// PUBLIC INSTANCE METHODS IMPLEMENTING IGameWorldListener ////////////////////

void Asteroids::OnObjectRemoved(GameWorld* world, shared_ptr<GameObject> object)
{
	if (object->GetType() == GameObjectType("Asteroid"))
	{
		shared_ptr<GameObject> explosion = CreateExplosion();
		explosion->SetPosition(object->GetPosition());
		explosion->SetRotation(object->GetRotation());
		mGameWorld->AddObject(explosion);
		mAsteroidCount--;
		if (mAsteroidCount <= 0 && mAlienCount <= 0)
		{
			SetTimer(500, START_NEXT_LEVEL);
		}
	}
	if (object->GetType() == GameObjectType("Alien"))
	{
		shared_ptr<GameObject> explosion = CreateExplosion();
		explosion->SetPosition(object->GetPosition());
		explosion->SetRotation(object->GetRotation());
		mGameWorld->AddObject(explosion);
		mAlienCount--;
		if (mAsteroidCount <= 0 && mAlienCount <= 0)
		{
			SetTimer(500, START_NEXT_LEVEL);
		}
	}
}

// PUBLIC INSTANCE METHODS IMPLEMENTING ITimerListener ////////////////////////

void Asteroids::OnTimer(int value)
{
	if (value == CREATE_NEW_PLAYER)
	{
		mSpaceship->Reset();
		
		mGameWorld->AddObject(mSpaceship);
	}

	if (value == START_NEXT_LEVEL)
	{
		if (mSpaceship == NULL)
		{
			mSpaceship->Reset();
			mGameWorld->AddObject(mSpaceship);
		}
		mLevel++;
		int num_asteroids = 10 + 2 * mLevel;
		int num_aliens = 1 + mLevel;
		CreateAsteroids(num_asteroids);
		CreateAliens(num_aliens);
	}

	if (value == SHOW_GAME_OVER)
	{
		vector<string> high_scores = ReadScoreFile();
		// Add the last score to the current sessions vector
		//AddSessionScore(mScoreKeeper.GetScore());
		// Sort it in descending order
		//std::sort(high_scores.begin(), high_scores.end(), std::greater<int>());

		int limit = 0;
		// Loop through session scores to display
		/*for (std::vector<int>::const_iterator i = high_scores.begin(); i != high_scores.end(); ++i)
		{
			if (limit < 10)
			{
				msg_stream << *i << ' ';
				++limit;
			}
			else
			{
				break;
			}
		}*/
		// Create a new GUILabel for the 10 highest scores
		while (limit < high_scores.size() && limit < 10)
		{
			ostringstream convert;
			convert << high_scores[limit];
			string score = convert.str();
			// Create a new GUILabel and wrap it up in a shared_ptr
			shared_ptr<GUILabel> mTopScoreLabel = shared_ptr<GUILabel>(new GUILabel(convert.str()));
			mHighScoresLabel.push_back(mTopScoreLabel);

			// Add the GUILabel to the GUIContainer  
			shared_ptr<GUIComponent> top_score_component
				= static_pointer_cast<GUIComponent>(mTopScoreLabel);
			mGameDisplay->GetContainer()->AddComponent(top_score_component, GLVector2f(0.4f, 0.8f - (limit * 0.05f)));
			
			limit++;
		}
		mGameOverLabel->SetVisible(true);
		mFinalScoreLabel->SetVisible(true);
		mRestartLabel->SetVisible(true);
		mUserInputLabel->SetVisible(true);
		started = false;
		game_over = true;
	}

}

// PROTECTED INSTANCE METHODS /////////////////////////////////////////////////
shared_ptr<GameObject> Asteroids::CreateSpaceship()
{
	// Create a raw pointer to a spaceship that can be converted to
	// shared_ptrs of different types because GameWorld implements IRefCount
	mSpaceship = make_shared<Spaceship>();
	mSpaceship->SetBoundingShape(make_shared<BoundingSphere>(mSpaceship->GetThisPtr(), 4.0f));
	shared_ptr<Shape> bullet_shape = make_shared<Shape>("bullet.shape");
	mSpaceship->SetBulletShape(bullet_shape);
	Animation *anim_ptr = AnimationManager::GetInstance().GetAnimationByName("spaceship");
	shared_ptr<Sprite> spaceship_sprite =
		make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	mSpaceship->SetSprite(spaceship_sprite);
	mSpaceship->SetScale(0.1f);
	// Reset spaceship back to centre of the world
	mSpaceship->Reset();
	// Return the spaceship so it can be added to the world
	return mSpaceship;

}

void Asteroids::CreateAsteroids(const uint num_asteroids)
{
	mAsteroidCount = num_asteroids;
	for (uint i = 0; i < num_asteroids; i++)
	{
		Animation *anim_ptr = AnimationManager::GetInstance().GetAnimationByName("asteroid1");
		shared_ptr<Sprite> asteroid_sprite
			= make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
		asteroid_sprite->SetLoopAnimation(true);
		shared_ptr<GameObject> asteroid = make_shared<Asteroid>();
		asteroid->SetBoundingShape(make_shared<BoundingSphere>(asteroid->GetThisPtr(), 10.0f));
		asteroid->SetSprite(asteroid_sprite);
		asteroid->SetScale(0.2f);
		mGameWorld->AddObject(asteroid);
	}
}

void Asteroids::RemoveAsteroids()
{
	GameObjectList mGameObjects = mGameWorld->GetObjects();
	for each (shared_ptr<GameObject> object in mGameObjects)
	{
		object->RemoveFromWorld(object);
	}
}

void Asteroids::CreateAliens(const uint num_aliens)
{
	mAlienCount = num_aliens;
	for (uint i = 0; i < num_aliens; i++)
	{
		Animation *anim_ptr = AnimationManager::GetInstance().GetAnimationByName("alien");
		shared_ptr<Sprite> alien_sprite
			= make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
		alien_sprite->SetLoopAnimation(true);
		shared_ptr<GameObject> alien = make_shared<Alien>();
		alien->SetBoundingShape(make_shared<BoundingSphere>(alien->GetThisPtr(), 10.0f));
		alien->SetSprite(alien_sprite);
		alien->SetScale(0.2f);
		mGameWorld->AddObject(alien);
	}
}

void Asteroids::CreateGUI()
{
	// Add a (transparent) border around the edge of the game display
	mGameDisplay->GetContainer()->SetBorder(GLVector2i(10, 10));

	// Create a new GUILabel and wrap it up in a shared_ptr
	mScoreLabel = make_shared<GUILabel>("Score: 0");
	// Set the vertical alignment of the label to GUI_VALIGN_TOP
	mScoreLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
	// Set the visibility of the label to false (hidden)
	mScoreLabel->SetVisible(false);
	// Add the GUILabel to the GUIComponent  
	shared_ptr<GUIComponent> score_component
		= static_pointer_cast<GUIComponent>(mScoreLabel);
	mGameDisplay->GetContainer()->AddComponent(score_component, GLVector2f(0.0f, 1.0f));

	// Create a new GUILabel and wrap it up in a shared_ptr
	mLivesLabel = make_shared<GUILabel>("Lives: 3");
	// Set the vertical alignment of the label to GUI_VALIGN_BOTTOM
	mLivesLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_BOTTOM);
	// Set the visibility of the label to false (hidden)
	mLivesLabel->SetVisible(false);
	// Add the GUILabel to the GUIComponent  
	shared_ptr<GUIComponent> lives_component = static_pointer_cast<GUIComponent>(mLivesLabel);
	mGameDisplay->GetContainer()->AddComponent(lives_component, GLVector2f(0.0f, 0.0f));

	// Create a new GUILabel and wrap it up in a shared_ptr
	mGameOverLabel = shared_ptr<GUILabel>(new GUILabel("GAME OVER"));
	// Set the horizontal alignment of the label to GUI_HALIGN_CENTER
	mGameOverLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	// Set the vertical alignment of the label to GUI_VALIGN_TOP
	mGameOverLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
	// Set the visibility of the label to false (hidden)
	mGameOverLabel->SetVisible(false);
	// Add the GUILabel to the GUIContainer  
	shared_ptr<GUIComponent> game_over_component
		= static_pointer_cast<GUIComponent>(mGameOverLabel);
	mGameDisplay->GetContainer()->AddComponent(game_over_component, GLVector2f(0.5f, 1.0f));

	// Create a new GUILabel and wrap it up in a shared_ptr
	mStartLabel = shared_ptr<GUILabel>(new GUILabel("PRESS SPACE TO START"));
	// Set the horizontal alignment of the label to GUI_HALIGN_CENTER
	mStartLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	// Set the vertical alignment of the label to GUI_VALIGN_MIDDLE
	mStartLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	// Set the visibility of the label to false (hidden)
	mStartLabel->SetVisible(true);
	// Add the GUILabel to the GUIContainer  
	shared_ptr<GUIComponent> start_component
		= static_pointer_cast<GUIComponent>(mStartLabel);
	mGameDisplay->GetContainer()->AddComponent(start_component, GLVector2f(0.5f, 0.5f));

	// Create a new GUILabel and wrap it up in a shared_ptr
	mFinalScoreLabel = shared_ptr<GUILabel>(new GUILabel("Top Scores:"));
	// Set the horizontal alignment of the label to GUI_HALIGN_CENTER
	mFinalScoreLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	// Set the vertical alignment of the label to GUI_VALIGN_MIDDLE
	mFinalScoreLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
	// Set the visibility of the label to false (hidden)
	mFinalScoreLabel->SetVisible(false);
	// Add the GUILabel to the GUIContainer  
	shared_ptr<GUIComponent> final_score_component
		= static_pointer_cast<GUIComponent>(mFinalScoreLabel);
	mGameDisplay->GetContainer()->AddComponent(final_score_component, GLVector2f(0.5f, 0.9f));

	// Create a new GUILabel and wrap it up in a shared_ptr
	mRestartLabel = shared_ptr<GUILabel>(new GUILabel("PRESS R TO RESTART"));
	// Set the horizontal alignment of the label to GUI_HALIGN_CENTER
	mRestartLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	// Set the vertical alignment of the label to GUI_VALIGN_MIDDLE
	mRestartLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_BOTTOM);
	// Set the visibility of the label to false (hidden)
	mRestartLabel->SetVisible(false);
	// Add the GUILabel to the GUIContainer  
	shared_ptr<GUIComponent> restart_label_component
		= static_pointer_cast<GUIComponent>(mRestartLabel);
	mGameDisplay->GetContainer()->AddComponent(restart_label_component, GLVector2f(0.5f, 0.0f));

	// Create a new GUILabel and wrap it up in a shared_ptr
	mRespawnLabel = shared_ptr<GUILabel>(new GUILabel("PRESS R TO RESPAWN"));
	// Set the horizontal alignment of the label to GUI_HALIGN_CENTER
	mRespawnLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	// Set the vertical alignment of the label to GUI_VALIGN_MIDDLE
	mRespawnLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_BOTTOM);
	// Set the visibility of the label to false (hidden)
	mRespawnLabel->SetVisible(false);
	// Add the GUILabel to the GUIContainer  
	shared_ptr<GUIComponent> respawn_label_component
		= static_pointer_cast<GUIComponent>(mRespawnLabel);
	mGameDisplay->GetContainer()->AddComponent(respawn_label_component, GLVector2f(0.5f, 0.0f));

	// Create a new GUILabel and wrap it up in a shared_ptr
	mUserInputLabel = shared_ptr<GUILabel>(new GUILabel("Name: "));
	// Set the horizontal alignment of the label to GUI_HALIGN_CENTER
	mUserInputLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_LEFT);
	// Set the vertical alignment of the label to GUI_VALIGN_MIDDLE
	mUserInputLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	// Set the visibility of the label to false (hidden)
	mUserInputLabel->SetVisible(false);
	// Add the GUILabel to the GUIContainer  
	shared_ptr<GUIComponent> user_input_label_component
		= static_pointer_cast<GUIComponent>(mUserInputLabel);
	mGameDisplay->GetContainer()->AddComponent(user_input_label_component, GLVector2f(0.0f, 0.5f));

	// Create a new GUILabel and wrap it up in a shared_ptr
	mUserInputLimitLabel = shared_ptr<GUILabel>(new GUILabel("Limit of 3 letters"));
	// Set the horizontal alignment of the label to GUI_HALIGN_CENTER
	mUserInputLimitLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_LEFT);
	// Set the vertical alignment of the label to GUI_VALIGN_MIDDLE
	mUserInputLimitLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	// Set the visibility of the label to false (hidden)
	mUserInputLimitLabel->SetVisible(false);
	// Add the GUILabel to the GUIContainer  
	shared_ptr<GUIComponent> user_input__limit_label_component
		= static_pointer_cast<GUIComponent>(mUserInputLimitLabel);
	mGameDisplay->GetContainer()->AddComponent(user_input__limit_label_component, GLVector2f(0.0f, 0.4f));
}

void Asteroids::OnScoreChanged(int score)
{
	// Format the score message using an string-based stream
	std::ostringstream msg_stream;
	msg_stream << "Score: " << score;
	// Get the score message as a string
	std::string score_msg = msg_stream.str();
	mScoreLabel->SetText(score_msg);
}

void Asteroids::OnInputReceived(string letter)
{
	mName.append(letter);
	// Format the score message using an string-based stream
	std::ostringstream msg_stream;
	msg_stream << "Name: " << mName;
	// Get the score message as a string
	std::string name_msg = msg_stream.str();
	mUserInputLabel->SetText(name_msg);
}

void Asteroids::OnPlayerKilled(int lives_left)
{
	shared_ptr<GameObject> explosion = CreateExplosion();
	explosion->SetPosition(mSpaceship->GetPosition());
	explosion->SetRotation(mSpaceship->GetRotation());
	mGameWorld->AddObject(explosion);

	// Format the lives left message using an string-based stream
	std::ostringstream msg_stream;
	msg_stream << "Lives: " << lives_left;
	// Get the lives left message as a string
	std::string lives_msg = msg_stream.str();
	mLivesLabel->SetText(lives_msg);

	if (lives_left > 0) 
	{ 
		mRespawnLabel->SetVisible(true);
		if (respawn)
		{
			respawn = false;
		}
	}
	else
	{
		SetTimer(500, SHOW_GAME_OVER);
	}
}

// Add current score to sessions vector
void Asteroids::AddSessionScore(int score)
{
	session_scores.push_back(score);
}

// Save's the parameter to the scores file and append's a new line
void Asteroids::SaveScoreToFile(string name, int score)
{
	ofstream score_file;
	score_file.open("scores.txt", ios::out | ios::app);
	score_file << score << " " << name << "\n";
	score_file.close();
}

bool is_not_digit(char c)
{
	return !isdigit(c);
}

bool numeric_string_compare(const std::string& s1, const std::string& s2)
{
	// handle empty strings...

	std::string::const_iterator it1 = s1.begin(), it2 = s2.begin();

	if (isdigit(s1[0]) && isdigit(s2[0])) {
		int n1, n2;
		std::stringstream ss(s1);
		ss >> n1;
		ss.clear();
		ss.str(s2);
		ss >> n2;

		if (n1 != n2) return n1 > n2;

		it1 = std::find_if(s1.begin(), s1.end(), is_not_digit);
		it2 = std::find_if(s2.begin(), s2.end(), is_not_digit);
	}

	return std::lexicographical_compare(it1, s1.end(), it2, s2.end());
}

vector<string> Asteroids::ReadScoreFile()
{
	string line;
	ifstream score_file;
	score_file.open("scores.txt");
	vector<string> scores;

	while (std::getline(score_file, line))
	{
		scores.push_back(line);
	}
	score_file.close();
	std::sort(scores.begin(), scores.end(), numeric_string_compare);
	return scores;
}

shared_ptr<GameObject> Asteroids::CreateExplosion()
{
	Animation *anim_ptr = AnimationManager::GetInstance().GetAnimationByName("explosion");
	shared_ptr<Sprite> explosion_sprite =
		make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	explosion_sprite->SetLoopAnimation(false);
	shared_ptr<GameObject> explosion = make_shared<Explosion>();
	explosion->SetSprite(explosion_sprite);
	explosion->Reset();
	return explosion;
}