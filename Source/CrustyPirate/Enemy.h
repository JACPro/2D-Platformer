#pragma once

#include "CoreMinimal.h"
#include "PaperZDCharacter.h"

#include "Components/SphereComponent.h"
#include "PlayerCharacter.h"
#include "PaperZDAnimInstance.h"
#include "Components/TextRenderComponent.h"

#include "Engine/TimerHandle.h"

#include "Enemy.generated.h"

/**
 * 
 */
UCLASS()
class CRUSTYPIRATE_API AEnemy : public APaperZDCharacter
{
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	USphereComponent* PlayerDetectorSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	APlayerCharacter* FollowTarget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UPaperZDAnimSequence* AttackAnimSequence;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UTextRenderComponent* HPText;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StopDistanceToTarget = 70.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int HitPoints = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackCooldownInSeconds = 3.0f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool IsAlive = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool IsStunned = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool CanMove = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool CanAttack = true;
	
	FTimerHandle StunTimer;
	FTimerHandle AttackCooldownTimer;

	FZDOnAnimationOverrideEndSignature OnAttackOverrideEndDelegate;

	AEnemy();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void DetectorOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void DetectorOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	bool ShouldMoveToTarget();
	void UpdateDirection(float MoveDirection);

	void UpdateHP(int NewHP);
	void TakeDamage(int DamageAmount, float StunDuration);
	void Die();

	void Stun(float DurationInSeconds);
	void OnStunTimerTimeout();

	void Attack();
	void OnAttackCooldownTimerTimeout();
	void OnAttackOverrideAnimEnd(bool Completed);
};
