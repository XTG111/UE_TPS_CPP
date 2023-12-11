---
title: README
created: '2023-12-05T12:23:41.202Z'
modified: '2023-12-05T14:40:54.850Z'
---

# UE_TPS_CPP
 多人TPS项目学习
 
# LAN UE
构建关卡，在角色按下按键后跳转到对应关卡利用节点Open Level可以设置Options为Listen将这个关卡作为监听服务器

LAN连接需要在同一个wifi下才有效果，当运行另一个机器上的游戏实例时，通过按键连接到作为服务器的本地机器上。
为了连接到服务器需要使用execute Console Command节点使用命令open [ipv4addr]

C++代码实现：
在角色蓝图中
首先时需要创建被当作服务器的地图：
```c++
//为服务器设置大厅
//地图地址后增加Listen将该地图指定为监听服务器地图
//当调用这个函数，调用的实例就会转到这个地图并且成为服务器
void AMPGameDemoCharacter::OpenLobby_Map()
{
	UWorld* World = GetWorld();
	if (World) {
		World->ServerTravel("/Game/ThirdPersonCPP/Maps/Lobby_Map?listen");
	}
}
```

下面两个函数都是客户端连接加载地图的方式
当客户端实例对象执行这个函数后，客户端将打开Ip为Addr的服务器上的地图
实际上OpenLevel更多的用于单人游戏上打开另一个关卡
```c++
#include "Kismet/GameplayStatics"
void AMPGameDemoCharacter::CallOpenLobby_Map(const FString& Addr)
{
	UGameplayStatics::OpenLevel(this, *Addr);
}
```
同样要对该角色类实现客户端的功能，由于ClientTravel受到角色控制器的控制，所以需要先获得客户端上角色实例的控制器，调用ClientTravel进行连接
```c++
void AMPGameDemoCharacter::CallClientTravel(const FString& Addr)
{
	//需要获取客户端上的角色控制器 
	APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
	if (PlayerController) {
		PlayerController->ClientTravel(Addr, ETravelType::TRAVEL_Absolute);
	}
}
```

# 虚幻子系统
UE抽象了向各个服务供应商的网络代码，我们只需要编写虚幻格式的网络架构，UE会自动处理分发到各个服务供应商的代码。使得游戏可以接受到其他玩家的连接
对于服务器：点击开始，UE子系统自动进行session设置，然后开始创建session最后打卡地图等待玩家加入
对于客户端：点击开始，UE子系统自动进行搜索设置，然后开始查找sessions，该步骤返回一个列表，将从里面选择一个有效的session进行连接，获取到session的IP使用clienttravel连接。
这些步骤都是被封装好的，一般写在角色的class类中

# 创建与Steam连接的会话SessionInterface
安装好插件，并且在Bulid.cs中添加Module后，重新编译项目。
并在DefaulEngine.ini中添加了Steam配置模块
```
[/Script/Engine.GameEngine]
+NetDriverDefinitions=(DefName="GameNetDriver",DriverClassName="OnlineSubsystemSteam.SteamNetDriver",DriverClassNameFallback="OnlineSubsystemUtils.IpNetDriver")

[OnlineSubsystem]
DefaultPlatformService=Steam

[OnlineSubsystemSteam]
bEnabled=true
SteamDevAppId=480

; If using Sessions
; bInitServerOnClient=true

[/Script/OnlineSubsystemSteam.SteamNetDriver]
NetConnectionClassName="OnlineSubsystemSteam.SteamNetConnection"
```

为了检测是否使用到OnlineSubsystem，在构造函数中定义了IOnlineSubsystem的指针，并且在Debug输出是否有连接。
```c++
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		OnlineSessionInterface = OnlineSubsystem->GetSessionInterface();

		//打印调试信息
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Blue, FString::Printf(TEXT("Found subsystem: %s"), *OnlineSubsystem->GetSubsystemName().ToString()));
		}
	}
```

创建会话界面，利用智能指针创建了SessionInterface类的接口变量，用来存储OnlineSubsystem的中的会话界面。
```c++
//.h文件中
public:
	//在线会话界面接口的指针，这是一个智能指针
	//有两种定义方式
	/*
	* //需要在.cpp文件中包含#include "Interfaces/OnlineSessionInterface.h"导入IOnlienSession类*/
	IOnlineSessionPtr OnlineSessionInterface;
	
	//第二种利用其自身的模板定义
	//TSharedPtr<class IOnlineSession, ESPMode::ThreadSafe> OnlineSessionInterface;

//.cpp文件中
OnlineSessionInterface = OnlineSubsystem->GetSessionInterface();
```
注意只有在打包后才会看到调试连接到了Steam

# 利用委托测试会话的连接
委托是一个拥有UE函数引用的对象，通过将函数绑定在一个委托上，当执行这个委托时，会广播(执行)绑定的所有函数。
之前定义的会话接口，通过CreateSession()与Steam连接，当Steam上的Session进行创建后，会返回信息告诉这个会话接口创建成功。
SessionInterface自定义了一个委托列表，用来响应返回的数据信息，并且通过触发条件进行迭代。
我们为了获得连接成功的消息，在角色Actor文件中定义一个委托事件，并将它加入到SessionInterface的委托列表中，然后绑定一个回调函数用来debug是否创建成功。
当成功创建，Steam会返回一个信号，SessionInterface会对这个委托列表遍历，然后触发这个委托。

首先在角色Actor中定义了一个蓝图可调用函数，用来按键操作创建Session，当我们利用OnlineSubsystem构造时，就利用一个变量SessionInterface存入了与Steam的连接，所以用这个连接来创建会话Session，首先检测是否创立了这个连接
```c++
//OnlineSessionInterface该变量，应当持有创建的Session连接了
	if (!OnlineSessionInterface.IsValid())
	{
		return;
	}
```
如果为真，还要检测当前连接上面是否已经有会话在运行，如果有先销毁再继续新建一个会话
```c++
//如果存在一个会话那么将不会被创建,默认Session名字是NAME_GameSession
	auto ExistingSession = OnlineSessionInterface->GetNamedSession(NAME_GameSession);
	if(ExistingSession != nullptr)
	{
		//如果存在一个会话，那么先销毁
		OnlineSessionInterface->DestroySession(NAME_GameSession);
	}

	//给会话Session添加委托
	OnlineSessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	//创建一个新的会话 首先进行会话设置FOnlineSessionSettings
	TSharedPtr<FOnlineSessionSettings> SessionSettings = MakeShareable(new FOnlineSessionSettings());
	SessionSettings->bIsLANMatch = false;
	SessionSettings->NumPublicConnections = 4;
	SessionSettings->bAllowJoinInProgress = true;
	SessionSettings->bAllowJoinViaPresence = true;
	SessionSettings->bShouldAdvertise = true;
	SessionSettings->bUsesPresence = true;

	//用于创建session的第一个参数，IP地址
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	OnlineSessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *SessionSettings);
```
当销毁了之前存在的会话后，我们需要对这个会话接口添加一个委托，用来检测是否会话创建成功，之后进行待创建会话的设置，然后利用该接口创建会话。
创建会话需要获得我们作为目标服务器的IP地址，可以利用LocalaPlayer的NetID来获取。

接下来我们来处理委托事件，就是在构造函数中利用列表初始化，对委托事件进行绑定回调函数
```c++
AMenuSystemCharacter::AMenuSystemCharacter() :
	CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &AMenuSystemCharacter::OnCreateSessionComplete))
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;
```
回调函数OnCreateSessionComplete的编写也比较简单，通过对bool值的判断，输出对应的调试信息
```c++
void AMenuSystemCharacter::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15., FColor::Blue, FString::Printf(TEXT("Created Session: %s"),*SessionName.ToString()));
		}
	}
	else
	{
		if(GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15., FColor::Red, FString::Printf(TEXT("Failed")));
		}
	}
}
```
这样我们就可以就利用Steam提供的服务与我们的服务器建立会话，形成连接了

# 其他机器加入这个会话
相当于在客户端也需要使用一个SessionInterface来维护与Steam建立的连接，然后通过这个接口Interface去搜索其他主机创建的Session
首先定义一个蓝图中可以绑定的函数，用于使用按键来开始搜索服务器创建的会话。
```c++
void AMenuSystemCharacter::JoinGameSession()
{
	//在客户端我们同样需要创建session接口，用来连接Steam上的Session
	if (!OnlineSessionInterface.IsValid()) 
	{
		return;
	}

	OnlineSessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionCompleteDelegate);
	//设置搜索返回结果
	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	//用来确定搜索多少次结果，因为是公用ID端口，所以要把搜索数变大一些，避免不能搜到
	SessionSearch->MaxSearchResults = 10000;
	SessionSearch->bIsLanQuery = false;
	SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	//设置会话接口中的寻找到的会话
	//同样需要传入LocalPlayer的NetID
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	OnlineSessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef());

}
```
为了检测是否搜索到服务器创建的会话，我们再定义了一个委托FindSessionCompleteDelegate，用于处理当客户端搜索到这个会话后的debug。同创建连接设置会话参数一样，UE提供了会话搜索的一种智能指针，并且利用该指针可以获得搜索结果，为了在debug中调用它，将其定义为全局变量，客户端利用接口维持的连接去寻找已创建的会话时，同样需要传入自己的NetID以及之前设置的搜索参数。
之后我们利用回调函数来输出信息
```c++
void AMenuSystemCharacter::OnFindSessionComplete(bool bWasSuccessful)
{
	for (auto Result : SessionSearch->SearchResults)
	{
		FString Id = Result.GetSessionIdStr();
		FString User = Result.Session.OwningUserName;
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Cyan, FString::Printf(TEXT("ID:%s, User:%s"), *Id, *User));
		}
	}
}
```

当寻找到当前的所有Session后，需要去判断哪一个是我们服务器创建的Session，这里利用了键值对来进行判断，首先在创建Session的代码中，对SessionSetting进行修改
```c++
//设置一些键值对，使得我们可以区分其他的session连接
	SessionSettings->Set(FName("Match"), FString("ClientJCS"), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
```
我们设置了键值对 Match:ClientJs，并在创建Session的回调函数中利用ServelTravel实现服务器地图关卡的转换
```c++
//跳转到大厅
		UWorld* World = GetWorld();
		if (World)
		{
			World->ServerTravel(FString("/Game/ThirdPersonCPP/Maps/Lobby?listen"));
		}
```
在FindSession函数中，定义一个变量来接受每个Session的Match值，当检测到这个变量和ClientJCS相同时，利用会话接口的JoinSession功能实现客户端与服务器的连接。
```c++
//添加委托事件
			OnlineSessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

			//客户端的IP
			const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();

			//给JoinSession添加客户端信息，session信息,session的名字和服务器信息
			OnlineSessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, Result);
```
当我们连接到服务器时，就需要进行跳转到连接大厅，所以对于JoinSession同样也添加一个委托绑定，回调函数就是编写跳转地图的指令，利用会话接口的GetResolvedConnectString函数可以获得服务器大厅地图的地址，然后使用ClientTravel进行地图关卡切换
```c++
FString Addr;
	if (OnlineSessionInterface->GetResolvedConnectString(NAME_GameSession, Addr))
	{
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, FString::Printf(TEXT("Client String:%s"), *Addr));
		}

		//客户端利用ClientTrave进行地图转换
		APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
		if (PlayerController)
		{
			PlayerController->ClientTravel(Addr, ETravelType::TRAVEL_Absolute);
		}
	}
```

# 自己插件的构建
首先利用Plugin中的Create Plugin创建一个空白的Plugin

通过包含OnlineSubSystem等来实现自己的OnlineSubSystem插件，为了能够使得我们的插件在整个游戏过程中能够使用，我们需要将其和游戏示例类绑定在一起，UE为我们提供了SubSystem的实例类，可以通过SubSystem来获取游戏实例的SubSystem，在我们构建的插件文件夹中添加class文件
[Program Subsystem](https://docs.unrealengine.com/4.27/en-US/ProgrammingAndScripting/Subsystems/)

1. 新建一个gamesubsystem的类MultiplayerSubsystem，这个类中将主要实现我们的OnlineSubsystem代码
主要工作就是创建了委托和回调函数。
首先对于一个OnlineSubsystem插件来说，第一点就是需要和一个服务器产生连接，我们可以通过OnlineSubsystem实例化一个对象来获取，获取到连接后，我们需要利用Session接口来维护产生的Session，所以需要实例化一个OnlineSessionPtr的对象，可以从OnlineSubsystem中直接获得。
对于一个Session接口(Session)其需要具备以下功能：Create,Find,Join,Delete,Start
所以在gamesubsystem类中，我们需要定义对应的5个委托和其回调函数，为了删除和开启一个委托，我们还需要增加5个对于的Handle用来维护委托

