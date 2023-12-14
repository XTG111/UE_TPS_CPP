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

1. 新建一个gamesubsystem的类MultiplayerSessionSubsystem，这个类中将主要实现我们的OnlineSubsystem代码
主要工作就是创建了委托和回调函数。
首先对于一个OnlineSubsystem插件来说，第一点就是需要和一个服务器产生连接，我们可以通过OnlineSubsystem实例化一个对象来获取，获取到连接后，我们需要利用Session接口来维护产生的Session，所以需要实例化一个OnlineSessionPtr的对象，可以从OnlineSubsystem中直接获得。
对于一个Session接口(Session)其需要具备以下功能：Create,Find,Join,Delete,Start
所以在gamesubsystem类中，我们需要定义对应的5个委托和其回调函数，为了删除和开启一个委托，我们还需要增加5个对于的Handle用来维护委托
gamesubsystem类是一个基于GameInstance的类，
我们在其他类中实例化我们的在线子系统时，可以通过获取当前游戏实例，利用GetSubsustem<Name>的方式来获取。
当我们在定义类中获取就直接使用IOnlineSubsystem::Get()就可以了。

## 以创建一个Session为例，分析SessionSubsystem中与之相关函数和变量的用处
首先我们为了创建一个Session即与服务器建立一个连接，定义了一个委托和管理它的Handle，以及两个函数。
1. 一个函数是用来当作委托的回调函数，该函数的作用就是当触发了CreateSession这个委托，会调用这个函数，执行这个函数的相关逻辑，一般可以写上地图的跳转功能(也可以写在主界面UI的点击功能下)
2. 另一个函数是对于我们这GameSubsystem类来说的，这个类就是我们插件所依靠的类，而我们也只能操作这个类去调用会话接口的内部函数实现创建一个会话的功能。所以这个函数的主要作用就有如下几点
- 如果存在Session那么先删除
- 对Session如何连接进行设置包括分配键值对
- 将我们定义的委托绑定到SessionInterface的委托列表中，并存储Handle
- 利用SessionInterface中的CreateSession创建一个Session
- 如果创建失败Clear Handle
3. 委托，就是当我们实现上一函数时，执行到第4步，如果成功，会去调用回调函数的实现
4. Handle，监视委托的变量，用于删除和维护


# 构建简单的UI界面实现，点击按钮进行连接
UI的源文件中主要需要进行两个按钮事件的绑定，
1. 首先需要定义两个UButton的变量，该变量名字需要和蓝图UI中的Button名字一样。
2. 其次定义OnClicked事件触发后的回调函数
3. 定义一个我们自己新建的MultiplayerSessionSubsystem的变量，该变量是用来在OnClicked对应的回调函数中实现我们的具体函数的，这样可以实现逻辑之间的分离。
```c++
void UMenuWidget::Button_HostClicked()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, FString(TEXT("Host Button Clicked")));
	}

	//调用Subsytem中的实际函数
	if (MultiplayerSessionSubsystem)
	{
		MultiplayerSessionSubsystem->CreateSession(NumPublicConnections, MatchType);
	}
}
```
4. 实现OnClicked委托的绑定，重载基类的Intialized函数
```c++
bool UMenuWidget::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	//利用自带委托绑定回调函数
	if (Button_Host)
	{
		Button_Host->OnClicked.AddDynamic(this, &UMenuWidget::Button_HostClicked);
	}
	if (Button_Join)
	{
		Button_Join->OnClicked.AddDynamic(this, &UMenuWidget::Button_JoinClicked);
	}
	return true;
}
```
5. 当成功连接到大厅，取出UI，并修改控制权，重载基类的OnLevelRemovedFromWorld函数

# 菜单UI->GameSubSystem(插件的依靠类)->SessionInterface
为了实现能够创建一个会话，加入一个会话等功能，本质上是通过调用SessionInterface类中的一系列CreateSession，FindSession来实现的。
而在实现Create或者Find后，我们需要插件干什么(跳转...)这时候就需要使用委托和回调函数，在Create和Find后自动调用，这一步是在GameSubsystem类中实现的。
那么什么时候开始Create或者Find呢？显然就是点击按钮这种操作，所以在菜单UI中就需要将OnClicked的事件绑定我们GameSubSystem中的一个函数(SessionCreate)，这个函数就是初始化Session的一些设置，并且将委托添加到SessionInterface的委托列表，以及最主要的功能SessionInterface->CreateSession(...)

我们可操作的只有两个类一个是菜单UI，一个是GameSubsystem
为了实现GameSubsystem和菜单UI的分离，菜单UI需要得到GameSubsystem在Create Find Join等操作后的信息比如是否成功之类的，所以需要我们自己创建委托，
```c++
//绑定自己创建的动态多播用于响应菜单UI界面的响应回调
//委托名字，参数类型，参数形参
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiPlayerOnCreateSessionComplete, bool, bWasSuccessful);

//动态多播的返回类型必须是UClass或者UStruct,并且与蓝图兼容

//寻找委托，返回一个数组结果，FOnlineSessionSearchResult不是一个UClass，所以在蓝图中也无法调用
//如果想在蓝图中调用，那么可以自己创建一个继承的UClass，然后采用动态多播
DECLARE_MULTICAST_DELEGATE_TwoParams(FMultiPlayerOnFindSessionComplete, const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);

//加入会话的委托传入加入的结果
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiPlayerOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type Result);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiPlayerOnDestroySessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiPlayerOnStartSessionComplete, bool, bWasSuccessful);
```
委托可以使得两个不同的类之间传递信息，比如A类想给B类通信
可以在A类中创建委托，在B类创建一个A类对象然后定义回调函数和绑定回调函数
之后在A类中需要通信的时候进行Broadcast广播相关值就可以给B类中的回调函数传递信息

回到菜单UI和GameSubsystem上，我们在GameSubsystem上创建了自定义委托，然后在UI上定义了回调函数。那么什么时候调用呢？首先UI只用两个按键一个是创建主机，一个是加入主机。
对于创建主机按键来说，这个函数内部调用GameSubsystem的CreateSessoin就可以了。
而对于加入主机来说，就比较复杂，因为它把我们之前写在Character中的操作给分散了，
1. 首先是点击按钮后，调用GameSubsystem的FindSessions函数，该函数设置了SessionSearch的参数，然后对SessionInterface进行了添加委托，如果没找到那么清除Handle，并且利用我们创建的Find自定义委托向UI的FindSession传入False这些信息。如果找到了，就代表会触发SessionInterface的回调函数，所以我们在那个回调函数中，清除Handle并且对UI的FindSession传入信息
2. UI的FindSession接受信息后，将搜寻与键值对匹配的session，如果搜寻成功，那么调用Gamesubsytem的JoinSession，Gamesubsytem的JoinSession主要就用来掉用SessionInterface的JoinSession，与Gamesubsytem的FindSession作用类似。然后将值广播给菜单UI中的JoinSession
3. 当菜单中的JoinSession得到一个结果后，就开始加载地图，而我们是客户端的操作，所以需要获取到服务器创建的会话接口，为了实现与插件的分离，我们自定义了一个局部变量首先获取Subsystem，然后利用Subsystem获取SessionInterface，之后就通过SessionInterface获取服务器地址，然后利用游戏实例GetGameInstance获取客户端的玩家控制权，最后利用ClientTravel进行关卡切换
至此插件的创建和加入功能就完成了

# 利用GameMode实现跟踪游戏状态
GameState和GameMode
GameMode是一个游戏关卡规则的制造者，GameState存储了游戏的状态数据包含玩家数量，我们通过GameMode访问GameState来获取加入我们游戏的玩家数量。
在项目不是插件中新建一个GameModeBase，其主要函数就是重载PostLogin和Logout，我们利用GameState获取玩家数量
```c++
void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	//GetGameState<AGameStateBase>()获取当前的GameState
	if (GetGameState<AGameStateBase>())
	{
	//利用GameState获取玩家数量
		int32 NumberOfPlayers = GetGameState<AGameStateBase>()->PlayerArray.Num();
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(1, 60.f, FColor::Yellow, FString::Printf(TEXT("Player in game: %d"), NumberOfPlayers));
			
			//利用PlayerState获取玩家的名字
			APlayerState* PlayerState = NewPlayer->GetPlayerState<APlayerState>();
			if (PlayerState)
			{
				FString PlayerName = PlayerState->GetPlayerName();
				GEngine->AddOnScreenDebugMessage(-1, 60.f, FColor::Cyan, FString::Printf(TEXT("%s has Join the Game"), *PlayerName));
			}
			
		}
	}
}
```
之后需要在UE中新建一个以这个GameMode为父类的BP_GameMode，然后运用在大厅地图上，这样每当有人加入到我们的大厅地图就可以打印信息了

# 删除会话和创建会话的优化
我们之前在创建会话中，有当存在会话，调用Destroy删除会话，但这样存在一个问题，就是删除是需要时间，但我们客户端已经被提出了服务器，这时候去创建是不能创建的。所以我们利用我们自己创建的DestroySession函数来实现销毁功能，在创建会话中检测到还存在一个会话的时候，我们先利用一个bool变量来表示我们正在由删除向创建变化，我们调用DesstroySession函数，其里面绑定SessionInterface的Destroy委托，当删除成功的时候，判断bool值，如果为true，那么重置bool值，然后调用创建的函数。并且创建会话函数的参数，我们可以先用变量存起来，两个变量的值就是上一次创建会话的参数值
```c++
//.h
	//用来判断是否还在删除，还是删除完成
	bool bCreateSessionOnDestroy{ false };

	//存储一些创建的信息，创建的连接数量和匹配的键值对
	int32 LastNumPublicConnections;
	FString LastMatchType;
	
//.cpp

//creatsession
	//如果存在会话销毁它
	auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if(ExistingSession != nullptr)
	{
		bCreateSessionOnDestroy = true;
		LastNumPublicConnections = NumPublicConnections;
		LastMatchType = MatchType;
		//使用自己创建的销毁会话函数
		DestroySession();
	}

//destroysession
void UMultiplayerSessionSubsystem::DestroySession()
{
	if (!SessionInterface.IsValid())
	{
		//向UI响应
		MultiPlayerOnDestroySessionComplete.Broadcast(false);
		return;
	}

	DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);
	if (!SessionInterface->DestroySession(NAME_GameSession))
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		MultiPlayerOnDestroySessionComplete.Broadcast(false);
	}
}

//ondestroysession
void UMultiplayerSessionSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	}
	//当被提出服务器，想自己创建时，点击Host先判断是否存在，如果存在调用Destroy，然后在回调成功下继续创建
	if (bWasSuccessful && bCreateSessionOnDestroy)
	{
		bCreateSessionOnDestroy = false;
		CreateSession(LastNumPublicConnections, LastMatchType);
	}
	MultiPlayerOnDestroySessionComplete.Broadcast(bWasSuccessful);

}
```