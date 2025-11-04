// Fill out your copyright notice in the Description page of Project Settings.


#include "ImageProcessor.h"
#include "Misc/FileHelper.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Modules/ModuleManager.h"
#include "Async/ParallelFor.h"
#include "HAL/PlatformProcess.h"

// Sets default values
AImageProcessor::AImageProcessor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AImageProcessor::BeginPlay()
{
	Super::BeginPlay();

	FString ImagePath = TEXT("C:\\Users\\2106125\\Documents\\pibbles.jpg");

	ProcessImage(ImagePath);
	
}

void AImageProcessor::ProcessImage(const FString& ImagePath)
{
	double StartTime = FPlatformTime::Seconds();

	// --- Load image file ---
	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *ImagePath))
	{
		UE_LOG(LogTemp, Error, TEXT("❌ Failed to load image: %s"), *ImagePath);
		return;
	}

	// --- Prepare Image Wrapper ---
	IImageWrapperModule& ImageWrapperModule =
		FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);

	if (!ImageWrapper->SetCompressed(FileData.GetData(), FileData.Num()))
	{
		UE_LOG(LogTemp, Error, TEXT("❌ Invalid or unsupported image format: %s"), *ImagePath);
		return;
	}

	// --- Decompress to raw RGBA data ---
	TArray64<uint8> RawData;
	if (!ImageWrapper->GetRaw(ERGBFormat::RGBA, 8, RawData))
	{
		UE_LOG(LogTemp, Error, TEXT("❌ Could not decompress image."));
		return;
	}

	int32 Width = ImageWrapper->GetWidth();
	int32 Height = ImageWrapper->GetHeight();
	const int32 NumPixels = Width * Height;
	const uint8* PixelPtr = RawData.GetData();

	// --- Parallel computation of red-channel sum ---
	std::atomic<uint64> RedSum(0);

	ParallelFor(NumPixels, [&](int32 Index)
	{
		int32 PixelIndex = Index * 4; // RGBA = 4 bytes per pixel
		uint8 R = PixelPtr[PixelIndex];
		RedSum.fetch_add(R, std::memory_order_relaxed);
	});

	double EndTime = FPlatformTime::Seconds();
	double ExecutionTime = EndTime - StartTime;

	UE_LOG(LogTemp, Display, TEXT("✅ Red Channel Sum: %llu"), RedSum.load());
	UE_LOG(LogTemp, Display, TEXT("⏱ Execution Time: %.6f seconds"), ExecutionTime);
}


