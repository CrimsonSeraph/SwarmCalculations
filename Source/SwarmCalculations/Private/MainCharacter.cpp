#include "MainCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"

AMainCharacter::AMainCharacter()
{
    // 本角色不需要每帧 Tick，关闭以提升性能
    PrimaryActorTick.bCanEverTick = false;

    // ----------------------------- 创建弹簧臂 -----------------------------
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent);

    // 上帝视角通常不希望弹簧臂跟随控制器旋转，我们手动设置固定角度
    SpringArm->bUsePawnControlRotation = false;
    SpringArm->TargetArmLength = 800.0f;                 // 相机距离角色的距离
    SpringArm->SetRelativeRotation(FRotator(-60.0f, 0.0f, 0.0f)); // 俯视角度（可调整）
    SpringArm->bInheritPitch = false;                    // 不继承控制器的俯仰
    SpringArm->bInheritRoll = false;                     // 不继承控制器的横滚
    SpringArm->bInheritYaw = false;                      // 不继承控制器的偏航（相机固定在世界空间）

    // ----------------------------- 创建摄像机 -----------------------------
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

    // ----------------------------- 角色移动设置 -----------------------------
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    MoveComp->MaxWalkSpeed = 600.0f;          // 移动速度
    MoveComp->JumpZVelocity = 0.0f;            // 禁止跳跃（上帝视角通常不需要）
    MoveComp->AirControl = 0.0f;                // 空中无法控制（防止跳跃后漂移）
    MoveComp->bOrientRotationToMovement = true; // 角色朝向移动方向（可选，使转身更自然）
}

void AMainCharacter::Move(const FInputActionValue& Value)
{
    // 获取输入二维向量（X = 左右，Y = 前后）
    FVector2D MovementVector = Value.Get<FVector2D>();

    // 确保角色已被控制
    if (Controller == nullptr)
        return;

    // 获取相机的朝向向量（忽略垂直分量，确保只在水平面移动）
    FVector CameraForward = Camera->GetForwardVector();
    FVector CameraRight = Camera->GetRightVector();

    // 将 Z 分量置零，实现水平面移动
    CameraForward.Z = 0.0f;
    CameraRight.Z = 0.0f;

    // 重新归一化，确保方向向量长度为 1
    CameraForward.Normalize();
    CameraRight.Normalize();

    // 根据输入值向相机的前方和右方施加移动
    // 注意：MovementVector.Y 对应 W/S（前后），MovementVector.X 对应 A/D（左右）
    AddMovementInput(CameraForward, MovementVector.Y);
    AddMovementInput(CameraRight, MovementVector.X);
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
    }
}
