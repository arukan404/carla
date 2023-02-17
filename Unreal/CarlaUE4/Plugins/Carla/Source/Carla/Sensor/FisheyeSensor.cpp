// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla.h"
#include "Carla/Sensor/FisheyeSensor.h"

#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla/Game/CarlaEpisode.h"

#include "Engine/TextureRenderTargetCube.h"
#include "Components/SceneCaptureComponentCube.h"

#include "CoreGlobals.h"
#include "Engine/TextureRenderTargetCube.h"
#include "Runtime/ImageWriteQueue/Public/ImagePixelData.h"
#include "CubemapUnwrapUtils.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/Buffer.h>
#include <carla/sensor/SensorRegistry.h>
#include <compiler/enable-ue4-macros.h>


static auto SCENE_CAPTURE_COUNTER_CUBE = 0u;

AFisheyeSensor::AFisheyeSensor(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = true;
  PrimaryActorTick.TickGroup = TG_PrePhysics;
  
  CaptureRenderTarget = CreateDefaultSubobject<UTextureRenderTargetCube>(FName(*FString::Printf(TEXT("CaptureRenderTarget_d%d"), SCENE_CAPTURE_COUNTER_CUBE)));
  CaptureRenderTarget->CompressionSettings = TextureCompressionSettings::TC_Default;
  CaptureRenderTarget->SRGB = false;  

  Fisheye = CreateDefaultSubobject<USceneCaptureComponentCube>(
      FName(*FString::Printf(TEXT("SceneCaptureComponentCube_%d"), SCENE_CAPTURE_COUNTER_CUBE)));
  Fisheye-> bCaptureRotation = true;
  Fisheye->SetupAttachment(RootComponent);
  Fisheye->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
  ++SCENE_CAPTURE_COUNTER_CUBE;
}

FActorDefinition AFisheyeSensor::GetSensorDefinition()
{
  auto Definition = UActorBlueprintFunctionLibrary::MakeGenericSensorDefinition(
      TEXT("camera"),
      TEXT("fisheye"));

  FActorVariation XSize;
  XSize.Id = TEXT("image_size_x");
  XSize.Type = EActorAttributeType::Float;
  XSize.RecommendedValues = { TEXT("1280.0") };
  XSize.bRestrictToRecommended = false;

  FActorVariation YSize;
  YSize.Id = TEXT("image_size_y");
  YSize.Type = EActorAttributeType::Float;
  YSize.RecommendedValues = { TEXT("720.0") };
  YSize.bRestrictToRecommended = false;

  FActorVariation MaxAngle;
  MaxAngle.Id = TEXT("fov");
  MaxAngle.Type = EActorAttributeType::Float;
  MaxAngle.RecommendedValues = { TEXT("210.0") };
  MaxAngle.bRestrictToRecommended = false;

  FActorVariation Fx;
  Fx.Id = TEXT("f_x");
  Fx.Type = EActorAttributeType::Float;
  Fx.RecommendedValues = { TEXT("320.0") };
  Fx.bRestrictToRecommended = false;

  FActorVariation Fy;
  Fy.Id = TEXT("f_y");
  Fy.Type = EActorAttributeType::Float;
  Fy.RecommendedValues = { TEXT("320.0") };
  Fy.bRestrictToRecommended = false;

  FActorVariation Cx;
  Cx.Id = TEXT("c_x");
  Cx.Type = EActorAttributeType::Float;
  Cx.RecommendedValues = { TEXT("640.0") };
  Cx.bRestrictToRecommended = false;

  FActorVariation Cy;
  Cy.Id = TEXT("c_y");
  Cy.Type = EActorAttributeType::Float;
  Cy.RecommendedValues = { TEXT("360.0") };
  Cy.bRestrictToRecommended = false;

  FActorVariation D1;
  D1.Id = TEXT("d_1");
  D1.Type = EActorAttributeType::Float;
  D1.RecommendedValues = { TEXT("0.08309221636708493") };
  D1.bRestrictToRecommended = false;

  FActorVariation D2;
  D2.Id = TEXT("d_2");
  D2.Type = EActorAttributeType::Float;
  D2.RecommendedValues = { TEXT("0.01112126630599195") };
  D2.bRestrictToRecommended = false;

  FActorVariation D3;
  D3.Id = TEXT("d_3");
  D3.Type = EActorAttributeType::Float;
  D3.RecommendedValues = { TEXT("-0.008587261043925865") };
  D3.bRestrictToRecommended = false;

  FActorVariation D4;
  D4.Id = TEXT("d_4");
  D4.Type = EActorAttributeType::Float;
  D4.RecommendedValues = { TEXT("0.0008542188930970716") };
  D4.bRestrictToRecommended = false;

  Definition.Variations.Append({ XSize, YSize, MaxAngle, Fx, Fy, Cx, Cy, D1, D2, D3, D4});

  return Definition;
}

void AFisheyeSensor::Set(const FActorDescription &Description)
{
  Super::Set(Description);

  XSize = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "image_size_x",
      Description.Variations,
      1280.0f);

  YSize = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "image_size_y",
      Description.Variations,
      720.0f);
  
  MaxAngle = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "fov",
      Description.Variations,
      210.0f);

  Fx = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "f_x",
      Description.Variations,
      320.0f);

  Fy = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "f_y",
      Description.Variations,
      320.0f);

  Cx = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "c_x",
      Description.Variations,
      640.0f);

  Cy = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "c_y",
      Description.Variations,
      360.0f);

  D1 = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "d_1",
      Description.Variations,
      0.08309221636708493f);

  D2 = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "d_2",
      Description.Variations,
      0.01112126630599195f);

  D3 = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "d_3",
      Description.Variations,
      -0.008587261043925865f);

  D4 = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "d_4",
      Description.Variations,
      0.0008542188930970716f);

  CaptureRenderTarget -> SizeX = XSize;
}

float AFisheyeSensor::GetImageWidth() const
{
  check(CaptureRenderTarget != nullptr);
  return XSize;
}

float AFisheyeSensor::GetImageHeight() const
{
  check(CaptureRenderTarget != nullptr);
  return YSize;
}

float AFisheyeSensor::GetFOV() const
{
  check(CaptureRenderTarget != nullptr);
  return MaxAngle;
}

template <typename TSensor>
void AFisheyeSensor::SendPixelsInRenderThread(TSensor &Sensor, float MaxAngle, float SizeX, float SizeY, float Fx, float Fy, float Cx, float Cy, float D1, float D2, float D3, float D4)
{
  check(Sensor.CaptureRenderTarget != nullptr);

  UTextureRenderTargetCube *RenderTarget = Sensor.CaptureRenderTarget;

  TArray64<uint8> RawData;

  FIntPoint Size;

  EPixelFormat Format;  

  CubemapHelpersFisheye::FFisheyeParams FisheyeParams;

  FisheyeParams.ImageSize = FIntPoint(SizeX, SizeY);

  FisheyeParams.CameraMatrix = FVector4(Fx, Fy, Cx, Cy);

  FisheyeParams.DistortionCoeffs = FVector4(D1, D2, D3, D4);

  FisheyeParams.MaxAngle = MaxAngle;

  bool bUnwrapSuccess = CubemapHelpersFisheye::GenerateLongLatUnwrapFisheye(RenderTarget, RawData, FisheyeParams, Size, Format);
  // bool bUnwrapSuccess = CubemapHelpersFisheye::GenerateLongLatUnwrapFisheye(RenderTarget, RawData, Size, Format);

  auto Stream = Sensor.GetDataStream(Sensor);

  if (!Sensor.IsPendingKill())
  {
    auto Buffer = Stream.PopBufferFromPool();
    Buffer.copy_from(carla::sensor::SensorRegistry::get<TSensor *>::type::header_offset, RawData.GetData(), RawData.Num());
    Stream.Send(Sensor, std::move(Buffer));
  }
}

void AFisheyeSensor::BeginPlay()
{
   
  CaptureRenderTarget-> Init(GetImageWidth(), PF_B8G8R8A8);

  Fisheye->Deactivate();
  Fisheye->TextureTarget = CaptureRenderTarget;
  Fisheye->CaptureScene();

  Fisheye->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR; 
  // Fisheye->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDR; 

  Fisheye->UpdateContent();
  Fisheye->Activate();

  UKismetSystemLibrary::ExecuteConsoleCommand(
      GetWorld(),
      FString("g.TimeoutForBlockOnRenderFence 300000"));

  GetEpisode().GetWeather()->NotifyWeather();

  Super::BeginPlay();
}

void AFisheyeSensor::PrePhysTick(float DeltaTime)
{
  Super::PrePhysTick(DeltaTime);
  IStreamingManager::Get().AddViewInformation(
      Fisheye->GetComponentLocation(),
      XSize,
      YSize);
  Fisheye->UpdateContent();
  // std::cout << "Size X: " << XSize << std::endl;
  // std::cout << "Size Y: " << YSize << std::endl;
  SendPixelsInRenderThread(*this, MaxAngle, XSize, YSize, Fx, Fy, Cx, Cy, D1, D2, D3, D4);

  ReadyToCapture = true;
}

void AFisheyeSensor::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaTime)
{
  Super::PostPhysTick(World, TickType, DeltaTime);
  ReadyToCapture = true;
}

void AFisheyeSensor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  Super::EndPlay(EndPlayReason);
  SCENE_CAPTURE_COUNTER_CUBE = 0u;
}