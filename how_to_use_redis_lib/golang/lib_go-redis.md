# go-redis
  
## 개요
- Redis 라이브러리.
- 사용하기 편해서 좋다.
- [GitHub](https://github.com/go-redis/redis  )
  
  
## Quickstart
     
```  
func ExampleNewClient() {
    client := redis.NewClient(&redis.Options{
        Addr:     "localhost:6379",
        Password: "", // no password set
        DB:       0,  // use default DB
    })
 
    pong, err := client.Ping().Result()
    fmt.Println(pong, err)
    // Output: PONG <nil>
}
 
func ExampleClient() {
    err := client.Set("key", "value", 0).Err()
    if err != nil {
        panic(err)
    }
 
    val, err := client.Get("key").Result()
    if err != nil {
        panic(err)
    }
    fmt.Println("key", val)
 
    val2, err := client.Get("key2").Result()
    if err == redis.Nil {
        fmt.Println("key2 does not exists")
    } else if err != nil {
        panic(err)
    } else {
        fmt.Println("key2", val2)
    }
    // Output: key value
    // key2 does not exists
}
```
  
   
## Redis-Pub / Sub 사용하기
[출처](https://qiita.com/Tommy_/items/67c5808abcf03f4e1dde  )  
KVS로 유명한 Redis에는 Pub/Sub 기능이 있다. 이번에는 Redis의 Pub/Sub를 사용하여 메시지 교환을 한다. 언어는 Go 언어를 사용한다.  
  
### 구현  
이번에는 Publisher, Subscriber가 모두 하나인 경우를 생각한다.   
go-redis/redis 를 사용하면 다음과 같이 Publisher, Subscriber을 구조체로 나타낼 수 있다.  
  
publisher.go  
```
//Publisher 구조체. redis 클라이언트나 Pub/Sub에 관련된 데이터를 가진다
type Publisher struct{
    redis *redis.Client
    channel string
    pubsub *redis.PubSub
}
 
func NewPublisher(channel string) *Publisher {
    client := NewRedis()
    return &Publisher{
        redis:client,
        channel:channel,
        pubsub:client.Subscribe(channel),
    }
}
 
//SubScriber 용 채널을 생성
func (p Publisher) SubChannel() <-chan *redis.Message{
    _, err := p.pubsub.Receive()
    if err != nil {
        panic(err)
    }
 
    return p.pubsub.Channel()
}
 
func (p Publisher)Close() error {
    err := p.pubsub.Close()
    return err
}
 
//메시지 보내기
func (p Publisher) Publish(message string) error {
    err := p.redis.Publish(p.channel,message).Err()
    return err
}

subscriber.go
//SubScriber는 메시지를 수신하는 채널을 가진다
type Subscriber struct {
    ch <-chan *redis.Message
}
 
func (s Subscriber)RecieveMessage() {
    for msg:= range s.ch{
        fmt.Println("recieve: ",msg)
    }
}
```
  
Publisher는 Redis 클라이언트를 갖고, Pub/Sub 통신하는 방 이름과 redis의 PubSub용 구조체를 가지고 있다.   
Publish 메소드에 의해 Pub/Sub에 메시지를 보낸다.   
Subscriber 관해서는, Go 언어는 "채널"이라는 비동기 통신을 하는데 편리한 기능이 있고, 이번에는 메시지의 수신에 수신 전용 채널을 사용하고 있다.  
  
  
### 실제 메시지 교환
실제로 Publisher와 Subscriber을 준비하고 메시지의 교환을 실시한다  
  
main.go  
```
publisher := NewPublisher("channel1")
 
 
    subscriber := Subscriber{
        ch: publisher.SubChannel(),
    }
 
    go func() {
        for i := 0; i < 10; i++ {
            time.Sleep(1 * time.Second)
             //시간을 보낸다
            err := publisher.Publish(time.Now().String())
            if err != nil {
                log.Fatal(err)
            }
        }
        publisher.Close()
    }()
 
    // message 수신
    for msg := range subscriber.ch {
        fmt.Println(msg.Payload)
    }
```  
Publisher는 10초 동안 동안 그때의 시간을 보낸다.   
SubScriber는 마지막의 for 문에서 채널이 Close 될 때까지 채널에서 전송된 데이터를 읽는다.   
그러면 다음과 같은 결과를 얻을 수 있다.  
간단한 예이지만, 이제 Pub/Sub 통신을 할 수 있다.  
  
  
## 연결하기 / 끊기
연결    
```
func connectRedis(poolSize int, address string, pw string) *redis.Client {	
	client := redis.NewClient(&redis.Options{
		Addr:         address,
		Password:     pw,
		DialTimeout:  3 * time.Second,
		ReadTimeout:  3 * time.Second,
		WriteTimeout: 3 * time.Second,
		PoolSize:     poolSize,
	})
	
	if err := client.Ping().Err(); err != nil {
		return nil
	}

	return client
}
```
    
연결 끊기    
```
redisDBChannelStatus := connectRedis()

func Close() {
	if redisDBChannelStatus != nil {
		redisDBChannelStatus.Close()
	}
}
```
  
  
## BLPop
List를 자료 타구조에서 왼쪽에서 pop을 하고, pop 가능할 때까지 대기한다.     
```
func _readUserDataNotify() (bool, int64, int) {
	value, err:= redisDBGamerverQueue.BLPop(0, gameServerUserDataChangedNotifyKey).Result()

	if err != nil {
		return false, -1, -1
	}
	
	// value[0]에는 key가 들어가 있다
	if value[1] != "" {
		notifyData := strings.Split(value[1], OBJECT_DELIMETER)
		dataType, result := strconv.Atoi(notifyData[0])
		return true, int64(userId), dataType
	}
	return false, -1, -1
}
```  
  
## 참고 
- [(일어)Go로 Redis의 Sorted Set을 즐겁게 다루고 싶다](https://qiita.com/izumin5210/items/d51c28631c39392c6b16 )