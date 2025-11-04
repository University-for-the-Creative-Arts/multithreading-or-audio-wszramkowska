#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ImageProcessor.generated.h"

UCLASS()
class PARALLELPROCESSING_API AImageProcessor : public AActor
{
	GENERATED_BODY()
	
public:
	AImageProcessor();

protected:
	virtual void BeginPlay() override;

private:
	void ProcessImage(const FString& ImagePath);

};
