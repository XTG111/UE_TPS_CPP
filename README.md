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

想将客户端的信息传递给服务器只能使用RPC，服务器向客户端传递可以使用RPC或者Rep_Notify也就是说，当一个客户端的状态要让所用客户端直到，那么必须先通过RPC传递给服务器，服务器再选择使用RPC或者Repnotify来传递给所有的客户端

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

# 网络更新和根骨骼旋转的冲突
动画蓝图上的根骨骼旋转Rotate Root Bone并不是每一帧更新的，所以会在模拟ACotr的机器上出现动画抽搐的现象
对于本地或者服务器上，角色的权限都是高于模拟代理的，当我们权限高于模拟代理我们可以开启我们的rotateRootbone，然后再调用我们需要涉及到RotateRootBone的函数比如控制AO_YAw的旋转上，
如果我们是模拟代理那么就需要重写编写设计到RotateRootBone的函数，可以通过直接播放动画来实现。
```c++
/** The network role of an actor on a local/remote network context */
UENUM()
enum ENetRole
{
	/** No role at all. */
	ROLE_None,
	/** Locally simulated proxy of this actor. */
	ROLE_SimulatedProxy,
	/** Locally autonomous proxy of this actor. */
	ROLE_AutonomousProxy,
	/** Authoritative control over the actor. */
	ROLE_Authority,
	ROLE_MAX,
};
```
当我们编辑了如下的一个函数用来控制actor在模拟机器上的动作后
```c++
//模拟转向
void AXCharacter::SimProxiesturn()
{
	if (CombatComp == nullptr || CombatComp->EquippedWeapon == nullptr) return;
	//如果是模拟转向
	bRotateRootBone = false;
	//如果是本地或者服务器控制才能够开启bRotateRootBOne
	CalculateAO_Pitch();

	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;

	//
	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETuringInPlace::ETIP_Right;
		}
		else if(ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETuringInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETuringInPlace::ETIP_NoTurning;
		}
		return;
	}
	TurningInPlace = ETuringInPlace::ETIP_NoTurning;
}
```
功能实现了，但是还是存在相同的问题就是网络更新不是tick调用，而这个函数我们是tick调用的，导致上一帧和当前帧的差值不会时刻更新并且可能会为0，所以为了解决这个问题，就需要考虑当这个旋转值发生改变是就进行更新，那么就可以利用repnotify通知来实现。
而UE为我们提供了这样的一个通知函数OnRep_ReplicatedMovement()
还有一个点是什么时候调用这个通知函数，是网络每一帧更新时，如果动作发生了变化，就会调用这个函数，这样以来我们在模拟机器上的更新频率就和网络更新一样了。
为了防止网络延迟等情况，我们还定义了一个变量用来记录OnRep_ReplicatedMovement()的上一次调用和这一次调用间隔时间，它每个tick增加DeltaTime，如果这个时间大于了某个设定值，将强行调用一次OnRep_ReplicatedMovement()
```c++
//OnRep_ReplicatedMovement()
void AXCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	SimProxiesturn();
	TimeSinceLastMovementReplication = 0;
}

//Tick
void AXCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//只有当角色权限大于模拟且是本地控制是才执行AimOffset
	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		//更新时间
		TimeSinceLastMovementReplication += DeltaTime;
		//当时间超过一个阈值，那说明我们这段时间没有动作或者延迟，那么调用模拟的函数
		//然后重置时间
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}
	HideCameraIfCharacterClose();
}
```

# 自动开火
利用定时器，在第一次开火时调用定时器计算，然后在定时器结束后继续调用开火
```c++
void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || CharacterEx == nullptr) return;
	CharacterEx->GetWorldTimerManager().SetTimer(FireTime, this, &UCombatComponent::FireTimeFinished, EquippedWeapon->FireDelay);
}
//当定时器结束再调用ControlFire相当于一个循环
void UCombatComponent::FireTimeFinished()
{
	if (EquippedWeapon == nullptr) return;
	bCanFire = true;
	if (bFired && EquippedWeapon->bAutomatic)
	{
		ControlFire(bFired);
	}
}

void UCombatComponent::ControlFire(bool bPressed)
{
	if (bCanFire)
	{
		bCanFire = false;
		if (bFired)
		{
			ServerFire(bFired, HitTarget);
			if (EquippedWeapon)
			{
				CrosshairShootingFactor = 0.75f;
			}
		}
		StartFireTimer();
	}

}
```

# Game FrameWork
1. GameMode : Sever Only
	a. Default Classes
	b. Rules
	c. Match State
2. GameState: Server and All Clients
	a. State Of the game
	b. Players States
3. PlayerState: Server and All Clients
	a. Player State
4. Player Controller: Sever and Owning Client
	a. Access to The HUD/Widgets
5. Pawn: Server And All Clients
6. HUD/Widgets: Owing Client Only

# 血条绘制
和准星绘制一样
首先需要创建一个Widget类，然后将这个类添加到HUD类中，在蓝图里绑定相应的蓝图。
通过角色控制器来调用HUD，绘制这个Widget，准星的绘制也可以修改到这里，这不过就需要在重写一个Tick函数。

UI的制作
1. 创建一个PlayerController 和 HUD类。PlayerController用来调用HUD显示Widget，基本所有Widget都组合在HUD里面
2. 创建一个Widget为你具体想实现的UI。记得利用属性说明符和元数据meta=(BindWidget)，变量名字要与蓝图中一致
3. 在UE中新建三个蓝图类分别以上述三个类为父类。HUD类中绑定Widget蓝图类，Widget蓝图类中类属性设置绑定对应的C++类
4. 在PlayerController()中初始化HUD类，以及编写设计Widget中参数的函数实现对UI的更改。
5. 第4步不一定在PlayerController()类中实现，只要可以调用到HUD的类方法都可以使用从而实现。

# 伤害响应
1. 对子弹使用UE内置的ApplyDamage函数设置，以满足伤害响应
2. 对于被击中角色，就需要在角色类中使用对应的回调函数，而UE已经设置了响应的委托，所以我们是需要定义符合参数的回调函数就可以了。
```c++
void ReceivedDamage(AActor* DamageActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);
```
3. 编辑血量数学表达式，并且从游戏开始就初始化这个回调函数，利用HasAuthority来限制之间服务器上响应。
4. 我们将血量设置为可复制的利用REPNotify来传递给其他客户端，这样一来，我们就可以把血条的更改和受击动画的播放写在OnRep里面了，避免了对RPC的占用
```c++
void UXPropertyComponent::ReceivedDamage(AActor* DamageActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	Health = FMath::Clamp(Health - Damage, 0.f, MAXHealth);
	XCharacter->UpdateHUDHealth();
	//将受击动画的播放改到这里，降低RPC调用的负担
	XCharacter->PlayHitReactMontage();
	
}

void UXPropertyComponent::OnRep_HealthChange()
{
	XCharacter->UpdateHUDHealth();
	XCharacter->PlayHitReactMontage();
}
```

# 设置GameMode，和死亡动画
控制角色死亡，得分等等
GameMode可以理解为整个游戏的规则，所以死亡，得分，升级这些可以写在这里

1. 死亡动画的播放
	利用GameMode处理玩家是否死亡，可以定义一个的PlayerEliminated函数，其可以接受到要消失的那个actor，然后利用这个函数调用角色死亡的处理函数。
```c++
void AXBlasterGameMode::PlayerEliminated(AXCharacter* ElimmedCharacter, AXBlasterPlayerController* VictimController, AXBlasterPlayerController* AttackerController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim();
	}
}
```
2. 播放死亡动画
	在Character类中利用Montage播放动画，并在处理死亡的函数中调用
```c++
void AXCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance&&ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
	}
}

void AXCharacter::Elim()
{
	//对动画蓝图通信，死亡和非死亡的动画选择不一样
	bElimmed = true;
	PlayElimMontage();
}
```
3. 当血量降为0时，调用GameMode里面的PlayerEliminated函数，从而调用角色中的Elim函数。我们的血量计算是放在属性组件中的所以需要将函数调用放在属性组件中。
为了实现动画从服务器到客户端，还应当将Elim()设置为多播RPC，当调用服务器上的Elim函数播放死亡动画的时候会传播到所有客户端体现。
```c++
//PropertyComponent.cpp
void UXPropertyComponent::ReceivedDamage(float Damage, AController* InstigatorController)
{
	Health = FMath::Clamp(Health - Damage, 0.f, MAXHealth);
	XCharacter->UpdateHUDHealth();
	//将受击动画的播放改到这里，降低RPC调用的负担
	XCharacter->PlayHitReactMontage();

	if (Health == 0.0f)
	{
		AXBlasterGameMode* XBlasterGameMode = GetWorld()->GetAuthGameMode<AXBlasterGameMode>();
		if (XBlasterGameMode && XCharacter)
		{	
			//XCharacter->XBlasterPlayerController = XCharacter->XBlasterPlayerController == nullptr ? Cast<AXBlasterPlayerController>(XCharacter->Controller) : XCharacter->XBlasterPlayerController;
			AXBlasterPlayerController* AttackerContorller = Cast<AXBlasterPlayerController>(InstigatorController);
			XBlasterGameMode->PlayerEliminated(XCharacter, XCharacter->GetXBlasterPlayerCtr(), AttackerContorller);
		}
	}
}

//Character
void AXCharacter::Elim_Implementation()
{
	//对动画蓝图通信，死亡和非死亡的动画选择不一样
	bElimmed = true;
	PlayElimMontage();
}

```

# 死亡角色的消失和重生
利用定时器实现角色的重生。
将重生函数写在GameMode里，函数需要一个Actor用于确定哪一个Actor消失，然后一个控制器用来控制Actor的生成。
在角色的代码中，当我们的血量为0开始播放死亡动画时，开启定时器，当定时器结束调用重生函数生成Actor
```c++
//GameMode
void AXBlasterGameMode::RequestRespawn(AXCharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}
	if (ElimmedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);

		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);

		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
	}
}

//character
void AXCharacter::Elim()
{
	MulticastElim();
	GetWorldTimerManager().SetTimer(ElimTimer, this, &AXCharacter::ElimTimerFinished, ElimDelay);
}

//重生
void AXCharacter::ElimTimerFinished()
{
	//重生角色
	AXBlasterGameMode* XBlasterGameMode = GetWorld()->GetAuthGameMode<AXBlasterGameMode>();

	if (XBlasterGameMode)
	{
		XBlasterGameMode->RequestRespawn(this, Controller);
	}
}
```

# 溶解材质 ♥♥♥
利用噪声纹理创建溶解的材质，更改纹理的大小即可控制黑白的交替
为了高亮边框，我们利用当前纹理的uv图来获取黑白的位置，利用u坐标，将另一个噪声纹理添加上去，这样就可以获得上一个纹理黑白之间的边缘

# 曲线控制溶解
利用时间轴，控制材质的参数，调整溶解效果。
1. 时间轴
```c++
//.h
	//时间轴组件
	UPROPERTY(VisibleAnywhere)
		UTimelineComponent* DissolveTimeline;
	//相当于蓝图中的轨道
	FOnTimelineFloat DissolveTrack;

	//时间轴曲线
	UPROPERTY(EditAnywhere)
		UCurveFloat* DissolveCurve;

	//获取曲线上的值，用于更新我们要操作的值，相当于蓝图中的输出
	UFUNCTION()
		void UpdateDissolveMaterial(float Dissolve);
	//开始溶解，时间轴绑定上方那个获取值函数的地方，可以被外界调用
	void StartDissolve();
	
//.cpp

//进行时间轴的初始化，绑定，添加曲线，开始运行时间轴等等
//被其他函数调用，从而调用时间轴
void AXCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &AXCharacter::UpdateDissolveMaterial);
	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

//时间轴的回调函数，用于处理我们的具体逻辑
void AXCharacter::UpdateDissolveMaterial(float Dissolve)
{
	if (DynamicDissolveMaterialInstance)
	{
	//对材质中的参数进行复制
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), Dissolve);
	}
}

//外界调用函数
void AXCharacter::MulticastElim_Implementation()
{
	bElimmed = true;
	PlayElimMontage();
	
	//设置材质
	if (DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Grow"), 200.0f);
	}
	StartDissolve();
}
```
2. 曲线
时间轴的曲线由UE中绘制，之后利用属性说明符UPROPERTY(EditAnywhere)进行添加
3. 材质
当死亡时，切换Actor的表面材质，使得其变为我们设置的材质，从而可以在代码中利用时间轴修改
```c++
//.h
	//动态材质用于在代码中操作变量 change at Runtime
	UPROPERTY(VisibleAnywhere, Category = Elim)
		UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;
	//静态材质用于在蓝图中设置，是角色本身的材质实例 set on the BP,use for set the dynamic Material
	UPROPERTY(EditAnywhere, Category = Elim)
		UMaterialInstance* DissolveMaterialInstance;
//.cpp
void AXCharacter::MulticastElim_Implementation()
{
	bElimmed = true;
	PlayElimMontage();
	
	//设置材质
	if (DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Grow"), 200.0f);
	}
	StartDissolve();
}
```
DissolveMaterialInstance用于设置初始材质，也可以选择硬编码直接设置动态材质，然后到死亡时替换。

# 死亡后禁用移动等
禁用移动和胶囊体碰撞都需要写在多播RPC里面
但是dettach武器只需要通过服务器Elim调用就就可以了，因为武器本身就设置了可复制的属性。
在武器父类中，利用之前的枚举和switch编写当武器丢掉后的一些碰撞修改比如胶囊体，网格之类的。因为武器状态我们定义为了一个具有属性复制属性的变量，
所以在武器父类中定义函数Drop首先改变武器枚举中的当前变量，并调用设置状态函数执行switch语句，对网格体等的碰撞进行修改，然后Detach，**最重要是要记得将这把武器的Owner设为空 setowner(nullptr)**，之后在角色代码中调用Drop
1. 注意
	在战斗组件的装备武器函数中，我们最开始只在服务器上对武器的状态进行了设置和武器的附加。但是由于网络的原因，先后顺序并不可靠，所以需要在客户端再写一遍状态的修改和武器附加，避免执行了武器附加但网格体组件这些还未修改
	
# 增加死亡动画
增加了一个机器人用于播放粒子特效，初始化放在多播死亡RPC中，当Actor死亡才会出现这个特效，值得注意的是该机器人的销毁，最开始是想通过角色定时器的销毁流程进行销毁，但由于定时器流程是写在服务器上的并没有使用多播RPC，所以客户端会一致存在不会销毁
可以使用之前子弹销毁的方法，通过重写Destroyed()函数，因为其自身就会复制到每个客户端，所以在角色类中重写Destroyed()函数，其内部语句就是调用这个机器人粒子特效的销毁。

# OnPosses
当一个角色控制器需要取请求控制一个Pawn时需要重写这个函数APlayerController::OnPossess。
当玩家重生时，角色控制器不会主动对重生角色的控制，导致UI绘制失败，就像如果我们游戏最开始不在Beginplay上调用UpdateHUD，那么角色控制器中的血量不会得到更新。我们在UpdateHUD里面的工作是将当前的控制器类型转换为了我们设置的，然后调用里面的绘制函数
重写OnPossess相当于将重生的角色绑定到我们编写的角色控制器上，从而通过角色控制器设置HUD上的值
OnPossese里面基本代码就是先解绑如果有其他控制器，然后将调用这个函数的类绑定给Pawn
```c++
		if (GetPawn() && bNewPawn)
		{
			UnPossess();
		}

		if (PawnToPossess->Controller != NULL)
		{
			PawnToPossess->Controller->UnPossess();
		}

		PawnToPossess->PossessedBy(this);
```

# PlayerState类的网络同步
PlayerState保存玩家自己的信息
在PlayerState的基类，UE已经定义得分和属性复制函数等一系列方法和变量，我们只需要重写就可以了。
下面来设置得分变化。
1. PlayerState会保存当前玩家的状态存储在服务器上，即使玩家死亡，只要没有退出游戏，这个PlayerState就不会被GC
2. 通过PlayerState可以获取到当前对应的玩家GetPawn()，利用玩家获得控制器Character->Controller()
3. 设置分数的更新，OnRep只会在客户端上被调用，所以需要新建一个服务器上处理得分并绘制UI的操作
4. 绘制UI，我们绘制UI的逻辑就是widget->HUD->PlayerController->被其他类调用
	a. widget就是我们在UE中创建的面板这些
	b. 利用HUD Add To ViewPort
	c. PlayerController可以设置HUD的参数从而更新HUD中的Widget
	- 在这里当我们得分了就需要更新widget所以在playerstate中调用contorller中的更新函数
5. GameMode游戏规则，何时进行得分操作，当一个角色死亡时进行，利用创建的Elimmed函数中有攻击者和被攻击者的控制器，获取对应的PlayerState(Controller->PlayerState)
在这个函数中调用PlayerState中的改变分数的函数
6. 为角色添加PlayerState
新建一个PlayerState类，但需要注意的是PlayerState不能在BeginPlay的时候初始化，因为在游戏开始的第一帧其并不能被创建。所以我们定义了一个函数专门用来处理这些不能在第一帧被创建的类，然后在tick中调用，如果其不为空就跳过创建。当为空时，调用PlayerState里面的AddToScore函数初始化得分为0，同时传给Controller使得绘制widget。
7. 注意PlayerState里面的Score为私有变量所以最好利用SetScore()和GetScore()来设置

# 类指针对象使用UPROPERTY()
对于类指针对象我们经常使用if来判断其是否为空指针，但是在UE中如果没有添加属性说明符，会导致其被GC机制放弃，从而被回收，这样该指针实际上并不是一个空指针在还没有初始化的时候，所以if(Ptr)可能永远为真。
1. 第一种方法时在定义时初始化为nullptr;
2. 第二种方法是添加UPROPERTY()属性说明符，被GC识别不会被回收，所以其最开始会是一个空指针。

# 向PLayerState里面添加一个自定义状态变量
1. 将变量属性声明为UPROPERTY(ReplicatedUsing = OnRep_FuncName)
2. 重写GetLifeTimeRepli...函数，定义OnRep_FuncName，定义服务器处理函数
3. 函数内部定义和Score类似

# 为UI添加子弹
需要主要更新的地方有两个：
1. 每当开火子弹数减少，利用将子弹数设置为可复制的进行调用
因为子弹是武器的所属，所以可以将子弹的计算和调用绘制UI放在武器类里面。本质上就是当子弹数发生变化时，调用绘制函数并广播。所以需要一个服务器处理函数（计算子弹数量和绘制），和一个repnotify函数（绘制）。然后在开火的时候调用。
2. 当玩家装备武器，即武器的Owner发生变化时显示当前这把武器的子弹数，由于Owner本身是可复制的，所以重新OnRep_Owner()
当角色拿到武器时，我们就需要更改UI上的子弹数
也就是说当武器有owner的时候进行UI更改，我们调用绘制函数也是通过Character->Controller->Func来绘制的。而Owner本身是一个被定义为可复制的变量，拥有OnRep_Owner()。
我们在战斗组件CombatComponet中通过装备武器函数为我们装备的武器设置了Owner此时Owner发生了改变。所以我们可以通过重载OnRep_Owner()来来实现当我们客户端角色装备了武器时的子弹数UI的绘制，然后在战斗组件上利用已装备武器EquippedWeapon这个变量调用OnRep_Owner这个函数实现服务器上的显示。

# 角色死亡，替换武器后的子弹更新
1. 重要的一点是，每当我们替换武器或者丢掉武器都需要将Owner，Character和Controller设为空
2. 在我们的代码中我们死亡后会调用Drop函数，我们替换武器也使用Drop函数，Drop函数内会将Owner设为空，那我们还需要将Character和Controller也设置为空。
3. 上述是服务器的流程逻辑，通过OnRep_Owner()我们还需要将客户端上的Character和Controller设置为空
```c++
//weapon.cpp
void AWeaponParent::OnRep_Owner()
{
	Super::OnRep_Owner();
	if (GetOwner() == nullptr)
	{
		XCharacter = nullptr;
		XBlasterPlayerController = nullptr;
	}
	else
	{
		SetHUDAmmo();
	}
}
```
4. 上述只是清空了与HUD绑定的三个对象，还需要更新HUD，更新HUD是通过Controller操作的。可以利用播放死亡特效的多播RPC获取到死亡Actor的控制器，然后设置HUD=0;
```c++
//character.cpp
void AXCharacter::MulticastElim_Implementation()
{
	//当该Actor死亡时，调用控制器，设置其子弹数为空
	if (XBlasterPlayerController)
	{
		XBlasterPlayerController->SetHUDWeaponAmmo(0);
	}
```

# 控制是否能够开枪
在战斗组件中设置一个判断能否开枪的函数，其返回值是子弹数和0的大小以及定时器的控制的或
```C++
EquippedWeapon->Ammo > 0 || !bCanFire;
```
然后将我们控制开枪的函数中的判断改为这个函数的返回值
同样我们需要限制子弹数利用FMath::Clamp(A,B,C)，可以将返回值A限制在B-C之间
```c++
Ammo = FMath::Clamp(Ammo - 1, 0, MaxAmmo);
//等价于
//Ammo--;
//0<Ammo<MaxAmmo
```

# 备弹和枪械类型的设置
设置了当前角色对装备在手上的这类武器的剩余子弹数。该子弹数是可复制的，并且只会由服务器传给武器Owner的客户端。我们还为武器类型新建了一个枚举值，用来存储各种武器。
并利用哈希表Map来存储武器类型(key)和它现在的子弹(value)
因为备弹数相当于角色的一个战斗属性，所以将其写在CombatComponent中
1. 在BeginPlay初始化Hash表，当装备武器的时候，通过当前武器的类型获取Hash表中对应的备弹数。然后绘制UI
2. 客户端上的绘制，由于备弹数是可复制的利用Rep_Notify函数调用绘制函数从而进行绘制。

# 换弹
利用蒙太奇播放来实现换弹的动画，主要考虑几个问题
增加了一个枚举值，用来确定当前枪械是在换弹还是空闲
1. IK的限制
由于我们使用了IK限制左手移动，所以当我们换弹时如果不接触IK将导致没法播放正确的动画，所以我们通过枚举值的改变，利用布尔混合姿势修改IK的使用
2. 动画的传播，从服务器到客户端有两种方式前面已经学习了--RPC和属性复制(OnRep_Notify).
RPC是客户端或者服务器上请求然后在服务器上调用，
所以我们使用属性复制OnRep_Notify当枚举变量发生改变时，在客户端直接播放蒙太奇动画。
3. 动画播放的优先级控制
	a. 当我们动画播放完成要设置枚举变量为空闲这样才能在下一次Reload的时候正确播放蒙太奇动画，我们通过设置一个函数然后利用动画通知来在蓝图中调用它，从而实现状态的重置
	b. 其次我们需要控制换弹时不能开枪，所以在控制开枪的bool函数中增加判断
	```c++
	return EquippedWeapon->Ammo > 0 && !bCanFire && CombatState == ECombatState::ECS_Unoccupied;
	```
	c. 还有就是开火的粒子特效也不能在换弹时播放
	```c++
	if (CharacterEx && CombatState == ECombatState::ECS_Unoccupied)
	```
	d. 设置OnRep_CombateState里面的switch因为我们完成了换弹切换了状态，这个函数就会被客户端调用，为了更好的实现可以添加当此时按下了鼠标左键可以开火的功能
	```c++
	void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Unoccupied:
		if (bFired)
		{
			ControlFire(bFired);
		}
		break;
	case ECombatState::ECS_Reloading:
		HandleReload();
		break;
	case ECombatState::ECS_MAX:
		break;
	default:
		break;
	}
}
	```
	
4. 换弹的整体流程：
Character按键响应->调用CombatComp上的换弹函数->改变此时枪械状态->RPC设置服务器处理函数在服务器实现换弹动画的播放->OnRep_Notify实现所有客户端的状态同步->动画通知实现结束换弹后的状态重置
5. 对换弹与开火冲突的解决
	通过添加新建的枪械状态实现判断条件的增加，比如说在换弹时不能开火，换弹时接触IK绑定，换弹时不能有开火的粒子特效
	结束换弹后，添加判断，判断当前的攻击键是否按下，按下就执行开火等等

# 自动换弹
1. 在自动开火结束过后，判断是否现在还持有武器，如果当前武器子弹数为0那么可以调用Reload函数，在Reload函数里面我们判断了当前备弹量是否为0，然后进行换弹

2. 当Actor拾取到了一把空的枪后会自动换弹，调用reload
```c++

	////auro reload when no ammo in automatic Fire
	//if (EquippedWeapon->Ammo == 0)
	//{
	//	ReloadWeapon();
	//}
```

# 游戏剩余时间的同步
1. 游戏剩余时间 InitialTime - GetWorld()->GetSecondsTime()
2. 时间同步
	a. 客户端和服务器的传输时间
	b. 服务器器会维护一个GetWorld()->GetSecondsTime()，客户端也会维护一个GetWorld()->GetSecondsTime()
	c. 服务器开始的时间和客户端开始游戏的时间存在差异LoadingTime
3. 计算LoadingTime
	a. 利用RPC客户端向服务器请求当前的服务器上的时间，服务器再通过RPC返回时间
	b. RPC的传播也需要时间
	c. 服务器还需要添加ClientRPC到客户端
4. 往返时间RoundTripTime
	a. RTT = SeverRPCTime + ClientRPCTime
	b. 客户端发送RPC包含自己当前的时间ClientLastTime，服务器返回RPC包含客户端传过来的值以及服务器发送RPC的时间SeverReceiptTime
	c. RTT = ClientCurrentTime - ClientLastTime
	d. 那么客户端就知道了自己接受到RPC后现在服务器的时间ServerCurrentTime = SeverReceiptTime+1/2 RTT
	e. 所以服务器和客户端之间的时间差值就是：Client-SeverDeltaTime = ClientCurrentTime - ServerCurrentTime
5. RPC
```c++
//客户端向服务器发送的RPC
void AXBlasterPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequese)
{
	//获取接受时服务器的时刻
	float SeverTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequese, SeverTimeOfReceipt);
}

//服务器向客户端发送的RPC
void AXBlasterPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	//计算RTT
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	//ServerCurrentTime
	float CurrentServerTime = TimeServerReceivedClientRequest + (0.5f * RoundTripTime);

	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}
```
6. 计算当前服务器的实际时间
```c++
float AXBlasterPlayerController::GetSeverTime()
{
	//如果该控制器在服务器上
	if (HasAuthority())
	{
		return GetWorld()->GetTimeSeconds();

	}	
	else 
	{
		return GetWorld()->GetTimeSeconds() + ClientServerDelta;
	} 
}
```
7. 为了能够在游戏开始快速的同步，利用ReceivedPlayer求取时间差
```c++
void AXBlasterPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	//本地客户端的控制器
	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}
```
8. 保证时间正确性，利用Tick一定频率更新调用ServerRequestServerTime
9. 绘制

# AGameModeBase 和 AGameMode
AGameMode是AGameModeBase的子类，其拥有MatchState
1. MatchState控制GameState的整个顺序流程
```c++
/** Possible state of the current match, where a match is all the gameplay that happens on a single map */
namespace MatchState
{
	extern ENGINE_API const FName EnteringMap;			// We are entering this map, actors are not yet ticking
	extern ENGINE_API const FName WaitingToStart;		// Actors are ticking, but the match has not yet started
	extern ENGINE_API const FName InProgress;			// Normal gameplay is occurring. Specific games will have their own state machine inside this state
	extern ENGINE_API const FName WaitingPostMatch;		// Match has ended so we aren't accepting new players, but actors are still ticking
	extern ENGINE_API const FName LeavingMap;			// We are transitioning out of the map to another location
	extern ENGINE_API const FName Aborted;				// Match has failed due to network issues or other problems, cannot continue

	// If a game needs to add additional states, you may need to override HasMatchStarted and HasMatchEnded to deal with the new states
	// Do not add any states before WaitingToStart or after WaitingPostMatch
}
```
2. HasMatchStarted() HasMatchEnded() GetMatchState() SetMatchState() 
3. 利用OnMatchStateSet()来添加我们自定义的状态，可以在WaitingToStart之后，InProgress之前添加我们自定义的状态，在游戏结束后我们需要手动增加一个Cooldown在InProgress之后WaitingPostMatch之前
4. 在InProgress之前可以拥有一个热身时间WarmupTime 在其之后有一个冷却时间CoolDownTime InProgress为MatchTime，还需要一个加载时间从登录地图到游戏地图
5. GameState自带一个延迟启动属性bDelayedStart=true，如果其为真，那么在WaitingToStart阶段就已经启动了地图的加载，然后我们可以调用StartMatch进入，
在设置了其为true后，关卡中会生成默认的pawn就是那个球体，当我们调用StartMatch进入游戏时，才会将Pawn改为我们设置好的PAWN类

# 热身时间 before InProgress
通过设置bDelayedStart=true，使得我们需要手动调用StartMatch才能进行比赛，以及生成角色和对角色的操控。
在InProgress之前，当我们点击了运行按钮，会生成默认的Pawn类而不是我们给定的Pawn类，其可以有wasd操控移动。这是游戏中的等待状态。我们通过在GameMode里面设置热身时间和控制热身时间以及当前这个关卡的开始时间，可以实现倒计时结束后调用StartMatch()开始游戏。
1. 首先需要构造函数设置bDelayedStart=true
2. 定义一个热身时间10s，利用Tick函数更新记录时间，当记录时间为0时调用StartMatch.
3. 需要注意的是GetWorld()->GetTimeSeconds()是游戏一开始就会运行，而我们需要的这个关卡的运行时间，所以我们可以定义一个变量当进入这个关卡是接受GetWorld()->GetTimeSeconds()，因为GameMode()这是对于我们这个Level才拥有的，所以利用BeginPlay设置进入关卡的时间
```c++
AXBlasterGameMode::AXBlasterGameMode()
{
	bDelayedStart = true;
}

void AXBlasterGameMode::BeginPlay()
{
	Super::BeginPlay();
	LevelStartingTime = GetWorld()->GetTimeSeconds();
}


void AXBlasterGameMode::Tick(float DeltatTime)
{
	Super::Tick(DeltatTime);
	if (MatchState == MatchState::WaitingToStart)
	{
		CountDownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountDownTime <= 0.f)
		{
			StartMatch();
		}
	}
}
```

# HUD的绘制时机控制
当我们使用了bDelayedStart,那么我们需要控制Actor的UI应该在StartMatch之后绘制。那么就需要将MatchState传入到PlayerController中。在GameMode里面利用重写OnMatchStateSet函数，可以利用PlayerController的迭代器来遍历这个GameMode管理的所有PlayerController然后调用PlayerController中的函数，将MatchState值传入
```c++
//GameMode
void AXBlasterGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
	{
		AXBlasterPlayerController* XBlasterPlayerController = Cast<AXBlasterPlayerController>(*It);
		if (XBlasterPlayerController)
		{
			XBlasterPlayerController->OnMatchStateSet(MatchState);
		}
	}
}
//PlayerController
void AXBlasterPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;

	//当状态是InProgress时，调用UI绘制
	if (MatchState == MatchState::InProgress)
	{
		XBlasterHUD = XBlasterHUD == nullptr ? Cast<AXBlasterHUD>(GetHUD()) : XBlasterHUD;
		if (XBlasterHUD)
		{
			XBlasterHUD->AddCharacterOverlay();
		}
	}
}
void AXBlasterPlayerController::OnRep_MatchState()
{
	//当状态是InProgress时，调用UI绘制
	if (MatchState == MatchState::InProgress)
	{
		XBlasterHUD = XBlasterHUD == nullptr ? Cast<AXBlasterHUD>(GetHUD()) : XBlasterHUD;
		if (XBlasterHUD)
		{
			XBlasterHUD->AddCharacterOverlay();
		}
	}
}
```
需要通过属性赋值当MatchState改变时，让客户端也进行相应的操作，即当MatchState为InProgress时，调用绘制整体UI的函数。之前我们时防在整体XBlasterHUD的BeginPlay中进行的，会导致一开始就绘制。GameMode->PlayerController->XBlasterHUD。
2. 出现的问题--我们设置的这个CharacterWidget的初始化还没有完成就会被PlayerController控制的Actor上的属性比如血量，击杀数等等赋值，导致整个UI的绘制出错。
```c++
//调整顺序后，下方值可能在绘制的时候为false,导致无法绘制
bool bHUDvalid = XBlasterHUD &&	XBlasterHUD->CharacterOverlayWdg && XBlasterHUD->CharacterOverlayWdg->CarriedAmmoAmount;
```
就相当于在BeginPlay的时候，数据已经传入，而UI的控件的初始化是这一系列初始化的最后一个流程。而由于其又是只在BeginPlay里面运行一次，所以导致后面不会更新。
所以我们需要自己增加初始化流程，就是当BeginPlay的时候在数据存入到一个中间变量中，然后可以每个tick去检测绘制数据，或者等初始完成之后绘制。
```c++
//存储数据
void AXBlasterPlayerController::SetHUDDefeats(int32 Defeats)
{
	XBlasterHUD = XBlasterHUD == nullptr ? Cast<AXBlasterHUD>(GetHUD()) : XBlasterHUD;

	bool bHUDvalid = XBlasterHUD &&
		XBlasterHUD->CharacterOverlayWdg &&
		XBlasterHUD->CharacterOverlayWdg->DefeatsAmount;
	if (bHUDvalid)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Defeats));
		XBlasterHUD->CharacterOverlayWdg->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
	else
	{
		bInitializeCharacterOverlay = true;
		HUDDefeats = Defeats;//存储数据
	}
}

//PollInit
void AXBlasterPlayerController::PollInit()
{
	if (CharacterOverlayWdg == nullptr)
	{
		if (XBlasterHUD && XBlasterHUD->CharacterOverlayWdg)
		{
			CharacterOverlayWdg = XBlasterHUD->CharacterOverlayWdg;
			if (CharacterOverlayWdg)
			{
				SetHUDHealth(HUDHealth, HUDMaxHealth);
				SetHUDScore(HUDScore);
				SetHUDDefeats(HUDDefeats);
			}
		}
	}
}

//在Tick函数内调用 或者OnMatchStateSet()设置
```
这种情况只会发生在比如人的血量这种游戏开始就需要初始化的情况，对于枪械的子弹并不会有影响，因为其可以通过后面的拾取操作来进行更新。就如这个如果血量不需要初始化也不需要设置，因为之后会通过击中扣血更改HUD

# WarmTime UI
由于在WatingToStart的状态下HUD还没发初始化，所以不能通过一下方式进行绘制
```c++
	//
	if (MatchState == MatchState::WaitingToStart)
	{
		XBlasterHUD = XBlasterHUD == nullptr ? Cast<AXBlasterHUD>(GetHUD()) : XBlasterHUD;
		if (XBlasterHUD)
		{
			XBlasterHUD->AddAnnouncement();
		}
	}
```
只有通过BeginPlay在我们初始化了HUD之后进行绘制。
```c++
void AXBlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	XBlasterHUD = Cast<AXBlasterHUD>(GetHUD());

	//由于最开始HUD并不存在所以不能使用MatchState来判断绘制
	if (XBlasterHUD)
	{
		XBlasterHUD->AddAnnouncement();
	}
}
```
当进入到InProgress状态后，将这个UI切换为Hidden模式

# WarmUP时间的获得
1. ClientRPC的使用场景：许多变量需要传递，并且只需要传递一次
2. 更新时间就涉及到与GameMode进行通信，我们利用ServerRPC在服务器上获取到当前GameMode的所有时间，并且将热身UI的绘制放在这里来执行
```c++
void AXBlasterPlayerController::ServerCheckMatchState_Implementation()
{
	AXBlasterGameMode* GameMode = Cast<AXBlasterGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		
		//将服务器上的状态传给客户端
		ClientJoinMidGame(MatchState, WarmupTime, MatchTime, LevelStartingTime);
	}
}
```
我们会在BeginPlay的时候调用这个RPC，使得每个客户端都会获得服务器上此时的状态
```c++
void AXBlasterPlayerController::ClientJoinMidGame_Implementation(FName StateOfMatch,float Warmup,float Match,float StartingTime)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
	OnMatchStateSet(MatchState);

	if (XBlasterHUD && MatchState == MatchState::WaitingToStart)
	{
		XBlasterHUD->AddAnnouncement();
	}
}
```
现在我们获得了服务器上的时间和状态并客户端也接受到了这个时间和状态，我们就需要通过这个时间和状态来设置热身时间和游戏时间的变化。
因为MatchState是一个可复制的变量，所以为了防止由于属性复制和RPC的带来的值冲突，所以我们在RPC中也调用属性复制的函数。传入最新的状态。
也就是说RPC的优先级最高，一般来说属性复制都是最后校验的。但可能出现属性复制先进行的情况，所以为了使得所有更新发生，在ClientRPC中再调用一次属性复制的更新。
当我们的状态时WaritingToStart时，倒计时初始时间就是WarmupTime - GetServerTime() + LevelStartingTime
当我们的状态时InProgress时，倒计时初始时间就是WarmupTime + MatchTime - GetServerTime() + LevelStartingTime
```c++
void AXBlasterPlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart)
	{
		TimeLeft = WarmupTime - GetSeverTime() + LevelStartingTime;
	}
	else if (MatchState == MatchState::InProgress)
	{
		TimeLeft = WarmupTime + MatchTime - GetSeverTime() + LevelStartingTime;
	}
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);
	//当SecondsLeft和上一次不相等时更新UI;
	if (SecondsLeft != CountDownInt)
	{
		if (MatchState == MatchState::WaitingToStart)
		{
			SetHUDAnnouncementCountDown(SecondsLeft);
		}
		if (MatchState == MatchState::InProgress)
		{
			SetHUDGameTime(SecondsLeft);
		}
	}
	CountDownInt = SecondsLeft;
}
```

# 新增冷却状态到MatchState中
利用namespace名称空间新添加和AGameMode基类里相同的FName属性值CoolDown，利用extern声明其可以在其他文件中定义，使用当前项目的API表示只能被这个项目使用，并且具有dll属性
```c++

//GameMode.h
namespace MatchState
{
	extern XBLASTER_CP_API const FName CoolDown;//游戏结束，显示获胜玩家，并且开启显示结束缓冲时间
}
//GameMode.cpp
namespace MatchState
{
	const FName CoolDown = FName("CoolDown");
}
```
这样我们就为MatchState新增了一个状态，现在需要编写一下这个状态下客户端和服务器的响应，以及什么时候跳转到这个状态。
首先是什么时候跳转到这个状态，我们在GameMode里面处理各种MatchState的切换，从热身时间开始到整局游戏结束，将跳转到CoolDown状态,利用Tick实时检测，由于我们是新增状态，没法像InProgress一样可以通过UE写好的函数切换，所以使用SetMatchState(...)
```c++
void AXBlasterGameMode::Tick(float DeltatTime)
{
	Super::Tick(DeltatTime);
	if (MatchState == MatchState::WaitingToStart)
	{
		CountDownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountDownTime <= 0.f)
		{
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountDownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountDownTime <= 0.f)
		{
			SetMatchState(MatchState::CoolDown);
		}
	}
```
当状态切换后，就需要到PlayerController中处理状态修改完成之后的功能切换，比如UI的绘制等等，我们参考UE的格式，为每个状态要处理的函数集合到一起定义为HandleStateName命名格式的函数，然后在属性复制发生改变时调用它们
```c++
void AXBlasterPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;

	//当状态是InProgress时，调用UI绘制CharacterOverlay
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::CoolDown)
	{
		HandleCoolDown();
	}
}

void AXBlasterPlayerController::OnRep_MatchState()
{
	//当状态是InProgress时，调用UI绘制
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::CoolDown)
	{
		HandleCoolDown();
	}
}
```

# 重新开始游戏
1. 在GameMode里面Tick函数中增加了else if判断，当时间满足条件RestartGame();
2. 在冷却时间应该考虑的事情
	1. 角色的移动控制，通过新增一个bool变量，来控制是否进行操作，如果为真那么就直接返回
	```c++
	void AXCharacter::MoveForward(float value)
{
	//禁用
	if (bDisableGamePlay) return;
	FRotator ControlRot = GetControlRotation();
	ControlRot.Pitch = 0.0f;
	ControlRot.Roll = 0.0f;
	ForwardValue = value;
	FVector FowardVector = FRotationMatrix(ControlRot).GetScaledAxis(EAxis::X);
	AddMovementInput(FowardVector, value);
}
	```
	2. 控制是根据游戏特性来决定的，但需要注意一些问题，比如在开枪的过程中游戏时间结束了，这时候不能只设置这个bool值，还需要去设置能否开枪。
	```c++
	void AXBlasterPlayerController::HandleCoolDown()
{
	XBlasterHUD = XBlasterHUD == nullptr ? Cast<AXBlasterHUD>(GetHUD()) : XBlasterHUD;
	if (XBlasterHUD)
	{
		XBlasterHUD->CharacterOverlayWdg->RemoveFromParent();
		if (XBlasterHUD->AnnouncementWdg)
		{
			XBlasterHUD->AnnouncementWdg->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementText("New Match Starts In: ");
			if (XBlasterHUD->AnnouncementWdg->AnnouncementText)
			{
				XBlasterHUD->AnnouncementWdg->AnnouncementText->SetText(FText::FromString(AnnouncementText));
			}
			if (XBlasterHUD->AnnouncementWdg->InfoText)
			{
				XBlasterHUD->AnnouncementWdg->InfoText->SetText(FText());
			}
		}
	}
	AXCharacter* XCharacter = Cast<AXCharacter>(GetPawn());
	if (XCharacter)
	{
		XCharacter->bDisableGamePlay = true;
		if (XCharacter->GetCombatComp())
		{
			XCharacter->GetCombatComp()->IsFired(false);
			if (XCharacter->GetCombatComp()->GetEquippedWeapon())
			{
				XCharacter->GetCombatComp()->GetEquippedWeapon()->Drop();
			}
		}
	}
}
	```
	3. 还有比如转向这些，即那些在角色中，通过调用其他函数或者该操作是发生一个过程直到过程结束，我们需要单独的去控制它们的结束，相当于手动释放。
	
# GameState
可以用来存储整局游戏的玩家状态
```c++
//GameMode
GetGameState<AClassGameState>();
//其他类
Cast<AClassGameState>(UGameplayStatics::GetGameState(this));
```
可以在GameState里面维护一个存储了玩家状态的数组，将这个数组设置为可复制的类型，用于从服务器传递到客户端，现在只维护了一个存储最高得分玩家的数组，而不是所有玩家。在GameMode里面我们设置了死亡时攻击者增加得分，这时候将攻击者的PlayerState存到这个数组里。最后在CoolDownState将文本打印出来。

**后续可以增加一个存储所有玩家状态的Map，然后在BeginPlay的时候把每个PlayerState存入，当有得分或者死亡时更新它**

# 新增了火箭筒武器，以及Niagra 不太会 