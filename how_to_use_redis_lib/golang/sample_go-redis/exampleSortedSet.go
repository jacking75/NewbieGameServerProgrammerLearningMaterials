package main

import (
	"fmt"
	"github.com/go-redis/redis/v8"
	"strconv"
)

func ExampleSortedSet() {
	fmt.Println("[ExampleSortedSet] begin")

	rdb := getClient()
	competitionListKey := "competitionListKey"

	// 모두 지운다
	rdb.Del(ctx, competitionListKey)


	fmt.Println("[필드를 추가한다")
	_ZAdd(rdb, competitionListKey, "aa", float64(24))
	_ZAdd(rdb, competitionListKey, "bb", float64(12))
	_ZAdd(rdb, competitionListKey, "cc", float64(20))
	_ZRangePrint(rdb, competitionListKey)
	fmt.Println("")


	fmt.Println("[ZRangeByScore. 0~20]")
	minText := strconv.FormatInt(0, 10)
	maxText := strconv.FormatInt(20, 10)
	competitions, err := rdb.ZRangeByScore(ctx, competitionListKey, &redis.ZRangeBy{Min:minText, Max:maxText,}).Result()
	if err != nil {
		fmt.Println("[fail] - ZRangeByScore: ", err)
	}
	for _, str := range competitions {
		fmt.Println("ZRange: ", str)
	}
	fmt.Println("")


	fmt.Println("[덮어 쓰기] bb 120")
	_ZAdd(rdb, competitionListKey, "bb", float64(120))
	_ZRangePrint(rdb, competitionListKey)


	fmt.Println("[ExampleSortedSet] end")
	fmt.Println("")
}


func _ZAdd(rdb *redis.Client, competitionListKey string, member string, score float64) {
	if _, err := rdb.ZAdd( ctx, competitionListKey, &redis.Z{
		Score:  score,
		Member: member,
	}).Result(); err != nil {
		fmt.Println("[fail] _ZAdd: ", err)
	}
}

func _ZRangePrint(rdb *redis.Client, competitionListKey string) {
	fmt.Println("[ZRange]")
	competitions, err := rdb.ZRange(ctx, competitionListKey, 0, -1).Result()
	if err != nil {
		fmt.Println("[fail] - ZRange: ", err)
		return
	}
	for _, str := range competitions {
		score := rdb.ZScore(ctx, competitionListKey, str)
		fmt.Println("ZRange: ", score)
	}
	fmt.Println("")
}