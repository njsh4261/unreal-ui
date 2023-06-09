# 개요
Unreal Engine 5에서 UI를 구성하고 기능을 구현한 예시 프로젝트
- Slate로 UI 구성
  - 생성한 C++ UUserWidget 클래스를 블루프린트 클래스를 통해 레벨 뷰포트에 출력
  - 블루프린트 또는 UMG에서 별도 UI 작업 없이 RebuildWidget() 오버라이딩하여 UI 구현
- 위젯 클래스 생성 시 동적으로 텍스처를 로딩하여 버튼 브러쉬 세팅 기능 구현
  - 기본 이미지, 마우스 호버 이미지, 클릭 이미지 다르게 세팅
- 버튼 클릭 시 외부 URL의 이미지를 텍스쳐로 로딩하도록 기능 구현
  - HTTP request를 통해 이미지를 가져오도록 비동기 처리
  - 가져온 이미지를 UTexture2D로 변환하도록 비동기 처리



# 개발 환경
- Unreal Engine 5.2
- Visual Studio 2022
- C++ 17



# 예시 영상
https://github.com/njsh4261/unreal-ui/assets/54832818/886d0bc7-5995-4c62-9641-25d9fa149151
