#include "HeroBotCharacter.h"

#include "HeroAIController.h"

AHeroBotCharacter::AHeroBotCharacter()
{
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
    AIControllerClass = AHeroAIController::StaticClass();
}

EHeroBotRole AHeroBotCharacter::GetBotRole() const
{
    return BotRole;
}
