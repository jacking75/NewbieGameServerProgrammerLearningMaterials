package main

import (
	"fmt"
	"github.com/go-redis/redis/v8"
)

func Examplelist() {
	fmt.Println("[Examplelist] begin")

	rdb := getClient()
	battleKey := "332_4347"

	// 모두 지운다
	rdb.Del(ctx, battleKey)


	// 리스트에 데이터 추가한다
	fmt.Println("리스트에 데이터 추가한다")
	_RPush(rdb, battleKey, []byte("dsdsd1"))
	_RPush(rdb, battleKey, []byte("dsdsd2"))
	_RPush(rdb, battleKey, []byte("dsdsd3"))


	fmt.Println("순회한다")
	_LRange(rdb, battleKey, 0, -1)
	fmt.Println("")


	fmt.Println("앞에 2개만 순회한다")
	_LRange(rdb, battleKey, 0, 1)
	fmt.Println("")


	fmt.Println("Key를 삭제 후 순회해본다")
	rdb.Del(ctx, battleKey)
	_LRange(rdb, battleKey, 0, -1)
	fmt.Println("")


	fmt.Println("[Examplelist] end")
	fmt.Println("")
}

func _RPush(rdb *redis.Client, battleKey string, playData []byte) {
	if err := rdb.RPush(ctx, battleKey, playData).Err(); err != nil {
		fmt.Println("[fail] _RPush: ", err)
	}
}

func _LRange(rdb *redis.Client, battleKey string, start int64, last int64) {
	if values, err := rdb.LRange(ctx, battleKey, start, last).Result(); err != nil {
		fmt.Println("[fail] _LRange: ", err)
	} else {
		for _, value := range values {
			fmt.Println(value)
		}
	}
}