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

# 构建AnimInstance的父类
主要重载两个函数，一个是在初始化NativeInitializeAnimation，一个是每帧执行NativeUpdateAnimation

# Seamless Travel
GameMode->bUseSeamlessTravel = true;
需要一个过渡地图，用于在地图切换之间使用，相当于一个过渡界面.
## Travel的方式
UWorld::ServerTravel
只有服务器调用，当服务器切换地图后，所有连接的客户端一同跳转，利用APlayerController::ClientTravel

APlayerController::ClientTravel
必须指定服务器地址

## 创建一个GameMode
该GameMode用于统计人数，当连入大厅的人数到达阈值，进行跳转

# LocalRole RemoteRole NetWorkRole
区分现在角色被谁操控
ENetRole:

当是LocalRole时，本地，相当于服务器本地创建，然后传给每个客户端
	1. Role_Authority：服务器
	2. Simulatedproxy：其他客户端上你的角色
	3. AutonomousProxy：自己机器上的角色，可以被你操控
	4. None：没有一个机器上有这个角色

当是RemoteRole时，相当于是每个客户端创建，然后传给服务器
	1. Role_Authority：客户端你操控的角色和其他客户端显示在你的客户端上的角色
	2. Simulatedproxy：服务器上观测到对应的所有客户端角色
	3. AutonomousProxy：服务器上操控的角色
	4. None：没有一个机器上有这个角色
	
# 设置武器
编写的所有逻辑都是服务器的逻辑
客户端和服务器
将重叠判断操作都放在服务端进行，设置bReplicates = true;新建一个武器类可以作为父类，然后设置骨骼网格体组件和球体检测组件，默认设置球体网格体组件的碰撞为对所有通道忽略，不开启碰撞，这是在客户端的设置，因为客户端只需要播放一个拾取动画。
判断是否能够拾取，则使通过服务器来判断的，所以服务器上的武器球体需要设置为模拟物理，并且对人的Channel设置重叠事件

## Replication
bReplicates = true;
必须开启复制

在客户端显示服务器的处理结果，repnotify
在角色文件中，添加武器的变量并定义UPROPERTY(Replicated)，为了获取这个复制的变量，需要重写一个函数GetLifetimeReplicatedProps，为了复制这个变量实例的生命周期
```c++
//.h
virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

UPROPERTY(Replicated)
class AWeapon* Weapon;

//.cpp
void AXCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//复制我们需要的变量生命周期,只复制到当前操控角色的客户端
	DOREPLIFETIME_CONDITION(AXCharacter, OverlappingWeapon,COND_OwnerOnly);
}
```
我们在武器蓝图中定义了一个显示可拾取的UI，如果只是在代码中设置重叠事件响应调用SetVisibility函数，那么只会在服务器上有反应，所以我们需要向客户端复制这个情况，首先在角色蓝图中设置一个存储weapon的变量，并定义一个给weapon赋值的函数，当我们在武器代码中发生重叠或者结束重叠时，调用这个函数给角色的weapon赋值。

然后在定义的RepNotify函数中，编写客户端的响应，比如显示UI和消失UI。由于RepNotify我们定义的是复制时调用，所以只有服务器的值复制到客户端时才会调用UI的显示和消失，而如果服务器上存在一个可以控制的actor，为了使得服务器上的actor也有这个效果，我们可以在给weapon赋值的函数上进行UI的显示和消失设置(记住，一切代码都是运行在服务器上的)，服务器和客户端通过复制的方式传递服务器处理后的信息。

```c++
//Character_Class
//.h

//角色中存在的武器
UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
class AWeapon* OverlappingWeapon;
//Rep_Notify函数,LastWeapon可以存储上一次碰撞的结果
UFUNCTION()
void OnRep_OverlappingWeapon(AWeapon* LastWeapon)

//设置OverlappingWeapon的函数，在Weapon_Class中调用
//并且服务器上Actor对于球体碰撞的响应也写在这里，因为Weapon_Class只调用了这一个函数
UFUNCTION()
void SetOverlappingWeapon(AWeapon* Weapon);

//.cpp
void OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	//对于重叠，检测OverlappingWeapon是否为空
	if(OverlappingWeapon)
	{
		OverlappingWeapon->ShowWidget(true);
	}
	//对于结束重叠，LastWeapon不为空
	if(LastWeapon)
	{
		LastWeapon->ShowWidget(false);
	}
}

void SetOverlappingWeapon(AWeapon* Weapon)
{
	//服务器上的Actor结束重叠，此时OverlappingWeapon还不为Null
	if(OverlappingWeapon)
	{
		OverlappingWeapon->ShowWidget(false);
	}
	OverlappingWeapon = Weapon;
	//服务器上的Actor开始重叠，被Weapon赋值，不为空
	if(OverlappingWeapon)
	{
		OverlappingWeapon->ShowWidget(true);
	}
}

//Weapon_Class
//三个函数
OnBeginOverlap{Character_Class->SetOverlappingWeapon(this);}
EndOverlap{Character_Class->SetOverlappingWeapon(nullptr);}
//控制UI的显示
void AWeaponParent::ShowPickUpWidget(bool bShowWidget)
{
	if (PickUpWidgetComp)
	{
		PickUpWidgetComp->SetVisibility(bShowWidget);
	}
	
}
```

# 装备武器
我们为战斗系统新建了一个Component，这个Component将管理我们的所有有关战斗的功能，为了实现装备武器，利用骨骼Socket实现武器的附加，并且装备武器这个功能，应当是服务器处理请求，然后通过复制状态传递给客户端，也就是说，在角色源码中拾取的动作响应应当限制为服务器即使用HasAuthority()

首先在武器源码中，设置武器状态，对应之前我们定义的枚举值。然后在角色源码中，设置绑定按键Equip，装备的武器就是发生了overlap的武器，然后在战斗Component中，进行具体的附加操作
获取socket:Character->GetMesh()->GetSocketByName()
附加武器到骨骼：Socket->AttachActor(WeaponActor,Character);
附加上还需要关闭显示的widget

# RPC
客户端使用RPC，远处调用服务器上的函数，通过使用UFUNCTION()说明符来指定RPC调用，比如客户端调用，服务器执行使用UFUNCTION(Server)，并且还需要指定，RPC的可靠性(类似TCP和UDP)。
在定义具体实现是函数名需要修改为FUNCName_Implementation。
为了实现客户端拾取操作的显示，利用RPC定义函数，函数中调用战斗组件中的装备函数，之后再在角色的按键响应函数中调用即可实现。

# 复制传递每个客户端的枪械状态


# RPC和属性复制
两种服务器与客户端交互的方法，属性复制只能从服务器到客户端，也就是说Rep_Notify函数只会在客户端上实现，且必须在构造函数中开启bReplicates = true;或者对于某个组件使用SetIsReplicated(true)
RPC是双向的，通过UFUCNTION()说明Sever还是Client

任何逻辑都是服务器上运行。

## 属性复制的基本流程
1. 设置要复制的类对象，或者组件的Replicate
```c++
bReplicates = true;
SetIsReplicated(true);
```
bReplicates = true;SetIsReplicated(true)
2. UE内部定义的用于复制传递生命周期的函数GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)
```c++
//.h
//通过复制向客户端传递服务器的处理结果
virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

//.cpp
void AWeaponParent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWeaponParent, WeaponState);
}
//若只要操控actor的客户端有复制结果，应当使用
DOREPLIFETIME_CONDITION(AXCharacter, OverlappingWeapon,COND_OwnerOnly);

//分别对应 要复制的类，要复制的对象
```
3. 如果当复制值发生改变时，客户端另一些变量也要发生改变，这时候就需要使用RepNotify功能，将服务器此时的状态再传递给客户端
```c++
//.h
//声明一个要复制的变量
UPROPERTY(VisibleAnywhere,ReplicatedUsing = OnRep_FuncName)
	class Class* A;
//RepNotify函数
UFUNCTION()
void OnRep_FuncName();
```
RepNotify函数一般不带参数，如果需要带只能带一个同样类型的形参。该参数可以表示上一次复制时的值。
4. 这样我们就可以再RepNotify函数中写变量是怎么变化的了，和服务器的写法是一样的。
```c++
//通过复制，客户端调用
void AWeaponParent::OnRep_WeponState()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Initial:
		break;
	case EWeaponState::EWS_Equipped:
		ShowPickUpWidget(false);
		
		//SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EWeaponState::EWS_Dropped:
		break;
	case EWeaponState::EWS_MAX:
		break;
	default:
		break;
	}
}

//服务器调用
void AWeaponParent::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	switch (WeaponState)
	{
	case EWeaponState::EWS_Initial:
		break;
	case EWeaponState::EWS_Equipped:
		//当装备上时应当关闭UI和球体碰撞
		ShowPickUpWidget(false);
		//禁用球体碰撞
		SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EWeaponState::EWS_Dropped:
		break;
	case EWeaponState::EWS_MAX:
		break;
	default:
		break;
	}
}
```

## RPC的基本流程
RPC相当于函数调用，只不过是在一个机器请求，另一个机器调用实现后的结果返回请求的机器
1. RPC是双向的，但也要定义其可复制的属性为true，即与属性复制的1，2步操作相同
2. RPC使用说明符UFUNCTION(Server/Client, [Reliable])
```c++
	//RPC客户端调用，服务器执行，约定在函数名前加上Server
	UFUNCTION(Server,Reliable)
		void ServerEquipWeapon();
```
3. 在.cpp文件中实现函数
```c++
//在客户端上的Actor执行
void AXCharacter::ServerEquipWeapon_Implementation()
{
	if (CombatComp)
	{
		CombatComp->EquipWeapon(OverlappingWeapon);
	}
}
```
需要注意的是，我们只是定义和声明了这个RPC函数，但是它不像属性复制的RepNotify函数可以自己调用，所以我们需要将其手动调用。
\_Implementation是UE定义的，为了识别为RPC，在定义函数的时候需要加上，调用和声明不用
4. 调用，我们对于需要调用的时候，就是按照单机游戏时，需要调用的位置进行判断。那么对于网络游戏，利用HasAuthority()来判断是不是服务器的Actor的请求调用，如果是，那么就和单机游戏的操作一样，如果不是，那么就调用这个RPC函数，RPC函数内部的实现也是和单机游戏一样的。
```c++
//装备武器
//只需要在服务器上验证，如果在服务器上的Actor
void AXCharacter::EquipWeapon()
{
	if (CombatComp)
	{
		if (HasAuthority())
		{
			CombatComp->EquipWeapon(OverlappingWeapon);
		}
		else
		{
			ServerEquipWeapon();
		}
	}
}
//在客户端上的Actor执行
void AXCharacter::ServerEquipWeapon_Implementation()
{
	if (CombatComp)
	{
		CombatComp->EquipWeapon(OverlappingWeapon);
	}
}
```

# 装备武器动画
一个动画实例只能访问对应机器上的变量，客户端的动画实例只能查询客户端的变量，而无法获取服务器上的变量，所以对于是否装备了武器，我们通过战斗组件中的装备武器变量是否为空来判断，那么这个值应当进行属性复制给客户端
```c++
//战斗组件.h
//通过复制向客户端传递服务器的处理结果 EquippedWeapon
virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
//装备上的武器实例
//EquippedWeapon用来判断是否装备武器，从而切换动画，需要赋值给客户端
UPROPERTY(Replicated ,VisibleAnywhere)
	class AWeaponParent* EquippedWeapon;
	
//战斗组件.cpp
void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bUnderAiming);
}
```

# 瞄准状态
同样所有逻辑都是运行在服务器上的，我们编写了瞄准的控制和动画蓝图，只会在服务器上有体现，所以为了客户端的动画能够播放，所以我们需要将瞄准状态的值属性复制给客户端
```c++
//战斗组件.h
//瞄准状态
UPROPERTY(Replicated, VisibleAnywhere)
	bool bUnderAiming;
```

但之后又出现一个问题就是我们的瞄准只会出现在当前的客户端上，服务器和其他客户端不知道，这是因为属性复制是单向的，所以我们需要利用RPC来实现瞄准状态的获取，就需要我们在战斗组件中定义一个更改瞄准状态的函数，和一个RPC更改状态的函数，在其中设置瞄准状态的改变，从角色源码中调用。
```c++
//character.h
void AXCharacter::RelaxToAimMode()
{
	if (CombatComp)
	{
		CombatComp->SetAiming(true);
	}
}

void AXCharacter::AimToRelaxMode()
{
	if (CombatComp)
	{
		CombatComp->SetAiming(false);
	}
}

//战斗组件.h
//服务器的最终处理函数
void SetAiming(bool)

//RPC函数
UFUNCTION(Server,Reliable)
void ServerSetAiming(bool)

//战斗组件.cpp
void SetAiming(bool)
{
	//避免网络延迟
	bAiming = bool;
	if(!character->HasAuthority())
	{
		ServerSetAiming(bool)
	}
}
void ServerSetAiming_Implementation(bool)
{
	bAiming = bool;
}
```
这样的流程就是客户端调用服务器上的瞄准函数，服务器上该客户端角色的变量值发生改变，然后服务器将这个状态广播给其他所有客户端

值得注意的是，由于网络延迟的原因，我们可以在调用之前先设置变量的改变，使得服务器可以提前做出响应。

## AimOffset
客户端向服务器发送pitch旋转值的时候，会进行压缩，导致负数变为无符号的正数，使得体现在服务器上的AO出现问题，进而导致其他客户端的出现问题
```c++
//CharacterMovement.cpp
void FSavedMove_Character::GetPackedAngles(uint32& YawAndPitchPack, uint8& RollPack) const
{
	// Compress rotation down to 5 bytes
	YawAndPitchPack = UCharacterMovementComponent::PackYawAndPitchTo32(SavedControlRotation.Yaw, SavedControlRotation.Pitch);
	RollPack = FRotator::CompressAxisToByte(SavedControlRotation.Roll);
}

//Rotator.h
FORCEINLINE uint16 FRotator::CompressAxisToShort( float Angle )
{
	// map [0->360) to [0->65536) and mask off any winding
	return FMath::RoundToInt(Angle * 65536.f / 360.f) & 0xFFFF;
}
```
解决方法就是利用映射将270-\>360变为-90-\>\0，需要注意的是，只在不是本地的机器上进行转换，因为本地客户端是正确的响应
```c++
if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		//客户端-90,0会被压缩为360,270，进行强制转换
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}
```

# FABRIK
需要在代码中设置IK的变换，通过添加插槽，设置这个插槽的变换来实现的GetSocketTransform，之后需要通过目标骨骼的限制来获取这个变换的具体位置TransformToBoneSpace。
```c++
	//获取插槽的变换
	LeftHandTransform = EquippedWeapon->WeaponMesh->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
	FVector OutPosition;
	FRotator OutRotation;
	//设置基于目标骨骼的变换
	XCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
	LeftHandTransform.SetLocation(OutPosition);
	LeftHandTransform.SetRotation(FQuat(OutRotation));
```

# 网络更新频率
转身更平滑，开启rotate root bone的interp result

角色蓝图中的replication选项：net update frequency
```.ini
在默认引擎设置中，设置服务器最大刷新率，即服务器向客户端传输的最大更新值
[/Script/OnlineSubsystemUtils.IpNetDriver]
NetServerMaxTickRate = 60
```

# 动画滑步的简单修复
通过改变动画，增加混合空间的插值量
添加音效：sync markers,在混合空间中可以同步，各动画的声音

# 射击设计
1. projectile方式
2. 射线检测方式

# 射击应用RPC
rep通知只在值发生改变时，进行函数逻辑的执行。
选择Multicast RPC进行多个客户端的通知
在服务器上运行Multicast会在服务器和它所属的所有客户端上运行
在客户端上运行Multicast只会在该客户端上运行。
所以我们定义和声明的Multicast函数，该函数的具体调用是在我们的Server RPC函数内
具体实现功能操作函数->调用ServerRPC函数->调用Multicast函数->内部写上具体逻辑

# 子弹射击运动过程中粒子特效的添加
首先需要一个轨道的定义，然后在BeginPlay中初始化
```c++
	UPROPERTY(EditAnywhere)
		class UParticleSystem* Tracer;
	class UParticleSystemComponent* TracerComponent;
```
为了使得只有服务器控制子弹射击的效果，而客户端无法修改，所以需要将开枪的函数逻辑设置为只有HasAuthority()才能执行，并且为了使得客户端也有粒子特效的效果，可以将子弹类修改为可复制的，这样就会从服务器向客户端复制。但处理这个的权限还是在服务器上。

# 传递准星位置使得服务器上有准确响应
在Fire的RPC中传递参数类型为FVector_NetQuantize的参数，其有助于减少带宽，，在可以考虑的精度范围内。
射线检测到的HitResult.ImpactPoint就是这个类型
```c++
/**
 *	FVector_NetQuantize
 *
 *	0 decimal place of precision.
 *	Up to 20 bits per component.
 *	Valid range: 2^20 = +/- 1,048,576
 *
 *	Note: this is the historical UE format for vector net serialization
 *
 */
```
通过RPC函数传递的参数，会随着RPC函数的广播，一同广播复制

# 服务器向客户端传递音效等射击特效
可以利用netmulticast来传播，但这样会增加网络带宽。
我们可以利用Destroyed()函数来实现传递，当子弹被销毁的时候，我们产生特效
Destoyed()函数会在actor在游戏过程中或者编辑器内被销毁的过程中进行调用 the actor is explicitly being destroyed during gameplay or in the editor
```c++
	/** Called when this actor is explicitly being destroyed during gameplay or in the editor, not called during level streaming or gameplay ending */
	virtual void Destroyed();
```
所以当我们调用了destroy()来销毁子弹后，会自动调用Destroyed()函数，由于我们将子弹的属性设置为了可复制的，所以当一个子弹销毁的时候，销毁这个表现也会被服务器复制到每个客户端上。

# 抛壳特效
利用socket来确定抛壳位置，同产生子弹一样
下面总结一下在一个socket生成物体的模板
```c++
#include "Engine/SkeletalMeshSocket.h"

//获取socket
USkeletalMeshSocket* SocketName = SocketOwnerMesh->GetSocketByName(FName("Socket_Name"));

//获取当前socket的变换transform
FTransform SocketTransform = SocketName->GetSocketTransform(SocketOwnerMesh);

//可以自定义生成的方向和朝向,还有生成时需要忽略的等等
//生成actor在socket
UWorld* World = GetWorld();
if(World)
{
	World->SpawnActor<ClassOfActor>(Actor,SocketTransform,...);
}
```
设置完之后，记得在蓝图中新建要生成的类，并在会生成这个actor的地方给Actor赋值

# HUD绘制
Player::Controller -->APlayerController::GetHUD()
AHUD::DrawHUD()
AHUD::DrawTexture()

为了实现准星的绘制和游戏控制，新建了playercontroller和hud文件，然后将这两个的蓝图类写入到gamemode里面。我们的准星实现是将准星数据存储到武器中，然后在利用战斗组件进行调用设置HUD的参数，从而绘制出来准星。
1. 先在武器父类中，添加2d纹理变量
2. 在HUD父类中，创建存储准星数据的结构体，并且重写DrawHUD函数，和定义一个设置结构体变量的函数
3. 在战斗组件中，通过Playercontroller调用HUD，然后通过是否装配武器判断，进行将已经存储在武器中的准星数据，赋值给结构体中的变量，利用函数
4. 利用DrawHUD绘制准星
首先获取屏幕中心位置
```c++
	GEngine->GameViewport->GetViewportSize(ViewPortSize);
	const FVector2D ViewPortCenter(ViewPortSize.X / 2.f, ViewPortSize.Y / 2.f);
```
对准星位置实施一些修改，进行了一些计算，本质上是调用DrawTexture来绘制对应的纹理
5. 准星动态变化
在结构体变量添加一个控制扩散的比例，设置最大的扩散距离，并且在准星位置的计算中添加了这个扩散。然后在战斗组件中，利用速度映射求取扩散系数的值，如果在空中就利用插值来获得扩散系数
```c++
//准星设置
		if (HUDPackage.CrosshairsBottom)
		{
			FVector2D Spread(0.f, SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsBottom, ViewPortCenter, Spread);
		}

//计算实际准星绘制位置
void AXBlasterHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();

	const FVector2D TextureDrawPoint(ViewportCenter.X - (TextureWidth / 2.f) + Spread.X, ViewportCenter.Y - (TextureHeight / 2.f) + Spread.Y);

	DrawTexture(Texture, TextureDrawPoint.X, TextureDrawPoint.Y, TextureWidth, TextureHeight, 0.f, 0.f, 1.f, 1.f, FLinearColor::White);
}

//战斗组件设置准星
void UCombatComponent::SetHUDCrossHairs(float Deltatime)
{
	if (CharacterEx == nullptr || CharacterEx->Controller == nullptr) return;

	XBlasterPlayerController = XBlasterPlayerController == nullptr ? Cast<AXBlasterPlayerController>(CharacterEx->Controller) : XBlasterPlayerController;

	if (XBlasterPlayerController)
	{
		XBlasterHUD = XBlasterHUD == nullptr ? Cast<AXBlasterHUD>(XBlasterPlayerController->GetHUD()) : XBlasterHUD;
		if (XBlasterHUD)
		{
			FHUDPackage HUDPackage;
			if (EquippedWeapon)
			{
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairCenter;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairRight;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairTop;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairBottom;
			}
			else
			{
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
			}
			//设置Spread calculate crosshair spread
			//利用速度映射0-1 [0,GetMaxWalkSpeed]
			FVector2D WalkSpeedRange(0.f, CharacterEx->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityRange(0.f, 1.f);
			FVector Velocity = CharacterEx->GetVelocity();
			Velocity.Z = 0.f;

			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityRange, Velocity.Size());

			//当在空中时，准星的扩散
			if (CharacterEx->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, Deltatime, 2.25f);
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, Deltatime, 30.f);
			}

			HUDPackage.CrosshairSpread = CrosshairVelocityFactor + CrosshairInAirFactor;

			XBlasterHUD->SetHUDPackage(HUDPackage);
		}
	}
}
```

# 设置视角变换,准星颜色变化
CameraComponent->SetFieldOfView()
设置放大后的清晰度：
	1. Depth Of Field -> Focus Distance
	2. Camera -> Aperture (F-stop)

通过添加接口，当射线检测的目标actor拥有这个接口时改变绘制准星的颜色。
```c++
TraceHitResult.GetActor()->Implements<UInteractWithCrosshairInterface>()
```

# 隐藏角色
当摄像机靠近墙时，隐藏角色
```c++
GetMesh()->SetVisibility(false);
if (CombatComp && CombatComp->EquippedWeapon && CombatComp->EquippedWeapon->WeaponMesh)
{
	CombatComp->EquippedWeapon->WeaponMesh->bOwnerNoSee = true;
}
```

# 蒙太奇的网络传播
多播RPC，利用多播RPC调用播放动画的函数，然后在子弹类的击中事件中调用这个多播RPC实现服务器向客户端的传播
```c++
//character.cpp
void AXCharacter::PlayHitReactMontage()
{
	if (CombatComp == nullptr)
	{
		return;
	}
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AXCharacter::MulticastHit_Implementation()
{
	PlayHitReactMontage();
}
//projectileActor.cpp
	//被击中对象播放击中动画
	AXCharacter* CharacterEx = Cast<AXCharacter>(OtherActor);
	if (CharacterEx)
	{
		CharacterEx->MulticastHit();
	}
```
为了使得打击的不是胶囊体而是mesh，定义一个新的objectchannel，然后将mesh变为这个碰撞类型。