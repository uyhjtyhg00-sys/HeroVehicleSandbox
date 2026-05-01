# HeroVehicleSandbox Function Comment Policy

이번 패치부터 핵심 gameplay 코드의 함수는 다음 형식으로 기능 주석을 유지합니다.

```cpp
// 기능: 이 함수가 게임 안에서 담당하는 역할을 한 문장으로 설명한다.
void SomeFunction();
```

## 적용 범위

이번 패치에서 실제로 수정된 파일은 함수 단위 기능 주석을 포함합니다.

- `Source/HeroVehicleSandbox/Public/HeroCharacter.h`
- `Source/HeroVehicleSandbox/Private/HeroCharacter.cpp`
- `Source/HeroVehicleSandbox/Private/HeroWeaponComponent.cpp`
- `Source/HeroVehicleSandbox/Private/HeroHUDWidget.cpp`
- `Source/HeroVehicleSandbox/Private/HeroPlayerController.cpp`
- `Source/HeroVehicleSandbox/Private/HeroSettingsWidget.cpp`
- `Source/HeroVehicleSandbox/Public/HeroTypes.h`

## 유지 규칙

1. 새 함수 추가 시 반드시 `// 기능:` 주석을 붙입니다.
2. 함수 이름보다 실제 플레이 기능을 설명합니다.
3. 입력/감도/반동/줌처럼 서로 영향을 주면, 어느 시스템에 종속되는지 명시합니다.
4. 마우스 반전 옵션은 수동 시점 조작에만 적용하고, 반동/카메라 킥에는 적용하지 않습니다.
5. DPI는 cm/360 표시용이며, 런타임 회전은 `Sensitivity × 0.0066`만 사용합니다.
