# C# redis 라이브러리 사용 방법 정리
  
# CloudStructures  
- CloudStructures는 StackExchange.Redis를 사용하기 편하게 + 기능을 추가한 라이브러리
    - 즉 CloudStructures의 내부는 StackExchange.Redis를 사용하고 있다
- [깃허브](https://github.com/neuecc/CloudStructures )
- 개발자는 모바일 게임 개발자로 일본의 C# MVP
- NuGet으로 설치한다
- 클래스를 직렬화 할 때에는 IValueConverter를 구현해야 한다.
    - C#용 MessagePack와 Utf8Json를 default로 제공하고 있다.
	- RedisConnection 생성자에 커스텀 IValueConverter를 넘기지 않으면 자동으로 Utf8JsonConverter을 사용한다. 즉 Json 포맷으로 저장한다.
    - MessagePack을 사용하고 싶다면 Nuget에서 'CloudStructures.Converters.MessagePack'을 설치한다.  
- Redis 명령어 설명은 [여기](http://redisgate.kr/redis/introduction/redis_intro.php)를 참고한다   
    
	  
## 사용 예
공식 사이트에 있는 코드  
```
// RedisConnection have to be held as static.
public static class RedisServer
{
    public static RedisConnection Connection { get; }
    public static RedisServer()
    {
        var config = new RedisConfig("name", "connectionString");
        Connection = new RedisConnection(config);
    }
}

// A certain data class
public class Person
{
    public string Name { get; set; }
    public int Age { get; set; }
}

// 1. Create redis structure
var key = "test-key";
var defaultExpiry = TimeSpan.FromDays(1); //데이터 유효기간. 24시간 동안 유효
var redis = new RedisString<Person>(RedisServer.Connection, key, defaultExpiry)

// 2. Call command
var neuecc = new Person("neuecc", 35);
await redis.SetAsync(neuecc);
var result = await redis.GetAsync();
```

  
## Redis 데이터와 CloudStructures 클래스

| Class                         | Description                             |
|-------------------------------|-----------------------------------------|
| RedisBit                      | Bits API                                |
| RedisDictionary<TKey, TValue> | Hashes API with constrained value type  |
| RedisGeo<T>                   | Geometries API                          |
| RedisHashSet<T>               | like RedisDictionary<T, bool>           |
| RedisHyperLogLog<T>           | Redis's HyperLogLog API                 |
| RedisList<T>                  | Redis's Lists API                       |
| RedisLua                      | Lua EVALSHA API                         |
| RedisSet<T>                   | Redis's Sets API                        |
| RedisSortedSet<T>             | Redis's SortedSets API                  |
| RedisString<T>                | Redis's Strings API                     |
   
   

## RedisString
https://github.com/neuecc/CloudStructures/blob/master/src/CloudStructures/Structures/RedisString.cs    
  
### 정수 다루기
정수는 long, double 타입만 지원한다.    
  
```
var redisConfig = new RedisConfig("test", "127.0.0.1");
var RedisConnection = new RedisConnection(redisConfig);

// 이 키의 유효기간은 무제한
var v = new RedisString<int>(RedisConnection, "test-incr", null);

// .Dump()는 LinqPad 툴에서 제공하는 함수이다. 이 코드를 LinqPad가 아닌 곳에서 실행할 때는 Dump() 함수를 사용하지 않는다.
// redis에 이미 값이 있다면 값을 반환하고, 없다면 50을 반환한다.
await v.GetOrSetAsync((() => 50), null).Dump();

// 값을 0 으로 설정
await v.SetAsync(0);

// 기존 값에서 10 증가한다.
await v.IncrementAsync(10).Dump();

// 기존 값에서 5 감소 시킨다.
await v.DecrementAsync(5).Dump();


// 30을 늘린다. 단 100을 넘지 않는다.
await v.IncrementLimitByMaxAsync(30, 100).Dump();

// 80을 늘린다. 단 100을 넘지 않는다.
await v.IncrementLimitByMaxAsync(80, 100).Dump();


// 최소 값 이상이 되도록 한다.
await v.SetAsync(100);
await v.IncrementLimitByMinAsync(-102, 100).Dump();
```
  
  
### 키의 유효 기간 얻기
`GetWithExpiryAsync`    
```
var redisConfig = new RedisConfig("test", "127.0.0.1");
var RedisConnection = new RedisConnection(redisConfig);

// 이 키의 유효기간은 무제한
var defaultExpiry = TimeSpan.FromDays(1);
var v = new RedisString<int>(RedisConnection, "test-incr", defaultExpiry);

await v.SetAsync(230);

// 반환 값의 타입은 RedisResultWithExpiry
// https://github.com/neuecc/CloudStructures/blob/e89268d8ad15452e586521576b501e42b1dee3ca/src/CloudStructures/RedisResult.cs
// 만약 Expiry 시간이 설정되어 있으면 Expiry(TimeSpan 타입)에 값이 설정 되어 있다. Expiry 설정이 없으면 null 
await v.GetWithExpiryAsync().Dump();
```
  
  
### 값을 읽고 삭제하기 
`GetAndDeleteAsync`    
```
var redisConfig = new RedisConfig("test", "127.0.0.1");
var RedisConnection = new RedisConnection(redisConfig);

// 이 키의 유효기간은 무제한
var defaultExpiry = TimeSpan.FromDays(1);
var v = new RedisString<int>(RedisConnection, "test-incr", defaultExpiry);

await v.SetAsync(230);
await v.GetAndDeleteAsync().Dump();

// 값이 없다고 나온다
await v.GetAsync().Dump();
```
  
  
### 이미 key가 있다면 요청 실패하기
`SetAsync` 에서 `When` 파라미터를 사용한다.  
https://github.com/StackExchange/StackExchange.Redis/blob/f52cba7bbe4f22a47a9a7d9c84c9f2824465cae7/src/StackExchange.Redis/Enums/When.cs  
```
namespace StackExchange.Redis
{
    /// <summary>
    /// Indicates when this operation should be performed (only some variations are legal in a given context)
    /// </summary>
    public enum When
    {
        /// <summary>
        /// The operation should occur whether or not there is an existing value 
        /// </summary>
        Always,
        /// <summary>
        /// The operation should only occur when there is an existing value 
        /// </summary>
        Exists,
        /// <summary>
        /// The operation should only occur when there is not an existing value 
        /// </summary>
        NotExists
    }
}
```
    
아래 코드는 키가 없을 때만 230을 저장한다. 만약 이미 있다면 저장하지 않는다.  
```
await v.SetAsync(230, null, When.NotExists);
```  
  
  
  
## list 타입 조작
https://github.com/neuecc/CloudStructures/blob/master/src/CloudStructures/Structures/RedisList.cs  
```
var redisConfig = new RedisConfig("test", "127.0.0.1");
var RedisConnection = new RedisConnection(redisConfig);

var key = "userDataList-test-key";
var defaultExpiry = TimeSpan.FromDays(1);
var redis = new CloudStructures.Structures.RedisList<int>(RedisConnection, key, defaultExpiry);

// LengthAsync 현재 리스트에 있는 데이터 수 얻기
await redis.LengthAsync().Dump();

// 오른쪽에서 push
await redis.RightPushAsync(10);
await redis.RightPushAsync(20);

// 왼쪽에서 pop
while (true)
{
	var value = redis.LeftPopAsync().Result;
	if (value.HasValue == false)
	{
		break;
	}

	value.Dump();
}


// 왼쪽에서 push
// LeftPushAsync

// 오른쪽에서 pop
// RightPopAsync
```
  
Redis 명령에 해당하는 함수   
- [LINDEX](https://redis.io/commands/lindex) : GetByIndexAsync
- [LINSERT](https://redis.io/commands/linsert) :  InsertAfterAsync, InsertBeforeAsync
- [LRANGE](https://redis.io/commands/lrange) : RangeAsync
- [LREM](http://redis.io/commands/lrem) : RemoveAsync
- [RPOPLPUSH](https://redis.io/commands/rpoplpush) : RightPopLeftPushAsync
- [LSET](https://redis.io/commands/lset) : SetByIndexAsync
- [LTRIM](https://redis.io/commands/ltrim) : TrimAsync	
- [SORT](https://redis.io/commands/sort) : SortAsync
  	
	
## Hash
예제 코드:   
```
class UpdateLobbyTask : RedisTask
{
	public UInt16 LobbyNumber;
	public UInt16 LobbyServerIdx;
	public UInt16 LobbyUserCnt;
}

class InsertLobbyTask : RedisTask
{
	public List<UpdateLobbyTask> LobbyInfoList;
}

var task = (UpdateLobbyTask)redisTask;
var redisKey = ServerCommon.RedisKeyForm.LobbyInfoList();
var redisMap = new RedisDictionary<UInt16, ServerCommon.RedisLobbyInfo>(RedisConnection, redisKey, TimeSpan.FromDays(1));
await redisMap.SetAsync(task.LobbyNumber, new ServerCommon.RedisLobbyInfo()
{
	LobbyServerIdx = task.LobbyServerIdx,
	UserCount = task.LobbyUserCnt
});
```
  
  
## 랭킹
  
### 멤버의 순위 얻기
ZRANK : https://redis.io/commands/zrank    
```
var redisConfig = new RedisConfig("test", "127.0.0.1");
var RedisConnection = new RedisConnection(redisConfig);

var set = new RedisSortedSet<string>(RedisConnection, "test-ranking", null);
await set.DeleteAsync();

await set.AddAsync("a", 10);
await set.AddAsync("d", 10000);
await set.AddAsync("b", 100);
await set.AddAsync("f", 1000000);
await set.AddAsync("e", 100000);
await set.AddAsync("c", 1000);

var rank = await set.RankAsync("c");
rank.Dump();
```
  
  
### 지정 순위 안의 랭킹 리스트
ZREVRANGE: 일정 범위의 랭킹 리스트를 가져온다.  
```
var redisConfig = new RedisConfig("test", "127.0.0.1");
var RedisConnection = new RedisConnection(redisConfig);

var set = new RedisSortedSet<string>(RedisConnection, "test-ranking", null);
await set.DeleteAsync();

await set.AddAsync("a", 10);
await set.AddAsync("d", 10000);
await set.AddAsync("b", 100);
await set.AddAsync("f", 1000000);
await set.AddAsync("e", 100000);
await set.AddAsync("c", 1000);

var range = await set.RangeByRankAsync();
range.Dump();

range = await set.RangeByRankAsync(0, 3);
range.Dump();

range = await set.RangeByRankAsync(1, 3);
range.Dump();
```
  
  
### 점수기반으로 순위 정렬  
[ZRANGEBYSCORE](http://redisgate.kr/redis/command/zrangebyscore.php#enter-join)  
[ZREVRANGEBYSCORE](http://redisgate.kr/redis/command/zrevrangebyscore.php#enter-join)  
		 
```
var redisConfig = new RedisConfig("test", "127.0.0.1");
var RedisConnection = new RedisConnection(redisConfig);

var set = new RedisSortedSet<string>(RedisConnection, "test-ranking", null);
await set.DeleteAsync();

await set.AddAsync("a", 10);
await set.AddAsync("d", 10000);
await set.AddAsync("b", 100);
await set.AddAsync("f", 1000000);
await set.AddAsync("e", 100000);
await set.AddAsync("c", 1000);

var range = await set.RangeByScoreAsync();
range.Dump();

// 점수가 100 이상부터
range = await set.RangeByScoreAsync(start: 100);
range.Dump();

// 100~100000 사이만
range = await set.RangeByScoreAsync(start: 100, stop: 100000);
range.Dump();



range = await set.RangeByScoreAsync(order: StackExchange.Redis.Order.Descending);
range.Dump();

range = await set.RangeByScoreAsync(start: 100, order: StackExchange.Redis.Order.Descending); 
range.Dump();

range = await set.RangeByScoreAsync(start: 100, stop: 100000, order: StackExchange.Redis.Order.Descending); 
range.Dump();


range = await set.RangeByScoreAsync(order: StackExchange.Redis.Order.Ascending);
range.Dump();

range = await set.RangeByScoreAsync(start: 100, order: StackExchange.Redis.Order.Ascending);
range.Dump();

range = await set.RangeByScoreAsync(start: 100, stop: 100000, order: StackExchange.Redis.Order.Ascending);
range.Dump();


range = await set.RangeByScoreAsync(order: StackExchange.Redis.Order.Ascending);
angebyscore
```  
  
  
### 멤버의 점수 업데이트
아래 함수를 사용한다.    
```
DecrementAsync
IncrementAsync
IncrementLimitByMinAsync
IncrementLimitByMaxAsync
```

  
## Redis로 lock을 걸고 싶을 때
분산 서버 환경에서 병렬 처리를 순차적으로 처리하고 싶을 때 Redis를 동기화 객체로 사용할 수 있다.  
[RedisLock](https://github.com/neuecc/CloudStructures/blob/master/src/CloudStructures/Structures/RedisLock.cs) 클래스를 사용한다.  
락 걸기 `TakeAsync`, 락 풀기 `ReleaseAsync`   
  
  
## Lua 스크립트 사용하기
[RedisLua](https://github.com/neuecc/CloudStructures/blob/master/src/CloudStructures/Structures/RedisLua.cs) 클래스를 사용한다.  
RedisString의 `IncrementLimitByMaxAsync` 함수가 Lua 스크립트를 사용하고 있다.  
  
  
## 데이터 직렬화로 MessagePack 사용하기
  
```
var redisConfig = new RedisConfig("test", "127.0.0.1");
var converter = new CloudStructures.Converters.MessagePackConverter();
var RedisConnection = new RedisConnection(redisConfig, converter);
```
  
  
## Redis 클러스터로 운용 시 연결 방법
  
```
var connString = "127.0.0.1:7000,127.0.0.1:7001,127.0.0.1:7002";
var redisConfig = new RedisConfig("test", connString);
var redisConnection = new RedisConnection(redisConfig);
```
  
  
## PubSub  
  
```
async Task ServerMsgTo(RedisConnection redisConnection)
{
	var msg = new ServerMsg()
	{
		cmd = "LDB_OFF",
	};
	
	var jsonStr = @"{""cmd"":""LDB_OFF"",""on"":0}";

	var pub = redisConnection.GetConnection().GetSubscriber();
	pub.Publish("Bus_heungbae-1_MgmServer", jsonStr);
}
```
    
