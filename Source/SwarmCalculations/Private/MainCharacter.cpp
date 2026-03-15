#include "MainCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"

AMainCharacter::AMainCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // ----------------------------- 创建弹簧臂 -----------------------------
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent);

    // 上帝视角通常不希望弹簧臂跟随控制器旋转，我们手动设置固定角度
    SpringArm->bUsePawnControlRotation = false;
    SpringArm->TargetArmLength = 5000.0f; // 相机距离角色的距离
    SpringArm->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f)); // 俯视角度（可调整）
    SpringArm->bInheritPitch = false; // 不继承控制器的俯仰
    SpringArm->bInheritRoll = false; // 不继承控制器的横滚
    SpringArm->bInheritYaw = false; // 不继承控制器的偏航（相机固定在世界空间）

    // ----------------------------- 创建摄像机 -----------------------------
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

    // ----------------------------- 角色移动设置 -----------------------------
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    MoveComp->SetMovementMode(MOVE_Flying); // 启用飞行模式
    MoveComp->MaxWalkSpeed = 1000.0f;
    MoveComp->MaxFlySpeed = 1000.0f; // 设置飞行速度
    MoveComp->BrakingDecelerationFlying = 5000.0f; // 设置飞行刹车减速度
    MoveComp->JumpZVelocity = 0.0f; // 可保留为0
    MoveComp->AirControl = 1.0f; // 飞行时建议开启空气控制
    MoveComp->bOrientRotationToMovement = true;

    // 默认边界
    MinBounds = FVector(-50000.0f, -50000.0f, 0.0f);
    MaxBounds = FVector(50000.0f, 50000.0f, 50000.0f);
}

void AMainCharacter::Move(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();
    if (Controller)
    {
        // 获取控制器旋转
        FRotator ControlRotation = Controller->GetControlRotation();
        // 只保留偏航角，构建一个仅水平旋转的旋转量
        FRotator YawRotation(0, ControlRotation.Yaw, 0);

        // 计算水平前方向（X轴）和水平右方向（Y轴）
        FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        // 应用移动
        AddMovementInput(ForwardDirection, MovementVector.Y);
        AddMovementInput(RightDirection, MovementVector.X);
    }
}

void AMainCharacter::MoveUpDown(const FInputActionValue& Value)
{
    float UpDownValue = Value.Get<float>();
    if (Controller != nullptr)
    {
        AddMovementInput(FVector::UpVector, UpDownValue);
    }
}


void AMainCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    ClampPositionToBounds();
}

void AMainCharacter::BeginPlay()
{
    Super::BeginPlay();
    ClampPositionToBounds();
}


void AMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // ----------------------------- 添加增强输入映射上下文 -----------------------------
    // 获取本地玩家子系统并添加上下文（必须在绑定输入之前完成）
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            if (DefaultMappingContext)
            {
                // 添加默认映射上下文，优先级设为0（可根据需要调整）
                Subsystem->AddMappingContext(DefaultMappingContext, 0);
            }
        }
    }

    // ----------------------------- 绑定输入动作 -----------------------------
    // 将输入组件转换为增强输入组件
    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // 绑定移动动作到 Move 函数，触发时机为持续触发（Triggered）
        if (MoveAction)
        {
            EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMainCharacter::Move);
        }
        // 垂直移动绑定
        if (UpDownAction)
        {
            EnhancedInput->BindAction(UpDownAction, ETriggerEvent::Triggered, this, &AMainCharacter::MoveUpDown);
        }
    }
}

void AMainCharacter::ClampPositionToBounds()
{
    FVector CurrentLocation = GetActorLocation();
    FVector ClampedLocation;
    ClampedLocation.X = FMath::Clamp(CurrentLocation.X, MinBounds.X, MaxBounds.X);
    ClampedLocation.Y = FMath::Clamp(CurrentLocation.Y, MinBounds.Y, MaxBounds.Y);
    ClampedLocation.Z = FMath::Clamp(CurrentLocation.Z, MinBounds.Z, MaxBounds.Z);

    if (!ClampedLocation.Equals(CurrentLocation))
    {
        SetActorLocation(ClampedLocation);
    }
}
