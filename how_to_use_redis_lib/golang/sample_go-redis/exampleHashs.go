package main

import "fmt"

func ExampleHashs() {
	fmt.Println("[ExampleHashs] begin")

	rdb := getClient()

	competitionKey := "compe_12345"
	authKey := "auth_dsd"
	authCode := 1213232

	// 모두 지운다
	rdb.Del(ctx, competitionKey)


	fmt.Println("auth 추가 해본다")
	if isSuccess, err := rdb.HSetNX(ctx, competitionKey, authKey, authCode).Result(); err != nil {
		fmt.Println("[fail] - HSetNX: %v, %t", err, isSuccess)
		return
	} else if isSuccess == false {
		fmt.Println("[fail] - HSetNX: dup")
		return
	}
	fmt.Println("")


	fmt.Println("같은 auth를  추가 해본다. 실패가 되어야 한다")
	if isSuccess, err := rdb.HSetNX(ctx, competitionKey, authKey, authCode).Result(); err != nil {
		fmt.Println("[fail] - Re HSetNX: %v, %t", err, isSuccess)
		return
	} else if isSuccess == false {
		fmt.Println("[fail] - Re HSetNX: dup")
	}
	fmt.Println("")


	fmt.Println("[필드를 추가한다]")
	if isSuccess, err := rdb.HSetNX(ctx, competitionKey, "battle1", 32323232).Result(); err != nil {
		fmt.Println("[fail] - Battle HSetNX: %v, %t", err, isSuccess)
		return
	} else if isSuccess == false {
		fmt.Println("[fail] - Battle HSetNX: dup")
		return
	}
	fmt.Println("")

	fmt.Println("[순회]")
	if bttleKeys, err := rdb.HKeys(ctx, competitionKey).Result(); err == nil {
		for _, key := range bttleKeys {
			fmt.Println("Battle: ", key)
		}
	}
	fmt.Println("")


	fmt.Println("[battle1 가 있는지 확인한다]")
	if isExists, err := rdb.HExists(ctx, competitionKey, "battle1").Result(); err != nil {
		fmt.Println("[fail] - Battle HExists: ", err)
		return
	} else {
		fmt.Println("Battle HExists: ", isExists)
	}
	fmt.Println("")


	fmt.Println("[auth code를 얻는다]")
	if value, err := rdb.HGet(ctx, competitionKey, authKey).Result(); err != nil {
		fmt.Println("[fail] - Battle HGet: ", err)
		return
	} else {
		fmt.Println("competitionKey authKey: ", value)
	}
	fmt.Println("")


	fmt.Println("[삭제-순회]")
	if err := rdb.HDel(ctx, competitionKey, authKey).Err(); err != nil {
		fmt.Println("[fail] - HDel: ", err)
	}
	if bttleKeys, err := rdb.HKeys(ctx, competitionKey).Result(); err == nil {
		for _, key := range bttleKeys {
			fmt.Println("Battle: ", key)
		}
	}
	fmt.Println("")

	fmt.Println("[ExampleHashs] end")
	fmt.Println("")
}
