#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MainCharacter.generated.h"

// 前向声明，避免包含完整头文件
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS()
class SWARMCALCULATIONS_API AMainCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    // 构造函数，设置组件默认值
    AMainCharacter();

protected:
    // 弹簧臂组件：用于连接摄像机并实现距离/角度控制
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    USpringArmComponent* SpringArm;

    // 摄像机组件：实际渲染视角
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    UCameraComponent* Camera;

    // ----------------------------- 输入相关 -----------------------------
    // 增强输入动作：移动（二维向量，对应 WASD）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* MoveAction;

    // 默认输入映射上下文：定义如何将物理按键映射到输入动作
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputMappingContext* DefaultMappingContext;

    // 用于上升/下降的一维输入动作
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* UpDownAction;

    // 移动范围边界（在蓝图中可编辑）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Bounds")
    FVector MinBounds;   // 最小边界 (X, Y, Z)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Bounds")
    FVector MaxBounds;   // 最大边界 (X, Y, Z)

    // 移动函数，由增强输入系统调用
    void Move(const FInputActionValue& Value);
    void MoveUpDown(const FInputActionValue& Value);

    virtual void Tick(float DeltaTime) override;
    virtual void BeginPlay() override;

    // 重写 SetupPlayerInputComponent 以绑定输入
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
    // 检查并限制位置
    void ClampPositionToBounds();
};
