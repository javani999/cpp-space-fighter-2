#include "PlayerShip.h"
#include "Level.h"
#include "Ship.h"

const float LIFE_TEXTURE_SCALE = 0.25f;

void PlayerShip::LoadContent(ResourceManager& resourceManager)
{
    ConfineToScreen();
    SetResponsiveness(0.1);

    m_pTexture = resourceManager.Load<Texture>("Textures\\PlayerShip.png");

    if (m_pTexture == nullptr)
    {
        return;
    }

    AudioSample* pAudio = resourceManager.Load<AudioSample>("Audio\\Effects\\Laser.wav");
    if (pAudio != nullptr)
    {
        pAudio->SetVolume(0.5f);
        GetWeapon("Main Blaster")->SetFireSound(pAudio);
    }

    SetPosition(Game::GetScreenCenter() + Vector2::UNIT_Y * 300);

    // Initialize lives textures
    m_lifeTextures.clear();
    for (int i = 0; i < m_lives; i++)
    {
        Texture* lifeTexture = resourceManager.Load<Texture>("Textures\\Lives.png");
        if (lifeTexture != nullptr)
        {
            m_lifeTextures.push_back(lifeTexture);
        }
    }
}

void PlayerShip::AddLife()
{
    m_lives++;
    // Texture* lifeTexture = resourceManager.Load<Texture>("Textures\\Lives.png");
    // if (lifeTexture != nullptr)
    // {
    //     m_lifeTextures.push_back(lifeTexture);
    // }
}

void PlayerShip::OnCollisionWithEnemy()
{
    RemoveLife();
    if (m_lives == 0)
    {
        std::cout << "Game Over" << "\n";
    }
}

void PlayerShip::RemoveLife()
{
    if (m_lives > 0)
    {
        m_lives--;
        m_lifeTextures.pop_back();
    }
    if (m_lives == 0)
    {
        Deactivate();
    }
}

void PlayerShip::Initialize(Level* pLevel, const Vector2& startPosition, ResourceManager& resourceManager)
{
    SetPosition(startPosition);
    SetHitPoints(GetMaxHitPoints());
    m_lives = 3;
    LoadContent(resourceManager);
}


void PlayerShip::HandleInput(const InputState& input)
{
    if (IsActive())
    {
        Vector2 direction;
        if (input.IsKeyDown(Key::DOWN)) direction.Y++;
        if (input.IsKeyDown(Key::UP)) direction.Y--;
        if (input.IsKeyDown(Key::Right)) direction.X++;
        if (input.IsKeyDown(Key::Left)) direction.X--;

        // Normalize the direction
        if (direction.X != 0 && direction.Y != 0)
        {
            direction *= Math::NORMALIZE_PI_OVER4;
        }

        TriggerType type = TriggerType::None;
        if (input.IsKeyDown(Key::SPACE)) type |= TriggerType::Primary;

        SetDesiredDirection(direction);
        if (type != TriggerType::None) FireWeapons(type);
    }
}

void PlayerShip::Update(const GameTime& gameTime)
{
    Vector2 targetVelocity = m_desiredDirection * GetSpeed() * gameTime.GetElapsedTime();
    m_velocity = Vector2::Lerp(m_velocity, targetVelocity, GetResponsiveness());
    TranslatePosition(m_velocity);

    if (m_isConfinedToScreen)
    {
        const int PADDING = 4;
        const int TOP = PADDING;
        const int Left = PADDING;
        const int Right = Game::GetScreenWidth() - PADDING;
        const int BOTTOM = Game::GetScreenHeight() - PADDING;

        Vector2* pPosition = &GetPosition();
        if (pPosition->X - GetHalfDimensions().X < Left)
        {
            SetPosition(Left + GetHalfDimensions().X, pPosition->Y);
            m_velocity.X = 0;
        }
        if (pPosition->X + GetHalfDimensions().X > Right)
        {
            SetPosition(Right - GetHalfDimensions().X, pPosition->Y);
            m_velocity.X = 0;
        }
        if (pPosition->Y - GetHalfDimensions().Y < TOP)
        {
            SetPosition(pPosition->X, TOP + GetHalfDimensions().Y);
            m_velocity.Y = 0;
        }
        if (pPosition->Y + GetHalfDimensions().Y > BOTTOM)
        {
            SetPosition(pPosition->X, BOTTOM - GetHalfDimensions().Y);
            m_velocity.Y = 0;
        }
    }

    Ship::Update(gameTime);
}

void PlayerShip::Draw(SpriteBatch& spriteBatch)
{
    if (IsActive()) {
        if (m_pTexture != nullptr)
        {
            spriteBatch.Draw(m_pTexture, GetPosition(), Color::WHITE, m_pTexture->GetCenter());
        }
        int heartSpacing = 50;

        for (int i = 0; i < m_lives; i++) {
            Vector2 lifePosition = Vector2(10 + i * heartSpacing, 10);
            Vector2 lifeScale = Vector2(LIFE_TEXTURE_SCALE, LIFE_TEXTURE_SCALE);
            Texture* lifeTexture = m_lifeTextures[i];
            Region lifeRegion(0, 0, lifeTexture->GetWidth(), lifeTexture->GetHeight());
            spriteBatch.Draw(lifeTexture, lifePosition, lifeRegion, Color::WHITE, Vector2::ZERO, lifeScale);
        }
    }
}

void PlayerShip::Hit(float damage)
{
    float hitPoints = GetHitPoints();
    hitPoints -= damage;
    SetHitPoints(hitPoints);

    if (hitPoints > 0) return;

    m_lives--;
    if (m_lives <= 0)
    {
        GameObject::Deactivate();
        GetCurrentLevel()->SpawnExplosion(this);
    }
    else
    {
        Vector2 startPosition = Game::GetScreenCenter();
        Initialize(GetCurrentLevel(), startPosition, *GetCurrentLevel()->GetResourceManager());
        GameObject::Activate();
    }
}

Vector2 PlayerShip::GetHalfDimensions() const
{
    return m_pTexture->GetCenter();
}

void PlayerShip::SetResponsiveness(const float responsiveness)
{
    m_responsiveness = Math::Clamp(0, 1, responsiveness);
}