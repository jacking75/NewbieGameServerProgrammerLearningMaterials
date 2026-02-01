package main

import (
	"fmt"
	"time"
)

func ExampleSetGet() {
	fmt.Println("[ExampleSetGet] begin")

	rdb := getClient()
	key := "aaa"

	// 모두 지운다
	rdb.Del(ctx, key)


	fmt.Println("set")
	if _, err := rdb.Set(ctx, key, "111", 0).Result(); err != nil {
		fmt.Println("fail")
		return
	}
	fmt.Println("")


	fmt.Println("get")
	if ret, err := rdb.Get(ctx, key).Result(); err != nil {
		fmt.Println("fail")
		return
	} else {
		fmt.Println("Return: ", ret)
	}
	fmt.Println("")


	fmt.Println("setnx")
	rdb.Del(ctx, key)
	if _, err := rdb.SetNX(ctx, key, "111", time.Second * 2).Result(); err != nil {
		fmt.Println("fail")
		return
	}
	fmt.Println("")


	fmt.Println("setnx. 실패해야 한다")
	if ret, _ := rdb.SetNX(ctx, key, "112", time.Second * 2).Result(); ret != false {
		fmt.Println("fail")
		return
	}
	fmt.Println("")


	fmt.Println("expire 시간 대기")
	time.Sleep(time.Second * 2)
	fmt.Println("")


	fmt.Println("setnx. 성공해야 한다")
	if _, err := rdb.SetNX(ctx, key, "111", time.Second * 2).Result(); err != nil {
		fmt.Println("fail")
		return
	}
	fmt.Println("")


	fmt.Println("")
}